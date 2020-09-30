# Building and testing the OPTIGA™ Trust M for the Connected Home IP Software


This delivery provides a set of files to allow the Infineon OPTIGA M device to perfom CHIP crypto operations. The OPTIGA M device will act as a HW accelerator to the MbedTLS library.

As an example, this document will detail how to insert the files in the (now deprecated)CHIP Autotools environment. This was tested on commit 
[`3b0835852afb3a5d11e94bfc43cf2d326d379ed2`](https://github.com/project-chip/connectedhomeip/tree/3b0835852afb3a5d11e94bfc43cf2d326d379ed2) dated 25 September 2020.

The CHIP codebase is at:
 
https://github.com/project-chip/connectedhomeip

These instructions are based on the build instructions: BUILDING.md (no longer part included in  CHIP as old automake build system is deprecated)

Firstly assemble the OPTIGA M Board on the Raspberry Pi and use the executable in the HWTest directory to check that the OPTIGA M is talking to the Raspberry Pi.

Then, to build the code:

## 1 -  Checkout the CHIP repo

```console
user@user:/home/pi$ git clone https://github.com/project-chip/connectedhomeip
```

## 2 - Build the standard CHIP code as per instructions - this will prompt to bring in any other tools you don't yet have (refer to build instructions to see additional tools required)

```console
user@user:/home/pi$ cd connectedhomeip
user@user:/home/pi$ ./bootstrap
user@user:/home/pi$ make -f Makefile-Standalone
```

## 3 - Create a build directory and create a custom version (i.e. using MbedTLS rather than OpenSSL). Here we use "ifx" as the directory name as an example

```console
user@user:/home/pi$ mkdir build/ifx
user@user:/home/pi$ cd build/ifx
user@user:/home/pi$ ../../configure -C --with-crypto=mbedtls
```

## 4 - Build the MbedTLS SW implementation in this build directory

```console
user@user:/home/pi$ ../../bootstrap
user@user:/home/pi$ make -C third_party/mbedtls 
```

## 5 - Run the crypto tests on the standard CHIP code with MbedTLS in SW as a sanity test

```console
user@user:/home/pi$ make -C src/crypto clean check
```

You should see the following summary:

```
============================================================================
# TOTAL: 1
# PASS:  1
# SKIP:  0
# XFAIL: 0
# FAIL:  0
# XPASS: 0
# ERROR: 0
============================================================================
```

If any tests fail - look at the crypto test log file

```console
user@user:/home/pi$ clear;cat src/crypto/tests/TestCryptoPAL.log
```

Now that the SW MbedTLS implementation is build and tested, overwrite the SW MbedTLS files with the ones to enable the OPTIGA M

## 6 - Get the OPTIGA M Files from Infineon 

```console
user@user:/home/pi$ cd ../../third_party/mbedtls/
user@user:/home/pi$ git clone https://github.com/Infineon/optiga-trust-m
```

## 7 - Merge the attached files with the original files

Note that the CHIP codebase is continually changing. Any updates to these files since commit 1fc71d8f6a798ccbf02af0c2ed1e0f5b37e0a529 dated 22 September 2020 will have to be merged into these files.

[MBedTLS/config.h](MBedTLS/config.h) -> /connectedhomeip/third_party/mbedtls/repo/include/mbedtls - config file to enable HW accellerator
[MBedTLS/Makefile.am](MBedTLS/Makefile.am) -> /connectedhomeip/third_party/mbedtls - automake file to build OPTIGA M SW
[MBedTLS/platform_alt.h](MBedTLS/platform_alt.h) -> /connectedhomeip/third_party/mbedtls/optiga-trust-m/optiga/include - init function declarations

[src/crypto/Makefile.am](src/crypto/Makefile.am) -> /connectedhomeip/src/crypto - automake file to build crypto driver
[src/crypto/CHIPCryptoPALmbedTLS.cpp](src/crypto/CHIPCryptoPALmbedTLS.cpp) -> /connectedhomeip/src/crypto - OPTIGA M crypto driver
[src/crypto/tests/Makefile.am](src/crypto/tests/Makefile.am) -> /connectedhomeip/src/crypto - automake file to build crypto driver tests
[src/crypto/CHIPCryptoPALTest.cpp](src/crypto/CHIPCryptoPALTest.cpp) -> /connectedhomeip/src/crypto - OPTIGA M crypto driver tests

(In future OPTIGA M Files direcly from IFX repo, no need to overwrite ??)
[OptigaMMbedTLS/trustm_ecdh.c](OptigaMMbedTLS/trustm_ecdh.c) ->/connectedhomeip/third_party/mbedtls/optiga-trust-m/examples/mbedtls_port - modified MBedTLS ECDH implementation using OptigaM
[OptigaMMbedTLS/trustm_ecdsa.c](OptigaMMbedTLS/trustm_ecdsa.c) ->/connectedhomeip/third_party/mbedtls/optiga-trust-m/examples/mbedtls_port - modified MBedTLS ECDSA implementation using OptigaM
[OptigaMMbedTLS/trustm_random.c](OptigaMMbedTLS/trustm_random.c) ->/connectedhomeip/third_party/mbedtls/optiga-trust-m/examples/mbedtls_port - fixed MBedTLS RNG implementation using OptigaM
[OptigaMMbedTLS/trustm_init.c](OptigaMMbedTLS/trustm_init.c) ->/connectedhomeip/third_party/mbedtls/optiga-trust-m/examples/mbedtls_port - new OPTIGA M Init routine using MBedTLS API

## 8 - Modify `optiga_lib_config.h`

```
 #define OPTIGA_COMMS_DEFAULT_RESET_TYPE     (1)
 //#define OPTIGA_COMMS_SHIELDED_CONNECTION  Disable shielded connection
```

## 9 Edit `CHIPCryptoPAL.h` to make the members of class P256Keypair public:

```cpp
public:
    P256PublicKey mPublicKey;
    P256KeypairContext mKeypair;
    bool mInitialized = false;
};
```


## 10 - Rebuild the MbedTLS SW implementation, now using the OPTIGA M

```console
user@user:/home/pi$ cd ../../build/ifx
user@user:/home/pi$ ../../bootstrap 
user@user:/home/pi$ make -C third_party/mbedtls clean
user@user:/home/pi$ make -C third_party/mbedtls 
```

## 11 - Run the crypto tests on the CHIP code with OPTIGA M 

```console
user@user:/home/pi$ make -C src/crypto clean check
```

**Note: For instrumenting the code there are several printfs in CHIPCryptoPALmbedTLS.cpp & ChipCryptoPALTest.cpp using the macro IFX_DBG(...) printf(__VA_ARGS__). This macro may be redefined as needed (i.e. stubbed out to supress debug messages)**


# Implementation details

The two files that really matter:

- `connectedhomeip/src/crypto/CHIPCryptoPALmbedTLS.cpp` - The main file of interest. This implements CHIP crypto by making calls to an mbedTLS library underneath. The file has been modified to allow the calls to asymmetric crypto. to be implemented by the OptigaM implementation of mbedTLS.

- `connectedhomeip/src/crypto/tests/ChipCryptoPALTest.cpp` - This file implements the crypto tests. Some tests have been modified to allow for OptigaM impelmentation. In
particular we cannot import private key test vectors, so the tests must create the keypair on the Optiga and then export the public key from the Optiga to the test harness. As this SW is still work in progress we have made the key data public in the SW structures, as a way of getting things working in the short term with minimum complexity. In the longer term - once the SW is stable - we can modify the code to encapsulate as much key data as feasible.

However, the keys are generated purely for testing of the crypto functionality. System wide management of the keys is considered out of scope for this work. (My guess is this will be customer specific and should certainly be managed in detail by the customer. Taking our off the shelf solution as a black box would seem a very dangerous thing for an OEM customer to do)

Also, there's currently no shielded connection currently in place in order to get up & running as easily as possible.



