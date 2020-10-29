# Building and testing the OPTIGA™ Trust M for the Connected Home IP Software

This delivery provides a set of files to allow the Infineon OPTIGA M device to perfom CHIP crypto operations. The OPTIGA M device will act as a HW accelerator to the mbedtls library.

The user should start with a Raspberry PI running Raspian Buster and must be online.

Firstly download the Infineon files by entering:

```console
pi@raspberrypi:git clone https://github.com/Infineon/connected-home-optiga-trust
```

Then assemble the OPTIGA M Board on the Raspberry Pi and enable GPIO in the raspberry configuration and use the executable in the HWTest directory to check that the OPTIGA M is talking to the Raspberry Pi by entering
```console
pi@raspberrypi:~ $ cd connected-home-optiga-trust/
pi@raspberrypi:~/connected-home-optiga-trust $ chmod 777 hwtest/trustx_chipinfo 
pi@raspberrypi:~/connected-home-optiga-trust $ sudo hwtest/trustx_chipinfocd 
```
This should produce a list of chip parameters which proves that the OPTIGA-M is talking via GPIO to the Raspberry PI

Then, to build the code:

## 1 -  Login as root , then run script to setup tools
```console
root@raspberrypi:apt-get update
root@raspberrypi:chmod +x ./prep_rpi.bash
root@raspberrypi:sudo bash ./prep_rpi.bash
```

## 2 - Clone the CHIP repo

Use commit bd7042eb5ab2a071dcbc726edc896a788269eeea , Wed Oct 28 15:37:51 2020 -0400

```console 
root@raspberrypi:su pi
pi@raspberrypi:cd ~
pi@raspberrypi:git clone https://github.com/project-chip/connectedhomeip
pi@raspberrypi:cd connectedhomeip
pi@raspberrypi:git submodule update --init
pi@raspberrypi:git checkout bd7042eb5ab2a071dcbc726edc896a788269eeea
```

## 3 - Install missing packages
```console
pi@raspberrypi:~/connectedhomeip$:sudo apt-get install libdbus-glib-1-2 libdbus-glib-1-dev libavahi-client-dev 
```


## 4 - Build with mbedtls
```console
pi@raspberrypi:~/connectedhomeip$:gn gen out/pi --args='chip_crypto="mbedtls" chip_enable_python_modules=false treat_warnings_as_errors = false'
pi@raspberrypi:~/connectedhomeip$ ninja -C out/pi
```

## 5 - Run the crypo test executable
```console
pi@raspberrypi:~/connectedhomeip$ out/pi/tests/CHIPCryptoPALTest
```

If all tests show "PASSED" the SW crypto has passed tests. Now insert the OPTIGA-M HW crypto.

## 6 - Insert the IFX OPTIGA-M Directory structure
```console
pi@raspberrypi:~/connectedhomeip$ cd third_party/mbedtls/
pi@raspberrypi:/home/pi/connectedhomeip/third_party/mbedtls# git clone https://github.com/Infineon/optiga-trust-m
```

## 7 - Replace the orignal CHIP codebase files with the files included in the Infineon file set



mbedtls/config.h -> /connectedhomeip/third_party/mbedtls/repo/include/mbedtls - config file to enable HW accellerator

mbedtls/BUILD.gn -> /connectedhomeip/third_party/mbedtls - gn file to build OPTIGA M SW

mbedtls/platform_alt.h -> /connectedhomeip/third_party/mbedtls/optiga-trust-m/optiga/include - init function declarations

src/crypto/BUILD.gn -> /connectedhomeip/src/crypto - automake file to build crypto driver

src/crypto/CHIPCryptoPALmbedtls.cpp -> /connectedhomeip/src/crypto - OPTIGA M crypto driver

src/crypto/tests/BUILD.gn -> /connectedhomeip/src/crypto - gn  file to build crypto driver tests

src/crypto/CHIPCryptoPALTest.cpp -> /connectedhomeip/src/crypto - OPTIGA M crypto driver tests

optigam_mbedtls/trustm_ecdh.c ->/connectedhomeip/third_party/mbedtls/optiga-trust-m/examples/mbedtls_port - modified mbedtls ECDH implementation using OptigaM

optigam_mbedtls/trustm_ecdsa.c ->/connectedhomeip/third_party/mbedtls/optiga-trust-m/examples/mbedtls_port - modified mbedtls ECDSA implementation using OptigaM

optigam_mbedtls/trustm_random.c ->/connectedhomeip/third_party/mbedtls/optiga-trust-m/examples/mbedtls_port - fixed mbedtls RNG implementation using OptigaM

optigam_mbedtls/trustm_init.c ->/connectedhomeip/third_party/mbedtls/optiga-trust-m/examples/mbedtls_port - new OPTIGA M Init routine using mbedtls API

## 8 - Modify optiga_lib_config.h

This file is at: /connectedhomeip/third_party/mbedtls/optiga-trust-m/optiga/include/optiga/optiga_lib_config.h

#define OPTIGA_COMMS_DEFAULT_RESET_TYPE     (1)

//#define OPTIGA_COMMS_SHIELDED_CONNECTION  Disable shielded connection

## 9  - Edit CHIPCryptoPAL.h to make the members of class P256Keypair public:

Edit /connectedhomeip/src/crypto/CHIPCryptoPAL.h as follows:

public:

    P256PublicKey mPublicKey;
    
    P256KeypairContext mKeypair;
    
    bool mInitialized = false;
    
};

## 10 - Rebuild with warnings allowed
```console
pi@raspberrypi:~/connectedhomeip$:gn gen out/pi --args='chip_crypto="mbedtls" chip_enable_python_modules=false treat_warnings_as_errors = false'
pi@raspberrypi:~/connectedhomeip$ ninja -C out/pi
```

## 11 - Run the crypo test executable
(Make sure that RaspPi I2C is enabled)
```console
pi@raspberrypi:~/connectedhomeip$ out/pi/tests/CHIPCryptoPALTest
```

Note: For instrumenting the code there are several printfs in CHIPCryptoPALmbedtls.cpp & ChipCryptoPALTest.cpp. They all start with printf("IFX_> , so you can use an editor to automatically comment or delete these as needed. The output appears in the crypto test output

IMPLEMENTATION DETAILS

The two files that really matter:

- connectedhomeip/src/crypto/CHIPCryptoPALmbedtls.cpp - The main file of interest. This implements CHIP crypto by making calls to an mbedtls library underneath. The file has been modified to allow the calls to asymmetric crypto. to be implemented by the OptigaM implementation of mbedtls.

- connectedhomeip/src/crypto/tests/ChipCryptoPALTest.cpp - This file implements the crypto tests. Some tests have been modified to allow for OptigaM impelmentation. In
particular we cannot import private key test vectors, so the tests must create the keypair on the Optiga and then export the public key from the Optiga to the test harness. As this SW is still work in progress we have made the key data public in the SW structures, as a way of getting things working in the short term with minimum complexity. In the longer term - once the SW is stable - we can modify the code to encapsulate as much key data as feasible.

However, the keys are generated purely for testing of the crypto functionality. System wide management of the keys is considered out of scope for this work. (My guess is this will be customer specific and should certainly be managed in detail by the customer. Taking our off the shelf solution as a black box would seem a very dangerous thing for an OEM customer to do)

Also, there's currently no shielded connection currently in place in order to get up & running as easily as possible.



