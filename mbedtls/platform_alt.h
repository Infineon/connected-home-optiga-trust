#ifndef MBEDTLS_PLATFORM_ALT_H
#define MBEDTLS_PLATFORM_ALT_H

#include <stdlib.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

//Platform initalization context
typedef struct mbedtls_platform_context
{
  int contextNumber; 
  int *contextNumberPtr; 
}
mbedtls_platform_context;

int mbedtls_platform_setup(mbedtls_platform_context * ctx	);
void mbedtls_platform_teardown( mbedtls_platform_context * ctx);

#ifdef __cplusplus
}
#endif

#endif /* MBEDTLS_PLATFORM_ALT_H */