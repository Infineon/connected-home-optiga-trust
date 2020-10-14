#!/usr/bin/env bash

# make sure it's not being sourced.
if [[ "${BASH_SOURCE[o]}" != "$0" ]]; then
    # NOTE: Needs to be the first thing run, normal message functions haven't been defined, yet.
    echo "ERROR: Please do not 'source' this script, run as a regular shell script (bash \"$0\")."
    exit 1
fi

# turn on strictures
#   - error if using uninitialized variables
#   - error if a non-zero return code is ignored
#   - if DEBUG env var. is set, turn on verbose/logging
set -ue${DEBUG+xv}


# setup handler for CTRL+C (abort / SIGINT)
handle_ctrl_c()
{
    handle_exit 1
}
trap "handle_ctrl_c" INT

# messages to summarize (and whether we should summarize them)
_G_MESSAGES=()
_G_SUMMARIZE=false

# setup exit handler (fires if the script exists for any reason).
handle_exit()
{
    local exit_code=$?
    trap - EXIT

    for msg in "${_G_MESSAGES[@]}"; do
        echo "$msg"
    done

    # make sure we clean up after ourselves however we exit
    cleanup

    if [ "$exit_code" != 0 ]; then
        # exiting with an error code, print additional / helpful information.
        echo "--ABORTING --"
        echo

        local i=${#FUNCNAME[@]}

        echo -e "\\tCallstack:"
        while (( i > 0 )); do
            i=$((i - 1))

            echo -e "\\t\\t${FUNCNAME[$i]} (called at line ${BASH_LINENO[$i]})"
        done

        echo
        echo -e "\\tCurrent directory: $PWD"
        echo -e "\\tBash Version     : $BASH_VERSION"
        echo -e "\\tSystem Info      : $(uname -a)"
        echo -e "\\tDistro Info      : $(lsb_release -si) $(lsb_release -sr) $(lsb_release -sc)"
    fi

    return $exit_code
}
trap "handle_exit" EXIT

_msg()
{
    local tag="$1"
    local space="${tag//?/ }"
    shift

    echo "$tag: $1"
    if $_G_SUMMARIZE; then
        _G_MESSAGES+=( "$tag: $1" )
    fi
    shift

    while (( $# > 0 )); do
        echo "$space: $1"
        if $_G_SUMMARIZE; then
            _G_MESSAGES+=( "$space: $1" )
        fi
        shift
    done
}

# print warning messages
info()
{
    _msg "INFO" "$@"
}

# print warning messages
warning()
{
    _msg "WARNING" "$@"
}

# print non-fatal error messages
error()
{
    _msg "ERROR" "$@"
}

# print fatal error messages
abort()
{
    error "$@"
    exit 1
}

# make sure given apps are available
assert_apps_exist()
{
    for exe in "$@"; do
        if ! type -p "$exe" >/dev/null 2>&1; then
            abort "Unable to find required utility: $exe"
        fi
    done
}

# sanity check the users environment (make sure it's supported, has requird utils, etc.)
check_env()
{
    # we require lsb_release so we can discover which distro andcpu we're running under.
    assert_apps_exist lsb_release uname

    local distro ver arch
    
    distro="$(lsb_release -si)"
    ver="$(lsb_release -sr)"
    arch="$(uname -m)"

    # make sure we're running under Raspberry Pi OS on an ARM CPU.
    local supported=true
    case "$distro/$ver" in
        Debian/10|Raspbian/10)
            # Raspberry Pi OS for 32b ARM reports Raspbian (old name of Raspberry Pi OS).
            # Raspberry Pi OS for 64b ARM reports Debian (distro Raspberry Pi OS is based on).
            true
            ;;

        *)
            supported=false
            ;;
    esac

    case "$arch" in
        arm*|aarch64)
            true
            ;;

        *)
            supported=false
            ;;
    esac

    if ! $supported; then
        warning "This script has only been tested on the latest Raspberry Pi OS based on Debian/Buster"
    fi

    if [[ "$USER" != "root" ]]; then
        abort "This script requires 'root' access (sudo bash \"$0\") so it can install required packages."
    fi
}


cleanup()
{
    if [[ -d "$_G_TMP" && "$_G_TMP" != "__INVALID__" ]]; then
        info "Removing temporary directory '$_G_TMP'."
        rm -rf "$_G_TMP"
    fi
}

# chip directory
_G_BASE="/usr/local"
_G_TMP="__INVALID__"
create_dirs()
{
    if [ ! -d "$_G_BASE/bin" ]; then
        info "Creating directory for CHIP utilities ('$_G_BASE/bin')..."
        mkdir -p "$_G_BASE/bin"
    fi

    _G_TMP="$(mktemp -d)"
    info "Creating temp directory for source-built packages ('$_G_TMP')..."
    [ -d "$_G_TMP" ] || abort "Unable to create temp directory!"
}

install_required_apt_packages()
{
    info "Making sure required packges are installed..."

    local desired=( wget python3 git gcc ninja-build clang libdbus-glib-1-2 libdbus-glib-1-dev )
    local missing=( )

    for pkg in "${desired[@]}"; do
        if ! dpkg-query --no-pager -s "$pkg" >/dev/null 2>&1; then
            missing+=( "$pkg" )
        fi
    done

    if (( ${#missing[@]} > 0 )); then
        info "Installing the following packages: ${missing[*]}"
        apt install -y "${missing[@]}" || abort "Unable to install 1 or more packages!"
    else
        info "Required packages already installed."
    fi
}

install_gn_from_source()
{
    if [[ ! -x "$_G_BASE/bin/gn" ]]; then
        local url="https://gn.googlesource.com/gn"
        local commit="a9eaeb80cb02c33d99dea76efcaa69d2e83bafc2"

        # NOTE: gn does not have any release tags or branches for us to checkout, so using a tested SHA1 hash.

        pushd "$_G_TMP" >&/dev/null 
        info "Downloading gn from via git from '$url'..."

        git clone --recurse-submodules "$url" gn
        cd gn
        git checkout "$commit"
        
        info "Building 'gn'..."
        python3 build/gen.py || abort "Unable to setup gn builds."
        ninja -C out || abort "Unable to build gn."

        info "Installing 'gn'..."
        cp out/gn "$_G_BASE/bin" || abort "Uanble to copy $(pwd)/out/gn to '$_G_BASE/bin'."
        chmod 755 "$_G_BASE/bin/gn" || abort "Unable to set permissions on '$_G_BASE/bin/gn'."

        popd >&/dev/null
    else
        info "Found '$_G_BASE/bin/gn', skipping 'gn' installation."
    fi
}

# print help message
print_help()
{
    cat <<EOF
${BASH_ARGV0##/} $_G_VERSION

[-h|--help] [-v|--version]


-h  --help          Print this usage information and exit.

-v  --version       Print the version and exit.

-b  --base <dir>    Directory to install utilities that will be built from source.
                    [optional, defaults to /usr/local/bin]

EOF
}

# print version
_G_VERSION="v1.0"
print_version()
{
    echo "${BASH_ARGV0##/} $_G_VERSION"
}

# main application
main()
{
    echo "========================================================="
    info "Preparing system for CHIP."
    echo

    while (( $# > 0 )); do
        case "$1" in
            -h|--help)
                print_help
                exit 0
                ;;

            -v|-version|--version)
                print_version
                exit 0
                ;;

            *)
                error "Unrecognzied argument '$1'"
                print_help
                exit 1
                ;;
        esac

        shift
    done

    check_env
    create_dirs
    install_required_apt_packages
    install_gn_from_source
}

main "$@"

