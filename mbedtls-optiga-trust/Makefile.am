#
#    Copyright (c) 2020 Project CHIP Authors
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.


#
#    Description:
#      This file is the GNU automake template for the CHIP in-package,
#      mbedTLS library.
#
#

include $(abs_top_nlbuild_autotools_dir)/automake/pre.am

if CHIP_WITH_MBEDTLS_INTERNAL

lib_LIBRARIES                                                                  = libmbedtls.a

# Never include the mbedTLS sources in a distribution; they always will get pulled from upstream
# via git operations, hence the 'nodist_' qualifier.

nodist_libmbedtls_a_SOURCES                                                    = \
    repo/optiga/crypt/optiga_crypt.c                          \
    repo/optiga/util/optiga_util.c                            \
    repo/optiga/cmd/optiga_cmd.c                              \
    repo/optiga/common/optiga_lib_common.c                    \
    repo/optiga/common/optiga_lib_logger.c                    \
    repo/optiga/comms/ifx_i2c/ifx_i2c.c                       \
    repo/optiga/comms/ifx_i2c/ifx_i2c_config.c                \
    repo/optiga/comms/ifx_i2c/ifx_i2c_data_link_layer.c       \
    repo/optiga/comms/ifx_i2c/ifx_i2c_physical_layer.c        \
    repo/optiga/comms/ifx_i2c/ifx_i2c_presentation_layer.c    \
    repo/optiga/comms/ifx_i2c/ifx_i2c_transport_layer.c       \
    repo/optiga/comms/optiga_comms_ifx_i2c.c                  \
    repo/examples/mbedtls_port/trustm_ecdh.c                  \
    repo/examples/mbedtls_port/trustm_ecdsa.c                 \
    repo/examples/mbedtls_port/trustm_random.c                \
    repo/pal/linux/pal.c                                      \
    repo/pal/linux/pal_gpio.c                                 \
    repo/pal/linux/pal_i2c.c                                  \
    repo/pal/linux/target/rpi3/pal_ifx_i2c_config.c           \
    repo/pal/linux/pal_os_event.c                             \
    repo/pal/linux/pal_os_datastore.c                         \
    repo/pal/linux/pal_logger.c                               \
    repo/pal/linux/pal_os_lock.c                              \
    repo/pal/linux/pal_os_timer.c                             \
    repo/pal/linux/pal_os_memory.c                            \
    repo/externals/mbedtls/library/aes.c                      \
    repo/externals/mbedtls/library/pem.c                      \
    repo/externals/mbedtls/library/aesni.c                    \
    repo/externals/mbedtls/library/pk.c                       \
    repo/externals/mbedtls/library/arc4.c                     \
    repo/externals/mbedtls/library/pkcs11.c                   \
    repo/externals/mbedtls/library/aria.c                     \
    repo/externals/mbedtls/library/pkcs12.c                   \
    repo/externals/mbedtls/library/asn1parse.c                \
    repo/externals/mbedtls/library/pkcs5.c                    \
    repo/externals/mbedtls/library/asn1write.c                \
    repo/externals/mbedtls/library/pkparse.c                  \
    repo/externals/mbedtls/library/base64.c                   \
    repo/externals/mbedtls/library/pk_wrap.c                  \
    repo/externals/mbedtls/library/bignum.c                   \
    repo/externals/mbedtls/library/pkwrite.c                  \
    repo/externals/mbedtls/library/blowfish.c                 \
    repo/externals/mbedtls/library/platform.c                 \
    repo/externals/mbedtls/library/camellia.c                 \
    repo/externals/mbedtls/library/platform_util.c            \
    repo/externals/mbedtls/library/ccm.c                      \
    repo/externals/mbedtls/library/poly1305.c                 \
    repo/externals/mbedtls/library/certs.c                    \
    repo/externals/mbedtls/library/chacha20.c                 \
    repo/externals/mbedtls/library/chachapoly.c               \
    repo/externals/mbedtls/library/cipher.c                   \
    repo/externals/mbedtls/library/cipher_wrap.c              \
    repo/externals/mbedtls/library/cmac.c                     \
    repo/externals/mbedtls/library/ripemd160.c                \
    repo/externals/mbedtls/library/ctr_drbg.c                 \
    repo/externals/mbedtls/library/rsa.c                      \
    repo/externals/mbedtls/library/debug.c                    \
    repo/externals/mbedtls/library/rsa_internal.c             \
    repo/externals/mbedtls/library/des.c                      \
    repo/externals/mbedtls/library/sha1.c                     \
    repo/externals/mbedtls/library/dhm.c                      \
    repo/externals/mbedtls/library/sha256.c                   \
    repo/externals/mbedtls/library/ecdh.c                     \
    repo/externals/mbedtls/library/sha512.c                   \
    repo/externals/mbedtls/library/ecdsa.c                    \
    repo/externals/mbedtls/library/ssl_cache.c                \
    repo/externals/mbedtls/library/ecjpake.c                  \
    repo/externals/mbedtls/library/ssl_ciphersuites.c         \
    repo/externals/mbedtls/library/ecp.c                      \
    repo/externals/mbedtls/library/ssl_cli.c                  \
    repo/externals/mbedtls/library/ecp_curves.c               \
    repo/externals/mbedtls/library/ssl_cookie.c               \
    repo/externals/mbedtls/library/entropy.c                  \
    repo/externals/mbedtls/library/ssl_srv.c                  \
    repo/externals/mbedtls/library/error.c                    \
    repo/externals/mbedtls/library/ssl_ticket.c               \
    repo/externals/mbedtls/library/gcm.c                      \
    repo/externals/mbedtls/library/ssl_tls.c                  \
    repo/externals/mbedtls/library/havege.c                   \
    repo/externals/mbedtls/library/threading.c                \
    repo/externals/mbedtls/library/hkdf.c                     \
    repo/externals/mbedtls/library/hmac_drbg.c                \
    repo/externals/mbedtls/library/version.c                  \
    repo/externals/mbedtls/library/md2.c                      \
    repo/externals/mbedtls/library/version_features.c         \
    repo/externals/mbedtls/library/md4.c                      \
    repo/externals/mbedtls/library/x509.c                     \
    repo/externals/mbedtls/library/md5.c                      \
    repo/externals/mbedtls/library/x509_create.c              \
    repo/externals/mbedtls/library/md.c                       \
    repo/externals/mbedtls/library/x509_crl.c                 \
    repo/externals/mbedtls/library/memory_buffer_alloc.c      \
    repo/externals/mbedtls/library/x509_csr.c                 \
    repo/externals/mbedtls/library/nist_kw.c                  \
    repo/externals/mbedtls/library/x509write_crt.c            \
    repo/externals/mbedtls/library/oid.c                      \
    repo/externals/mbedtls/library/x509write_csr.c            \
    repo/externals/mbedtls/library/padlock.c                  \
    repo/externals/mbedtls/library/xtea.c                     \
    repo/externals/mbedtls/library/md_wrap.c                  \
    $(NULL)

if !CONFIG_DEVICE_LAYER
nodist_libmbedtls_a_SOURCES                                                   += \
    repo/externals/mbedtls/library/entropy_poll.c             \
    repo/externals/mbedtls/library/timing.c                   \
    $(NULL)
endif

if CHIP_DEVICE_LAYER_TARGET_DARWIN
nodist_libmbedtls_a_SOURCES                                                   += \
    repo/externals/mbedtls/library/entropy_poll.c             \
    repo/externals/mbedtls/library/timing.c                   \
    $(NULL)
endif

if CHIP_DEVICE_LAYER_TARGET_LINUX
nodist_libmbedtls_a_SOURCES                                                   += \
    repo/externals/mbedtls/library/entropy_poll.c             \
    repo/externals/mbedtls/library/timing.c                   \
    $(NULL)
endif

libmbedtls_a_CPPFLAGS                                                          = \
    -I$(top_srcdir)/third_party/mbedtls/repo/externals/mbedtls/include                             \
    $(MBEDTLS_CPPFLAGS)                                                          \
    $(NULL)

endif # CHIP_WITH_MBEDTLS_INTERNAL

include $(abs_top_nlbuild_autotools_dir)/automake/post.am
