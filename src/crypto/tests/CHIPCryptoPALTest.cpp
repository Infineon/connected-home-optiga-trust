/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#define IFX_DBG(...) printf (__VA_ARGS__) 


#include "TestCryptoLayer.h"

#include "AES_CCM_128_test_vectors.h"
#include "AES_CCM_256_test_vectors.h"
#include "ECDH_P256_test_vectors.h"
#include "HKDF_SHA256_test_vectors.h"
#include "Hash_SHA256_test_vectors.h"
#include "PBKDF2_SHA256_test_vectors.h"

#include "SPAKE2P_FE_MUL_test_vectors.h"
#include "SPAKE2P_FE_RW_test_vectors.h"
#include "SPAKE2P_HMAC_test_vectors.h"
#include "SPAKE2P_POINT_MUL_ADD_test_vectors.h"
#include "SPAKE2P_POINT_MUL_test_vectors.h"
#include "SPAKE2P_POINT_RW_test_vectors.h"
#include "SPAKE2P_POINT_VALID_test_vectors.h"
#include "SPAKE2P_RFC_test_vectors.h"

#include <crypto/CHIPCryptoPAL.h>

#include <nlunit-test.h>
#include <support/CodeUtils.h>
#include <support/TestUtils.h>

#include <stdarg.h>
#include <unistd.h> //usleep()
#include <stdlib.h>
#include <string.h>
#include <support/CodeUtils.h>
#include <support/TestUtils.h>


#include <mbedtls/ecdsa.h>


#include <optiga/optiga_util.h>
#include <optiga/optiga_crypt.h>
#include "platform_alt.h"


using namespace chip;
using namespace chip::Crypto;


extern "C" {
int trustm_init(void);
int trustm_deinit(void);
optiga_crypt_t * get_optiga_crypt(void);
};

optiga_crypt_t * me = NULL;
optiga_lib_status_t crypt_event_completed_status;
static void optiga_crypt_event_completed(void * context, optiga_lib_status_t return_status)
{
    crypt_event_completed_status = return_status;
}
mbedtls_platform_context pctx;

static void TestAES_CCM_256EncryptTestVectors(nlTestSuite * inSuite, void * inContext)
{
    int numOfTestVectors = ArraySize(ccm_test_vectors);
    int numOfTestsRan    = 0;
    for (int vectorIndex = 0; vectorIndex < numOfTestVectors; vectorIndex++)
    {
        const ccm_test_vector * vector = ccm_test_vectors[vectorIndex];
        if (vector->key_len == 32 && vector->pt_len > 0)
        {
            numOfTestsRan++;
            uint8_t out_ct[vector->ct_len];
            uint8_t out_tag[vector->tag_len];

            CHIP_ERROR err = AES_CCM_encrypt(vector->pt, vector->pt_len, vector->aad, vector->aad_len, vector->key, vector->key_len,
                                             vector->iv, vector->iv_len, out_ct, out_tag, vector->tag_len);
            bool areCTsEqual  = memcmp(out_ct, vector->ct, vector->ct_len) == 0;
            bool areTagsEqual = memcmp(out_tag, vector->tag, vector->tag_len) == 0;
            NL_TEST_ASSERT(inSuite, areCTsEqual);
            NL_TEST_ASSERT(inSuite, areTagsEqual);
            NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

            if (!areCTsEqual)
            {
                printf("\n Test %d failed due to mismatching ciphertext", vector->tcId);
            }
            if (!areTagsEqual)
            {
                printf("\n Test %d failed due to mismatching tags", vector->tcId);
            }
        }
    }
    NL_TEST_ASSERT(inSuite, numOfTestsRan > 0);
}

static void TestAES_CCM_256DecryptTestVectors(nlTestSuite * inSuite, void * inContext)
{
    int numOfTestVectors = ArraySize(ccm_test_vectors);
    int numOfTestsRan    = 0;
    for (int vectorIndex = 0; vectorIndex < numOfTestVectors; vectorIndex++)
    {
        const ccm_test_vector * vector = ccm_test_vectors[vectorIndex];
        if (vector->key_len == 32 && vector->pt_len > 0)
        {
            numOfTestsRan++;
            uint8_t out_pt[vector->pt_len];
            CHIP_ERROR err = AES_CCM_decrypt(vector->ct, vector->ct_len, vector->aad, vector->aad_len, vector->tag, vector->tag_len,
                                             vector->key, vector->key_len, vector->iv, vector->iv_len, out_pt);

            bool arePTsEqual = memcmp(vector->pt, out_pt, vector->pt_len) == 0;
            NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
            NL_TEST_ASSERT(inSuite, arePTsEqual);
            if (!arePTsEqual)
            {
                printf("\n Test %d failed due to mismatching plaintext", vector->tcId);
            }
        }
    }
    NL_TEST_ASSERT(inSuite, numOfTestsRan > 0);
}

static void TestAES_CCM_256EncryptInvalidPlainText(nlTestSuite * inSuite, void * inContext)
{
    int numOfTestVectors = ArraySize(ccm_test_vectors);
    int numOfTestsRan    = 0;
    for (int vectorIndex = 0; vectorIndex < numOfTestVectors; vectorIndex++)
    {
        const ccm_test_vector * vector = ccm_test_vectors[vectorIndex];
        if (vector->key_len == 32 && vector->pt_len > 0)
        {
            numOfTestsRan++;
            uint8_t out_ct[vector->ct_len];
            uint8_t out_tag[vector->tag_len];

            CHIP_ERROR err = AES_CCM_encrypt(vector->pt, 0, vector->aad, vector->aad_len, vector->key, vector->key_len, vector->iv,
                                             vector->iv_len, out_ct, out_tag, vector->tag_len);
            NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INVALID_ARGUMENT);
            break;
        }
    }
    NL_TEST_ASSERT(inSuite, numOfTestsRan > 0);
}

static void TestAES_CCM_256EncryptNilKey(nlTestSuite * inSuite, void * inContext)
{
    int numOfTestVectors = ArraySize(ccm_test_vectors);
    int numOfTestsRan    = 0;
    for (int vectorIndex = 0; vectorIndex < numOfTestVectors; vectorIndex++)
    {
        const ccm_test_vector * vector = ccm_test_vectors[vectorIndex];
        if (vector->key_len == 32 && vector->pt_len > 0)
        {
            numOfTestsRan++;
            uint8_t out_ct[vector->ct_len];
            uint8_t out_tag[vector->tag_len];

            CHIP_ERROR err = AES_CCM_encrypt(vector->pt, vector->pt_len, vector->aad, vector->aad_len, nullptr, 32, vector->iv,
                                             vector->iv_len, out_ct, out_tag, vector->tag_len);
            NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INVALID_ARGUMENT);
            break;
        }
    }
    NL_TEST_ASSERT(inSuite, numOfTestsRan > 0);
}

static void TestAES_CCM_256EncryptInvalidIVLen(nlTestSuite * inSuite, void * inContext)
{
    int numOfTestVectors = ArraySize(ccm_test_vectors);
    int numOfTestsRan    = 0;
    for (int vectorIndex = 0; vectorIndex < numOfTestVectors; vectorIndex++)
    {
        const ccm_test_vector * vector = ccm_test_vectors[vectorIndex];
        if (vector->key_len == 32 && vector->pt_len > 0)
        {
            numOfTestsRan++;
            uint8_t out_ct[vector->ct_len];
            uint8_t out_tag[vector->tag_len];

            CHIP_ERROR err = AES_CCM_encrypt(vector->pt, vector->pt_len, vector->aad, vector->aad_len, vector->key, vector->key_len,
                                             vector->iv, 0, out_ct, out_tag, vector->tag_len);
            NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INVALID_ARGUMENT);
            break;
        }
    }
    NL_TEST_ASSERT(inSuite, numOfTestsRan > 0);
}

static void TestAES_CCM_256EncryptInvalidTagLen(nlTestSuite * inSuite, void * inContext)
{
    int numOfTestVectors = ArraySize(ccm_test_vectors);
    int numOfTestsRan    = 0;
    for (int vectorIndex = 0; vectorIndex < numOfTestVectors; vectorIndex++)
    {
        const ccm_test_vector * vector = ccm_test_vectors[vectorIndex];
        if (vector->key_len == 32 && vector->pt_len > 0)
        {
            numOfTestsRan++;
            uint8_t out_ct[vector->ct_len];
            uint8_t out_tag[vector->tag_len];

            CHIP_ERROR err = AES_CCM_encrypt(vector->pt, vector->pt_len, vector->aad, vector->aad_len, vector->key, vector->key_len,
                                             vector->iv, vector->iv_len, out_ct, out_tag, 13);
            NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INVALID_ARGUMENT);
            break;
        }
    }
    NL_TEST_ASSERT(inSuite, numOfTestsRan > 0);
}

static void TestAES_CCM_256DecryptInvalidCipherText(nlTestSuite * inSuite, void * inContext)
{
    int numOfTestVectors = ArraySize(ccm_test_vectors);
    int numOfTestsRan    = 0;
    for (int vectorIndex = 0; vectorIndex < numOfTestVectors; vectorIndex++)
    {
        const ccm_test_vector * vector = ccm_test_vectors[vectorIndex];
        if (vector->key_len == 32 && vector->pt_len > 0)
        {
            numOfTestsRan++;
            uint8_t out_pt[vector->pt_len];
            CHIP_ERROR err = AES_CCM_decrypt(vector->ct, 0, vector->aad, vector->aad_len, vector->tag, vector->tag_len, vector->key,
                                             vector->key_len, vector->iv, vector->iv_len, out_pt);
            NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INVALID_ARGUMENT);
            break;
        }
    }
    NL_TEST_ASSERT(inSuite, numOfTestsRan > 0);
}

static void TestAES_CCM_256DecryptInvalidKey(nlTestSuite * inSuite, void * inContext)
{
    int numOfTestVectors = ArraySize(ccm_test_vectors);
    int numOfTestsRan    = 0;
    for (int vectorIndex = 0; vectorIndex < numOfTestVectors; vectorIndex++)
    {
        const ccm_test_vector * vector = ccm_test_vectors[vectorIndex];
        if (vector->key_len == 32 && vector->pt_len > 0)
        {
            numOfTestsRan++;
            uint8_t out_pt[vector->pt_len];
            CHIP_ERROR err = AES_CCM_decrypt(vector->ct, vector->ct_len, vector->aad, vector->aad_len, vector->tag, vector->tag_len,
                                             nullptr, 32, vector->iv, vector->iv_len, out_pt);
            NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INVALID_ARGUMENT);
            break;
        }
    }
    NL_TEST_ASSERT(inSuite, numOfTestsRan > 0);
}

static void TestAES_CCM_256DecryptInvalidIVLen(nlTestSuite * inSuite, void * inContext)
{
    int numOfTestVectors = ArraySize(ccm_test_vectors);
    int numOfTestsRan    = 0;
    for (int vectorIndex = 0; vectorIndex < numOfTestVectors; vectorIndex++)
    {
        const ccm_test_vector * vector = ccm_test_vectors[vectorIndex];
        if (vector->key_len == 32 && vector->pt_len > 0)
        {
            numOfTestsRan++;
            uint8_t out_pt[vector->pt_len];
            CHIP_ERROR err = AES_CCM_decrypt(vector->ct, vector->ct_len, vector->aad, vector->aad_len, vector->tag, vector->tag_len,
                                             vector->key, vector->key_len, vector->iv, 0, out_pt);
            NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INVALID_ARGUMENT);
            break;
        }
    }
    NL_TEST_ASSERT(inSuite, numOfTestsRan > 0);
}

static void TestAES_CCM_256DecryptInvalidTestVectors(nlTestSuite * inSuite, void * inContext)
{
    int numOfTestVectors = ArraySize(ccm_invalid_test_vectors);
    int numOfTestsRan    = 0;
    for (int vectorIndex = 0; vectorIndex < numOfTestVectors; vectorIndex++)
    {
        const ccm_test_vector * vector = ccm_invalid_test_vectors[vectorIndex];
        if (vector->key_len == 32 && vector->pt_len > 0)
        {
            numOfTestsRan++;
            uint8_t out_pt[vector->pt_len];
            CHIP_ERROR err = AES_CCM_decrypt(vector->ct, vector->ct_len, vector->aad, vector->aad_len, vector->tag, vector->tag_len,
                                             vector->key, vector->key_len, vector->iv, vector->iv_len, out_pt);

            bool arePTsEqual = memcmp(vector->pt, out_pt, vector->pt_len) == 0;
            NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INTERNAL);
            NL_TEST_ASSERT(inSuite, arePTsEqual == false);
        }
    }
    NL_TEST_ASSERT(inSuite, numOfTestsRan > 0);
}

static void TestAES_CCM_128EncryptTestVectors(nlTestSuite * inSuite, void * inContext)
{
    int numOfTestVectors = ArraySize(ccm_128_test_vectors);
    int numOfTestsRan    = 0;
    for (int vectorIndex = 0; vectorIndex < numOfTestVectors; vectorIndex++)
    {
        const ccm_128_test_vector * vector = ccm_128_test_vectors[vectorIndex];
        if (vector->pt_len > 0)
        {
            numOfTestsRan++;
            uint8_t out_ct[vector->ct_len];
            uint8_t out_tag[vector->tag_len];

            CHIP_ERROR err = AES_CCM_encrypt(vector->pt, vector->pt_len, vector->aad, vector->aad_len, vector->key, vector->key_len,
                                             vector->iv, vector->iv_len, out_ct, out_tag, vector->tag_len);
            NL_TEST_ASSERT(inSuite, err == vector->result);

            if (vector->result == CHIP_NO_ERROR)
            {
                bool areCTsEqual  = memcmp(out_ct, vector->ct, vector->ct_len) == 0;
                bool areTagsEqual = memcmp(out_tag, vector->tag, vector->tag_len) == 0;
                NL_TEST_ASSERT(inSuite, areCTsEqual);
                NL_TEST_ASSERT(inSuite, areTagsEqual);
                if (!areCTsEqual)
                {
                    printf("\n Test %d failed due to mismatching ciphertext\n", vector->tcId);
                }
                if (!areTagsEqual)
                {
                    printf("\n Test %d failed due to mismatching tags\n", vector->tcId);
                }
            }
        }
    }
    NL_TEST_ASSERT(inSuite, numOfTestsRan > 0);
}

static void TestAES_CCM_128DecryptTestVectors(nlTestSuite * inSuite, void * inContext)
{
    int numOfTestVectors = ArraySize(ccm_128_test_vectors);
    int numOfTestsRan    = 0;
    for (int vectorIndex = 0; vectorIndex < numOfTestVectors; vectorIndex++)
    {
        const ccm_128_test_vector * vector = ccm_128_test_vectors[vectorIndex];
        if (vector->pt_len > 0)
        {
            numOfTestsRan++;
            uint8_t out_pt[vector->pt_len];
            CHIP_ERROR err = AES_CCM_decrypt(vector->ct, vector->ct_len, vector->aad, vector->aad_len, vector->tag, vector->tag_len,
                                             vector->key, vector->key_len, vector->iv, vector->iv_len, out_pt);

            NL_TEST_ASSERT(inSuite, err == vector->result);
            if (vector->result == CHIP_NO_ERROR)
            {
                bool arePTsEqual = memcmp(vector->pt, out_pt, vector->pt_len) == 0;
                NL_TEST_ASSERT(inSuite, arePTsEqual);
                if (!arePTsEqual)
                {
                    printf("\n Test %d failed due to mismatching plaintext\n", vector->tcId);
                }
            }
        }
    }
    NL_TEST_ASSERT(inSuite, numOfTestsRan > 0);
}

static void TestAES_CCM_128EncryptInvalidPlainText(nlTestSuite * inSuite, void * inContext)
{
    int numOfTestVectors = ArraySize(ccm_128_test_vectors);
    int numOfTestsRan    = 0;
    for (int vectorIndex = 0; vectorIndex < numOfTestVectors; vectorIndex++)
    {
        const ccm_128_test_vector * vector = ccm_128_test_vectors[vectorIndex];
        if (vector->pt_len > 0)
        {
            numOfTestsRan++;
            uint8_t out_ct[vector->ct_len];
            uint8_t out_tag[vector->tag_len];

            CHIP_ERROR err = AES_CCM_encrypt(vector->pt, 0, vector->aad, vector->aad_len, vector->key, vector->key_len, vector->iv,
                                             vector->iv_len, out_ct, out_tag, vector->tag_len);
            NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INVALID_ARGUMENT);
            break;
        }
    }
    NL_TEST_ASSERT(inSuite, numOfTestsRan > 0);
}

static void TestAES_CCM_128EncryptNilKey(nlTestSuite * inSuite, void * inContext)
{
    int numOfTestVectors = ArraySize(ccm_128_test_vectors);
    int numOfTestsRan    = 0;
    for (int vectorIndex = 0; vectorIndex < numOfTestVectors; vectorIndex++)
    {
        const ccm_128_test_vector * vector = ccm_128_test_vectors[vectorIndex];
        if (vector->pt_len > 0)
        {
            numOfTestsRan++;
            uint8_t out_ct[vector->ct_len];
            uint8_t out_tag[vector->tag_len];

            CHIP_ERROR err = AES_CCM_encrypt(vector->pt, vector->pt_len, vector->aad, vector->aad_len, nullptr, 0, vector->iv,
                                             vector->iv_len, out_ct, out_tag, vector->tag_len);
            NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INVALID_ARGUMENT);
            break;
        }
    }
    NL_TEST_ASSERT(inSuite, numOfTestsRan > 0);
}

static void TestAES_CCM_128EncryptInvalidIVLen(nlTestSuite * inSuite, void * inContext)
{
    int numOfTestVectors = ArraySize(ccm_128_test_vectors);
    int numOfTestsRan    = 0;
    for (int vectorIndex = 0; vectorIndex < numOfTestVectors; vectorIndex++)
    {
        const ccm_128_test_vector * vector = ccm_128_test_vectors[vectorIndex];
        if (vector->pt_len > 0)
        {
            numOfTestsRan++;
            uint8_t out_ct[vector->ct_len];
            uint8_t out_tag[vector->tag_len];

            CHIP_ERROR err = AES_CCM_encrypt(vector->pt, vector->pt_len, vector->aad, vector->aad_len, vector->key, vector->key_len,
                                             vector->iv, 0, out_ct, out_tag, vector->tag_len);
            NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INVALID_ARGUMENT);
            break;
        }
    }
    NL_TEST_ASSERT(inSuite, numOfTestsRan > 0);
}

static void TestAES_CCM_128EncryptInvalidTagLen(nlTestSuite * inSuite, void * inContext)
{
    int numOfTestVectors = ArraySize(ccm_128_test_vectors);
    int numOfTestsRan    = 0;
    for (int vectorIndex = 0; vectorIndex < numOfTestVectors; vectorIndex++)
    {
        const ccm_128_test_vector * vector = ccm_128_test_vectors[vectorIndex];
        if (vector->pt_len > 0)
        {
            numOfTestsRan++;
            uint8_t out_ct[vector->ct_len];
            uint8_t out_tag[vector->tag_len];

            CHIP_ERROR err = AES_CCM_encrypt(vector->pt, vector->pt_len, vector->aad, vector->aad_len, vector->key, vector->key_len,
                                             vector->iv, vector->iv_len, out_ct, out_tag, 13);
            NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INVALID_ARGUMENT);
            break;
        }
    }
    NL_TEST_ASSERT(inSuite, numOfTestsRan > 0);
}

static void TestAES_CCM_128DecryptInvalidCipherText(nlTestSuite * inSuite, void * inContext)
{
    int numOfTestVectors = ArraySize(ccm_128_test_vectors);
    int numOfTestsRan    = 0;
    for (int vectorIndex = 0; vectorIndex < numOfTestVectors; vectorIndex++)
    {
        const ccm_128_test_vector * vector = ccm_128_test_vectors[vectorIndex];
        if (vector->pt_len > 0)
        {
            numOfTestsRan++;
            uint8_t out_pt[vector->pt_len];
            CHIP_ERROR err = AES_CCM_decrypt(vector->ct, 0, vector->aad, vector->aad_len, vector->tag, vector->tag_len, vector->key,
                                             vector->key_len, vector->iv, vector->iv_len, out_pt);
            NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INVALID_ARGUMENT);
            break;
        }
    }
    NL_TEST_ASSERT(inSuite, numOfTestsRan > 0);
}

static void TestAES_CCM_128DecryptInvalidKey(nlTestSuite * inSuite, void * inContext)
{
    int numOfTestVectors = ArraySize(ccm_128_test_vectors);
    int numOfTestsRan    = 0;
    for (int vectorIndex = 0; vectorIndex < numOfTestVectors; vectorIndex++)
    {
        const ccm_128_test_vector * vector = ccm_128_test_vectors[vectorIndex];
        if (vector->pt_len > 0)
        {
            numOfTestsRan++;
            uint8_t out_pt[vector->pt_len];
            CHIP_ERROR err = AES_CCM_decrypt(vector->ct, vector->ct_len, vector->aad, vector->aad_len, vector->tag, vector->tag_len,
                                             nullptr, 0, vector->iv, vector->iv_len, out_pt);
            NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INVALID_ARGUMENT);
            break;
        }
    }
    NL_TEST_ASSERT(inSuite, numOfTestsRan > 0);
}

static void TestAES_CCM_128DecryptInvalidIVLen(nlTestSuite * inSuite, void * inContext)
{
    int numOfTestVectors = ArraySize(ccm_128_test_vectors);
    int numOfTestsRan    = 0;
    for (int vectorIndex = 0; vectorIndex < numOfTestVectors; vectorIndex++)
    {
        const ccm_128_test_vector * vector = ccm_128_test_vectors[vectorIndex];
        if (vector->pt_len > 0)
        {
            numOfTestsRan++;
            uint8_t out_pt[vector->pt_len];
            CHIP_ERROR err = AES_CCM_decrypt(vector->ct, vector->ct_len, vector->aad, vector->aad_len, vector->tag, vector->tag_len,
                                             vector->key, vector->key_len, vector->iv, 0, out_pt);
            NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INVALID_ARGUMENT);
            break;
        }
    }
    NL_TEST_ASSERT(inSuite, numOfTestsRan > 0);
}

static void TestHash_SHA256(nlTestSuite * inSuite, void * inContext)
{
    int numOfTestCases     = ArraySize(hash_sha256_test_vectors);
    int numOfTestsExecuted = 0;

    for (numOfTestsExecuted = 0; numOfTestsExecuted < numOfTestCases; numOfTestsExecuted++)
    {
        hash_sha256_vector v = hash_sha256_test_vectors[numOfTestsExecuted];
        uint8_t out_buffer[kSHA256_Hash_Length];
        Hash_SHA256(v.data, v.data_length, out_buffer);
        bool success = memcmp(v.hash, out_buffer, sizeof(out_buffer)) == 0;
        NL_TEST_ASSERT(inSuite, success);
    }
    NL_TEST_ASSERT(inSuite, numOfTestsExecuted == ArraySize(hash_sha256_test_vectors));
}

static void TestHash_SHA256_Stream(nlTestSuite * inSuite, void * inContext)
{
    int numOfTestCases     = ArraySize(hash_sha256_test_vectors);
    int numOfTestsExecuted = 0;
    CHIP_ERROR error       = CHIP_NO_ERROR;

    for (numOfTestsExecuted = 0; numOfTestsExecuted < numOfTestCases; numOfTestsExecuted++)
    {
        hash_sha256_vector v = hash_sha256_test_vectors[numOfTestsExecuted];
        const uint8_t * data = v.data;
        size_t data_length   = v.data_length;
        uint8_t out_buffer[kSHA256_Hash_Length];

        Hash_SHA256_stream sha256;

        error = sha256.Begin();
        NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);

        // Split data into 3 random streams.
        for (int i = 0; i < 2; ++i)
        {
            size_t rand_data_length = static_cast<unsigned int>(rand()) % (data_length + 1);

            error = sha256.AddData(data, rand_data_length);
            NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);

            data += rand_data_length;
            data_length -= rand_data_length;
        }

        error = sha256.AddData(data, data_length);
        NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);

        error = sha256.Finish(out_buffer);
        NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);

        bool success = memcmp(v.hash, out_buffer, sizeof(out_buffer)) == 0;
        NL_TEST_ASSERT(inSuite, success);
    }
    NL_TEST_ASSERT(inSuite, numOfTestsExecuted == ArraySize(hash_sha256_test_vectors));
}

static void TestHKDF_SHA256(nlTestSuite * inSuite, void * inContext)
{
    int numOfTestCases     = ArraySize(hkdf_sha256_test_vectors);
    int numOfTestsExecuted = 0;

    for (numOfTestsExecuted = 0; numOfTestsExecuted < numOfTestCases; numOfTestsExecuted++)
    {
        hkdf_sha256_vector v = hkdf_sha256_test_vectors[numOfTestsExecuted];
        size_t out_length    = v.output_key_material_length;
        uint8_t out_buffer[out_length];
        HKDF_SHA256(v.initial_key_material, v.initial_key_material_length, v.salt, v.salt_length, v.info, v.info_length, out_buffer,
                    v.output_key_material_length);
        bool success = memcmp(v.output_key_material, out_buffer, out_length) == 0;
        NL_TEST_ASSERT(inSuite, success);
    }
    NL_TEST_ASSERT(inSuite, numOfTestsExecuted == 3);
}

static void TestDRBG_InvalidInputs(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    error            = DRBG_get_bytes(nullptr, 10);
    NL_TEST_ASSERT(inSuite, error == CHIP_ERROR_INVALID_ARGUMENT);
    error = CHIP_NO_ERROR;
    uint8_t buffer[5];
    error = DRBG_get_bytes(buffer, 0);
    NL_TEST_ASSERT(inSuite, error == CHIP_ERROR_INVALID_ARGUMENT);
}

static void TestDRBG_Output(nlTestSuite * inSuite, void * inContext)
{
    // No good way to unit test a DRBG. Just validate that we get out something
    // Test 256 byte segmentation for OPTIGA-M implementation
    CHIP_ERROR error     = CHIP_ERROR_INVALID_ARGUMENT;
    uint8_t out_buf[270]  = { 0 };
    uint8_t orig_buf[270] = { 0 };

    error = DRBG_get_bytes(out_buf, sizeof(out_buf));
    NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, memcmp(out_buf, orig_buf, sizeof(out_buf)) != 0);
}

static void cleanup(void) {
    // destroy crypt instances
    if (me != NULL) {
       (void)optiga_crypt_destroy(me);
    }
}

//static void GenKeyPairOnOptiga(nlTestSuite * inSuite, uint16_t privkey_oid, unsigned char* public_key_out, uint16_t expected_public_key_len_in) {

//Generate keypair on OPTIGA M, Overwrite key values in keypair (Hacked to be made public for this PoC)
static void GenKeyPairOnOptiga(nlTestSuite * inSuite, uint16_t privkey_oid, P256Keypair* keypairIn) {
    //To test OptigaM signing we must generate private key in HW - cannot import private key
    //use OptigaM function - not testing mbedTLS version of key generation
    //export public key for testing
    optiga_lib_status_t crypt_sync_status = OPTIGA_CRYPT_ERROR;

    unsigned char hex_public_key_buffer[68];
    uint16_t public_key_buffer_length = sizeof(hex_public_key_buffer);
    unsigned char hex_private_key_buffer[32] = {0};
    uint16_t private_key_buffer_length = sizeof(hex_private_key_buffer);
    
    mbedtls_ecp_keypair * mbed_keypair = reinterpret_cast<mbedtls_ecp_keypair *> (&(keypairIn->mKeypair));
 
    //Set Private key to OID
    ((uint16_t * ) hex_private_key_buffer)[0] = privkey_oid;

    IFX_DBG("IFX_>optiga_crypt_ecc_generate_keypair()...\n");
    me = NULL;
    me = optiga_crypt_create(0, optiga_crypt_event_completed, NULL);
    if (NULL == me)  {
        cleanup();
        NL_TEST_ASSERT(inSuite, 0);
    }

    // Reset the status variable (updated via callback, when requested operation is finished, or timeout)
    crypt_event_completed_status = OPTIGA_LIB_BUSY;
    crypt_sync_status = optiga_crypt_ecc_generate_keypair(me,
                                                          OPTIGA_ECC_CURVE_NIST_P_256,
                                                          (optiga_key_usage_t)( OPTIGA_KEY_USAGE_KEY_AGREEMENT | OPTIGA_KEY_USAGE_AUTHENTICATION ),
                                                          FALSE,
                                                          &privkey_oid,
                                                          hex_public_key_buffer,
                                                          &public_key_buffer_length);
    if(OPTIGA_LIB_SUCCESS != crypt_sync_status) {
        cleanup();
        NL_TEST_ASSERT(inSuite, 0);
    }
    // Wait loop - Need to Wait until the optiga_crypt_ecdsa_verify is completed
    while (OPTIGA_LIB_BUSY == crypt_event_completed_status) {
        //pal_os_timer_delay_in_milliseconds(10);
        usleep(5*1000);
    }
    if(crypt_event_completed_status!= OPTIGA_LIB_SUCCESS) {
        cleanup();
        NL_TEST_ASSERT(inSuite, 0);
    }
		//OPTIGA M prepends public key with 3 DER format bytes
    IFX_DBG("IFX_>optiga_crypt_ecc_generate_keypair buffer length=>%d ...\n", public_key_buffer_length);
    for(uint16_t i=0; i < public_key_buffer_length; i++) {
      IFX_DBG(" %0x", hex_public_key_buffer[i]);
    }
    IFX_DBG("\n");
    IFX_DBG("IFX_>Expected Public Key length => %d \n", (*keypairIn).mPublicKey.Length());
    NL_TEST_ASSERT(inSuite, public_key_buffer_length == ((*keypairIn).mPublicKey.Length() +3) );
    
    //Copy public key to Public Key member
    memcpy(Uint8::to_uchar((*keypairIn).mPublicKey), &hex_public_key_buffer[3], (*keypairIn).mPublicKey.Length()); 
    //public & private to keypair member
    CHIP_ERROR err=  mbedtls_ecp_point_read_binary(&mbed_keypair->grp, &mbed_keypair->Q, &hex_public_key_buffer[3], (*keypairIn).mPublicKey.Length());
    NL_TEST_ASSERT(inSuite, err  == CHIP_NO_ERROR);
    err = mbedtls_mpi_read_binary(&mbed_keypair->d, hex_private_key_buffer, private_key_buffer_length);
    NL_TEST_ASSERT(inSuite, err  == CHIP_NO_ERROR);
 
    cleanup();
}


static void TestECDSA_Signing_SHA256(nlTestSuite * inSuite, void * inContext)
{
    const char * msg  = "Hello World!";
    size_t msg_length = strlen((const char *) msg);

    P256Keypair keypair;
    NL_TEST_ASSERT(inSuite, keypair.Initialize() == CHIP_NO_ERROR);

    GenKeyPairOnOptiga(inSuite, OPTIGA_KEY_ID_E0F1, &keypair);

    P256ECDSASignature signature;
    CHIP_ERROR signing_error = keypair.ECDSA_sign_msg((const uint8_t *) msg, msg_length, signature);
    NL_TEST_ASSERT(inSuite, signing_error == CHIP_NO_ERROR);

    CHIP_ERROR validation_error = keypair.Pubkey().ECDSA_validate_msg_signature((const uint8_t *) msg, msg_length, signature);
    NL_TEST_ASSERT(inSuite, validation_error == CHIP_NO_ERROR);
}

static void TestECDSA_ValidationFailsDifferentMessage(nlTestSuite * inSuite, void * inContext)
{
    const char * msg  = "Hello World!";
    size_t msg_length = strlen((const char *) msg);

    P256Keypair keypair;
    NL_TEST_ASSERT(inSuite, keypair.Initialize() == CHIP_NO_ERROR);

    GenKeyPairOnOptiga(inSuite, OPTIGA_KEY_ID_E0F1, &keypair);
 
    P256ECDSASignature signature;
    CHIP_ERROR signing_error = keypair.ECDSA_sign_msg((const uint8_t *) msg, msg_length, signature);
    NL_TEST_ASSERT(inSuite, signing_error == CHIP_NO_ERROR);

    const char * diff_msg  = "NOT Hello World!";
    size_t diff_msg_length = strlen((const char *) msg);
    CHIP_ERROR validation_error =
        keypair.Pubkey().ECDSA_validate_msg_signature((const uint8_t *) diff_msg, diff_msg_length, signature);
    NL_TEST_ASSERT(inSuite, validation_error != CHIP_NO_ERROR);
}

static void TestECDSA_ValidationFailIncorrectSignature(nlTestSuite * inSuite, void * inContext)
{
    const char * msg  = "Hello World!";
    size_t msg_length = strlen((const char *) msg);

    P256Keypair keypair;
    NL_TEST_ASSERT(inSuite, keypair.Initialize() == CHIP_NO_ERROR);

    GenKeyPairOnOptiga(inSuite, OPTIGA_KEY_ID_E0F1, &keypair);
    
    P256ECDSASignature signature;
    CHIP_ERROR signing_error = keypair.ECDSA_sign_msg((const uint8_t *) msg, msg_length, signature);
    NL_TEST_ASSERT(inSuite, signing_error == CHIP_NO_ERROR);
    signature[0] = static_cast<uint8_t>(~signature[0]); // Flipping bits should invalidate the signature.

    CHIP_ERROR validation_error = keypair.Pubkey().ECDSA_validate_msg_signature((const uint8_t *) msg, msg_length, signature);
    NL_TEST_ASSERT(inSuite, validation_error == CHIP_ERROR_INVALID_SIGNATURE);
}

static void TestECDSA_SigningInvalidParams(nlTestSuite * inSuite, void * inContext)
{
    const uint8_t * msg = (uint8_t *) "Hello World!";
    size_t msg_length   = strlen((const char *) msg);

    P256Keypair keypair;
    NL_TEST_ASSERT(inSuite, keypair.Initialize() == CHIP_NO_ERROR);

    GenKeyPairOnOptiga(inSuite, OPTIGA_KEY_ID_E0F1, &keypair);

    P256ECDSASignature signature;
    CHIP_ERROR signing_error = keypair.ECDSA_sign_msg(nullptr, msg_length, signature);
    NL_TEST_ASSERT(inSuite, signing_error == CHIP_ERROR_INVALID_ARGUMENT);
    signing_error = CHIP_NO_ERROR;

    signing_error = keypair.ECDSA_sign_msg(msg, 0, signature);
    NL_TEST_ASSERT(inSuite, signing_error == CHIP_ERROR_INVALID_ARGUMENT);
    signing_error = CHIP_NO_ERROR;
}

static void TestECDSA_ValidationInvalidParam(nlTestSuite * inSuite, void * inContext)
{
    const char * msg  = "Hello World!";
    size_t msg_length = strlen((const char *) msg);

    P256Keypair keypair;
    NL_TEST_ASSERT(inSuite, keypair.Initialize() == CHIP_NO_ERROR);

    GenKeyPairOnOptiga(inSuite, OPTIGA_KEY_ID_E0F1, &keypair);

    P256ECDSASignature signature;
    CHIP_ERROR signing_error = keypair.ECDSA_sign_msg((const uint8_t *) msg, msg_length, signature);
    NL_TEST_ASSERT(inSuite, signing_error == CHIP_NO_ERROR);

    CHIP_ERROR validation_error = keypair.Pubkey().ECDSA_validate_msg_signature(nullptr, msg_length, signature);
    NL_TEST_ASSERT(inSuite, validation_error == CHIP_ERROR_INVALID_ARGUMENT);
    validation_error = CHIP_NO_ERROR;

    validation_error = keypair.Pubkey().ECDSA_validate_msg_signature((const uint8_t *) msg, 0, signature);
    NL_TEST_ASSERT(inSuite, validation_error == CHIP_ERROR_INVALID_ARGUMENT);
    validation_error = CHIP_NO_ERROR;
}

static void TestECDH_EstablishSecret(nlTestSuite * inSuite, void * inContext)
{
    P256Keypair keypair1;
    NL_TEST_ASSERT(inSuite, keypair1.Initialize() == CHIP_NO_ERROR);
    GenKeyPairOnOptiga(inSuite, 0xe100, &keypair1);

    P256Keypair keypair2;
    NL_TEST_ASSERT(inSuite, keypair2.Initialize() == CHIP_NO_ERROR);
    GenKeyPairOnOptiga(inSuite, 0xe101, &keypair2);
    

    P256ECDHDerivedSecret out_secret1;
    out_secret1[0] = 0;

    P256ECDHDerivedSecret out_secret2;
    out_secret2[0] = 1;

    CHIP_ERROR error = CHIP_NO_ERROR;
    NL_TEST_ASSERT(inSuite,
                   memcmp(Uint8::to_uchar(out_secret1), Uint8::to_uchar(out_secret2), out_secret1.Capacity()) !=
                       0); // Validate that buffers are indeed different.

    error = keypair2.ECDH_derive_secret(keypair1.Pubkey(), out_secret1);
    NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);

    error = keypair1.ECDH_derive_secret(keypair2.Pubkey(), out_secret2);
    NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);

    bool signature_lengths_match = out_secret1.Length() == out_secret2.Length();
    NL_TEST_ASSERT(inSuite, signature_lengths_match);

    bool signatures_match = (memcmp(Uint8::to_uchar(out_secret1), Uint8::to_uchar(out_secret2), out_secret1.Length()) == 0);
    NL_TEST_ASSERT(inSuite, signatures_match);
}

#if CHIP_CRYPTO_OPENSSL
static void TestAddEntropySources(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR error = add_entropy_source(test_entropy_source, nullptr, 10);
    NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);
    uint8_t buffer[5];
    NL_TEST_ASSERT(inSuite, DRBG_get_bytes(buffer, sizeof(buffer)) == CHIP_NO_ERROR);
}
#endif

#if CHIP_CRYPTO_MBEDTLS
static void TestAddEntropySources(nlTestSuite * inSuite, void * inContext)
{
    //Adding Entropy Sources doesn't make sense on OPTIGA-M, just run several tests & verify output different each time
   uint8_t buffer1[315];
   uint8_t buffer2[315];
  for (int i = 0; i < 20; i++)
    {
        (void) DRBG_get_bytes(buffer1, sizeof(buffer1));
        NL_TEST_ASSERT(inSuite, memcmp(buffer1, buffer2, sizeof(buffer1)) != 0);
        (void) DRBG_get_bytes(buffer2, sizeof(buffer1));
        NL_TEST_ASSERT(inSuite, memcmp(buffer1, buffer2, sizeof(buffer1)) != 0);
 
    }

}
#endif

static void TestPBKDF2_SHA256_TestVectors(nlTestSuite * inSuite, void * inContext)
{
    int numOfTestVectors = ArraySize(pbkdf2_sha256_test_vectors);
    int numOfTestsRan    = 0;
    for (int vectorIndex = 0; vectorIndex < numOfTestVectors; vectorIndex++)
    {
        const pbkdf2_test_vector * vector = pbkdf2_sha256_test_vectors[vectorIndex];
        if (vector->plen > 0)
        {
            numOfTestsRan++;
            uint8_t out_key[vector->key_len];

            CHIP_ERROR err =
                pbkdf2_sha256(vector->password, vector->plen, vector->salt, vector->slen, vector->iter, vector->key_len, out_key);
            NL_TEST_ASSERT(inSuite, err == vector->result);

            if (vector->result == CHIP_NO_ERROR)
            {
                NL_TEST_ASSERT(inSuite, memcmp(out_key, vector->key, vector->key_len) == 0);
            }
        }
    }
    NL_TEST_ASSERT(inSuite, numOfTestsRan > 0);
}

static void TestP256_Keygen(nlTestSuite * inSuite, void * inContext)
{
    P256Keypair keypair;
    NL_TEST_ASSERT(inSuite, keypair.Initialize() == CHIP_NO_ERROR);

    GenKeyPairOnOptiga(inSuite, OPTIGA_KEY_ID_E0F1, &keypair);

    const char * msg         = "Test Message for Keygen";
    const uint8_t * test_msg = Uint8::from_const_char(msg);
    size_t msglen            = strlen(msg);

    P256ECDSASignature test_sig;
    NL_TEST_ASSERT(inSuite, keypair.ECDSA_sign_msg(test_msg, msglen, test_sig) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, keypair.Pubkey().ECDSA_validate_msg_signature(test_msg, msglen, test_sig) == CHIP_NO_ERROR);
}

static void TestCSR_Gen(nlTestSuite * inSuite, void * inContext)
{
    uint8_t csr[kMAX_CSR_Length];
    size_t length = sizeof(csr);

    P256Keypair keypair;
    NL_TEST_ASSERT(inSuite, keypair.Initialize() == CHIP_NO_ERROR);
    
    GenKeyPairOnOptiga(inSuite, OPTIGA_KEY_ID_E0F1, &keypair);

    NL_TEST_ASSERT(inSuite, keypair.NewCertificateSigningRequest(csr, length) == CHIP_NO_ERROR);
}

static void TestKeypair_Serialize(nlTestSuite * inSuite, void * inContext)
{
    P256Keypair keypair;
    NL_TEST_ASSERT(inSuite, keypair.Initialize() == CHIP_NO_ERROR);
    
    GenKeyPairOnOptiga(inSuite, OPTIGA_KEY_ID_E0F3, &keypair);

    P256SerializedKeypair serialized;
    NL_TEST_ASSERT(inSuite, keypair.Serialize(serialized) == CHIP_NO_ERROR);

    P256Keypair keypair_dup;
    NL_TEST_ASSERT(inSuite, keypair_dup.Deserialize(serialized) == CHIP_NO_ERROR);

    const char * msg         = "Test Message for Keygen";
    const uint8_t * test_msg = Uint8::from_const_char(msg);
    size_t msglen            = strlen(msg);

    P256ECDSASignature test_sig;
    NL_TEST_ASSERT(inSuite, keypair.ECDSA_sign_msg(test_msg, msglen, test_sig) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, keypair_dup.Pubkey().ECDSA_validate_msg_signature(test_msg, msglen, test_sig) == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, keypair_dup.ECDSA_sign_msg(test_msg, msglen, test_sig) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, keypair.Pubkey().ECDSA_validate_msg_signature(test_msg, msglen, test_sig) == CHIP_NO_ERROR);
}

static void TestSPAKE2P_spake2p_FEMul(nlTestSuite * inSuite, void * inContext)
{
    uint8_t fe_out[kMAX_FE_Length];

    int numOfTestVectors = ArraySize(fe_mul_tvs);
    int numOfTestsRan    = 0;
    for (int vectorIndex = 0; vectorIndex < numOfTestVectors; vectorIndex++)
    {
        const struct spake2p_fe_mul_tv * vector = fe_mul_tvs[vectorIndex];

        Spake2p_P256_SHA256_HKDF_HMAC spake2p;
        CHIP_ERROR err = spake2p.Init(nullptr, 0);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = spake2p.FELoad(vector->fe1, vector->fe1_len, spake2p.w0);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = spake2p.FELoad(vector->fe2, vector->fe2_len, spake2p.w1);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = spake2p.FEMul(spake2p.xy, spake2p.w0, spake2p.w1);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = spake2p.FEWrite(spake2p.xy, fe_out, sizeof(fe_out));
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        NL_TEST_ASSERT(inSuite, memcmp(fe_out, vector->fe_out, vector->fe_out_len) == 0);
        numOfTestsRan += 1;
    }
    NL_TEST_ASSERT(inSuite, numOfTestsRan > 0);
    NL_TEST_ASSERT(inSuite, numOfTestsRan == numOfTestVectors);
}

static void TestSPAKE2P_spake2p_FELoadWrite(nlTestSuite * inSuite, void * inContext)
{
    uint8_t fe_out[kMAX_FE_Length];

    int numOfTestVectors = ArraySize(fe_rw_tvs);
    int numOfTestsRan    = 0;
    for (int vectorIndex = 0; vectorIndex < numOfTestVectors; vectorIndex++)
    {
        const struct spake2p_fe_rw_tv * vector = fe_rw_tvs[vectorIndex];

        Spake2p_P256_SHA256_HKDF_HMAC spake2p;
        CHIP_ERROR err = spake2p.Init(nullptr, 0);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = spake2p.FELoad(vector->fe_in, vector->fe_in_len, spake2p.w0);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = spake2p.FEWrite(spake2p.w0, fe_out, sizeof(fe_out));
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        NL_TEST_ASSERT(inSuite, memcmp(fe_out, vector->fe_out, vector->fe_out_len) == 0);
        numOfTestsRan += 1;
    }
    NL_TEST_ASSERT(inSuite, numOfTestsRan > 0);
    NL_TEST_ASSERT(inSuite, numOfTestsRan == numOfTestVectors);
}

static void TestSPAKE2P_spake2p_Mac(nlTestSuite * inSuite, void * inContext)
{
    uint8_t mac[kMAX_Hash_Length];

    int numOfTestVectors = ArraySize(hmac_tvs);
    int numOfTestsRan    = 0;
    for (int vectorIndex = 0; vectorIndex < numOfTestVectors; vectorIndex++)
    {
        const struct spake2p_hmac_tv * vector = hmac_tvs[vectorIndex];

        Spake2p_P256_SHA256_HKDF_HMAC spake2p;
        CHIP_ERROR err = spake2p.Init(nullptr, 0);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = spake2p.Mac(vector->key, vector->key_len, vector->input, vector->input_len, mac);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        NL_TEST_ASSERT(inSuite, memcmp(mac, vector->output, vector->output_len) == 0);

        err = spake2p.MacVerify(vector->key, vector->key_len, vector->output, vector->output_len, vector->input, vector->input_len);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        numOfTestsRan += 1;
    }
    NL_TEST_ASSERT(inSuite, numOfTestsRan > 0);
    NL_TEST_ASSERT(inSuite, numOfTestsRan == numOfTestVectors);
}

static void TestSPAKE2P_spake2p_PointMul(nlTestSuite * inSuite, void * inContext)
{
    uint8_t output[kMAX_Point_Length];
    size_t out_len = sizeof(output);

    int numOfTestVectors = ArraySize(point_mul_tvs);
    int numOfTestsRan    = 0;
    for (int vectorIndex = 0; vectorIndex < numOfTestVectors; vectorIndex++)
    {
        out_len                                    = sizeof(output);
        const struct spake2p_point_mul_tv * vector = point_mul_tvs[vectorIndex];

        Spake2p_P256_SHA256_HKDF_HMAC spake2p;
        CHIP_ERROR err = spake2p.Init(nullptr, 0);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = spake2p.PointLoad(vector->point, vector->point_len, spake2p.L);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = spake2p.FELoad(vector->scalar, vector->scalar_len, spake2p.w0);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = spake2p.PointMul(spake2p.X, spake2p.L, spake2p.w0);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = spake2p.PointWrite(spake2p.X, output, out_len);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        NL_TEST_ASSERT(inSuite, memcmp(output, vector->out_point, vector->out_point_len) == 0);

        numOfTestsRan += 1;
    }
    NL_TEST_ASSERT(inSuite, numOfTestsRan > 0);
    NL_TEST_ASSERT(inSuite, numOfTestsRan == numOfTestVectors);
}

static void TestSPAKE2P_spake2p_PointMulAdd(nlTestSuite * inSuite, void * inContext)
{
    uint8_t output[kMAX_Point_Length];
    size_t out_len = sizeof(output);

    int numOfTestVectors = ArraySize(point_muladd_tvs);
    int numOfTestsRan    = 0;
    for (int vectorIndex = 0; vectorIndex < numOfTestVectors; vectorIndex++)
    {
        out_len                                       = sizeof(output);
        const struct spake2p_point_muladd_tv * vector = point_muladd_tvs[vectorIndex];

        Spake2p_P256_SHA256_HKDF_HMAC spake2p;
        CHIP_ERROR err = spake2p.Init(nullptr, 0);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = spake2p.PointLoad(vector->point1, vector->point1_len, spake2p.X);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = spake2p.PointLoad(vector->point2, vector->point2_len, spake2p.Y);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = spake2p.FELoad(vector->scalar1, vector->scalar1_len, spake2p.w0);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = spake2p.FELoad(vector->scalar2, vector->scalar2_len, spake2p.w1);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = spake2p.PointAddMul(spake2p.L, spake2p.X, spake2p.w0, spake2p.Y, spake2p.w1);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = spake2p.PointWrite(spake2p.L, output, out_len);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        NL_TEST_ASSERT(inSuite, memcmp(output, vector->out_point, vector->out_point_len) == 0);

        numOfTestsRan += 1;
    }
    NL_TEST_ASSERT(inSuite, numOfTestsRan > 0);
    NL_TEST_ASSERT(inSuite, numOfTestsRan == numOfTestVectors);
}

static void TestSPAKE2P_spake2p_PointLoadWrite(nlTestSuite * inSuite, void * inContext)
{
    uint8_t output[kMAX_Point_Length];
    size_t out_len = sizeof(output);

    int numOfTestVectors = ArraySize(point_rw_tvs);
    int numOfTestsRan    = 0;
    for (int vectorIndex = 0; vectorIndex < numOfTestVectors; vectorIndex++)
    {
        out_len                                   = sizeof(output);
        const struct spake2p_point_rw_tv * vector = point_rw_tvs[vectorIndex];

        Spake2p_P256_SHA256_HKDF_HMAC spake2p;
        CHIP_ERROR err = spake2p.Init(nullptr, 0);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = spake2p.PointLoad(vector->point, vector->point_len, spake2p.L);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = spake2p.PointWrite(spake2p.L, output, out_len);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        NL_TEST_ASSERT(inSuite, memcmp(output, vector->point, vector->point_len) == 0);

        numOfTestsRan += 1;
    }
    NL_TEST_ASSERT(inSuite, numOfTestsRan > 0);
    NL_TEST_ASSERT(inSuite, numOfTestsRan == numOfTestVectors);
}

static void TestSPAKE2P_spake2p_PointIsValid(nlTestSuite * inSuite, void * inContext)
{
    int numOfTestVectors = ArraySize(point_valid_tvs);
    int numOfTestsRan    = 0;
    for (int vectorIndex = 0; vectorIndex < numOfTestVectors; vectorIndex++)
    {
        const struct spake2p_point_valid_tv * vector = point_valid_tvs[vectorIndex];

        Spake2p_P256_SHA256_HKDF_HMAC spake2p;
        CHIP_ERROR err = spake2p.Init(nullptr, 0);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = spake2p.PointLoad(vector->point, vector->point_len, spake2p.L);
        // The underlying implementation may (i.e. should) check for validity when loading a point. Let's catch this case.
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR || vector->valid == 0);

        err = spake2p.PointIsValid(spake2p.L);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR || vector->valid == 0);

        numOfTestsRan += 1;
    }
    NL_TEST_ASSERT(inSuite, numOfTestsRan > 0);
    NL_TEST_ASSERT(inSuite, numOfTestsRan == numOfTestVectors);
}

// We need to "generate" specific field elements
// to do so we need to override the specific method
class Test_Spake2p_P256_SHA256_HKDF_HMAC : public Spake2p_P256_SHA256_HKDF_HMAC
{
public:
    CHIP_ERROR TestSetFE(const uint8_t * fe_in, size_t fe_in_len)
    {
        if (fe_in_len > kMAX_FE_Length)
        {
            return CHIP_ERROR_INTERNAL;
        }
        memcpy(fe, fe_in, fe_in_len);
        fe_len = fe_in_len;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR FEGenerate(void * feout) override { return FELoad(fe, fe_len, feout); }

private:
    uint8_t fe[kMAX_FE_Length];
    size_t fe_len;
};

static void TestSPAKE2P_RFC(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    uint8_t L[kMAX_Point_Length];
    size_t L_len = sizeof(L);
    uint8_t Z[kMAX_Point_Length];
    uint8_t V[kMAX_Point_Length];
    uint8_t X[kMAX_Point_Length];
    size_t X_len = sizeof(X);
    uint8_t Y[kMAX_Point_Length];
    size_t Y_len = sizeof(Y);
    uint8_t Pverifier[kMAX_Hash_Length];
    size_t Pverifier_len = sizeof(Pverifier);
    uint8_t Vverifier[kMAX_Hash_Length];
    size_t Vverifier_len = sizeof(Vverifier);
    uint8_t VKe[kMAX_Hash_Length];
    size_t VKe_len = sizeof(VKe);
    uint8_t PKe[kMAX_Hash_Length];
    size_t PKe_len = sizeof(PKe);

    int numOfTestVectors = ArraySize(rfc_tvs);
    int numOfTestsRan    = 0;
    // static_assert(sizeof(Spake2p_Context) < 1024, "Allocate more bytes for Spake2p Context");
    // printf("Sizeof spake2pcontext %lu\n", sizeof(Spake2p_Context));
    // printf("Sizeof mbedtls_sha256_context %lu\n", sizeof(mbedtls_sha256_context));
    // printf("Sizeof SHA256_CTX %lu\n", sizeof(SHA256_CTX));
    for (int vectorIndex = 0; vectorIndex < numOfTestVectors; vectorIndex++)
    {
        const struct spake2p_rfc_tv * vector = rfc_tvs[vectorIndex];

        Test_Spake2p_P256_SHA256_HKDF_HMAC Verifier;
        Test_Spake2p_P256_SHA256_HKDF_HMAC Prover;

        // First start the prover
        error = Prover.Init(vector->context, vector->context_len);
        NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);

        error = Prover.BeginProver(vector->prover_identity, vector->prover_identity_len, vector->verifier_identity,
                                   vector->verifier_identity_len, vector->w0, vector->w0_len, vector->w1, vector->w1_len);
        NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);

        // Monkey patch the generated x coordinate
        error = Prover.TestSetFE(vector->x, vector->x_len);
        NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);

        // Compute the first round and send it to the verifier
        X_len = sizeof(X);
        error = Prover.ComputeRoundOne(X, &X_len);
        NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, X_len == vector->X_len);
        NL_TEST_ASSERT(inSuite, memcmp(X, vector->X, vector->X_len) == 0);

        // Start up the verifier
        error = Verifier.Init(vector->context, vector->context_len);
        NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);

        // First pre-compute L (accessories with dynamic setup codes will do this)
        L_len = sizeof(L);
        error = Verifier.ComputeL(L, &L_len, vector->w1, vector->w1_len);
        NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, L_len == vector->L_len);
        NL_TEST_ASSERT(inSuite, memcmp(L, vector->L, vector->L_len) == 0);

        // Start up the verifier
        error = Verifier.BeginVerifier(vector->verifier_identity, vector->verifier_identity_len, vector->prover_identity,
                                       vector->prover_identity_len, vector->w0, vector->w0_len, L, L_len);
        NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);

        // Monkey patch the generated y coordinate
        error = Verifier.TestSetFE(vector->y, vector->y_len);
        NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);

        // Compute the first round and send it to the prover
        Y_len = sizeof(Y);
        error = Verifier.ComputeRoundOne(Y, &Y_len);
        NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, Y_len == vector->Y_len);
        NL_TEST_ASSERT(inSuite, memcmp(Y, vector->Y, vector->Y_len) == 0);

        // Compute the second round to also send to the prover
        Vverifier_len = sizeof(Vverifier);
        error         = Verifier.ComputeRoundTwo(X, X_len, Vverifier, &Vverifier_len);
        NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, Vverifier_len == vector->MAC_KcB_len);
        NL_TEST_ASSERT(inSuite, memcmp(Vverifier, vector->MAC_KcB, vector->MAC_KcB_len) == 0);

        error = Verifier.PointWrite(Verifier.Z, Z, kP256_Point_Length);
        NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, memcmp(Z, vector->Z, vector->Z_len) == 0);

        error = Verifier.PointWrite(Verifier.V, V, kP256_Point_Length);
        NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, memcmp(V, vector->V, vector->V_len) == 0);

        // Now the prover computes round 2
        Pverifier_len = sizeof(Pverifier);
        error         = Prover.ComputeRoundTwo(Y, Y_len, Pverifier, &Pverifier_len);
        NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, Pverifier_len == vector->MAC_KcA_len);
        NL_TEST_ASSERT(inSuite, memcmp(Pverifier, vector->MAC_KcA, vector->MAC_KcA_len) == 0);

        error = Prover.PointWrite(Verifier.Z, Z, kP256_Point_Length);
        NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, memcmp(Z, vector->Z, vector->Z_len) == 0);

        error = Prover.PointWrite(Verifier.V, V, kP256_Point_Length);
        NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, memcmp(V, vector->V, vector->V_len) == 0);

        // Both sides now confirm the keys they received
        error = Prover.KeyConfirm(Vverifier, Vverifier_len);
        NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);
        error = Verifier.KeyConfirm(Pverifier, Pverifier_len);
        NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);

        PKe_len = sizeof(PKe);
        error   = Prover.GetKeys(PKe, &PKe_len);
        NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, PKe_len == vector->Ke_len);
        NL_TEST_ASSERT(inSuite, memcmp(PKe, vector->Ke, vector->Ke_len) == 0);

        VKe_len = sizeof(VKe);
        error   = Verifier.GetKeys(VKe, &VKe_len);
        NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, VKe_len == vector->Ke_len);
        NL_TEST_ASSERT(inSuite, memcmp(VKe, vector->Ke, vector->Ke_len) == 0);

        numOfTestsRan += 1;
    }
    NL_TEST_ASSERT(inSuite, numOfTestsRan > 0);
    NL_TEST_ASSERT(inSuite, numOfTestsRan == numOfTestVectors);
}

namespace chip {
namespace Logging {
void __attribute__((weak)) LogV(uint8_t module, uint8_t category, const char * format, va_list argptr)
{
    (void) module, (void) category;
    vfprintf(stderr, format, argptr);
}
} // namespace Logging
} // namespace chip

/**
 *   Test Suite. It lists all the test functions.
 */

static const nlTest sTests[] = {

    NL_TEST_DEF("Test encrypting AES-CCM-128 test vectors", TestAES_CCM_128EncryptTestVectors),
    NL_TEST_DEF("Test decrypting AES-CCM-128 test vectors", TestAES_CCM_128DecryptTestVectors),
    NL_TEST_DEF("Test encrypting AES-CCM-128 invalid plain text", TestAES_CCM_128EncryptInvalidPlainText),
    NL_TEST_DEF("Test encrypting AES-CCM-128 using nil key", TestAES_CCM_128EncryptNilKey),
    NL_TEST_DEF("Test encrypting AES-CCM-128 using invalid IV", TestAES_CCM_128EncryptInvalidIVLen),
    NL_TEST_DEF("Test encrypting AES-CCM-128 using invalid tag", TestAES_CCM_128EncryptInvalidTagLen),
    NL_TEST_DEF("Test decrypting AES-CCM-128 invalid ct", TestAES_CCM_128DecryptInvalidCipherText),
    NL_TEST_DEF("Test decrypting AES-CCM-128 invalid key", TestAES_CCM_128DecryptInvalidKey),
    NL_TEST_DEF("Test decrypting AES-CCM-128 invalid IV", TestAES_CCM_128DecryptInvalidIVLen),
    NL_TEST_DEF("Test encrypting AES-CCM-256 test vectors", TestAES_CCM_256EncryptTestVectors),
    NL_TEST_DEF("Test decrypting AES-CCM-256 test vectors", TestAES_CCM_256DecryptTestVectors),
    NL_TEST_DEF("Test encrypting AES-CCM-256 invalid plain text", TestAES_CCM_256EncryptInvalidPlainText),
    NL_TEST_DEF("Test encrypting AES-CCM-256 using nil key", TestAES_CCM_256EncryptNilKey),
    NL_TEST_DEF("Test encrypting AES-CCM-256 using invalid IV", TestAES_CCM_256EncryptInvalidIVLen),
    NL_TEST_DEF("Test encrypting AES-CCM-256 using invalid tag", TestAES_CCM_256EncryptInvalidTagLen),
    NL_TEST_DEF("Test decrypting AES-CCM-256 invalid ct", TestAES_CCM_256DecryptInvalidCipherText),
    NL_TEST_DEF("Test decrypting AES-CCM-256 invalid key", TestAES_CCM_256DecryptInvalidKey),
    NL_TEST_DEF("Test decrypting AES-CCM-256 invalid IV", TestAES_CCM_256DecryptInvalidIVLen),
    NL_TEST_DEF("Test decrypting AES-CCM-256 invalid vectors", TestAES_CCM_256DecryptInvalidTestVectors),
    NL_TEST_DEF("Test ECDSA signing and validation using SHA256", TestECDSA_Signing_SHA256),
    NL_TEST_DEF("Test ECDSA signature validation fail - Different msg", TestECDSA_ValidationFailsDifferentMessage),
    NL_TEST_DEF("Test ECDSA signature validation fail - Different signature", TestECDSA_ValidationFailIncorrectSignature),
    NL_TEST_DEF("Test ECDSA sign msg invalid parameters", TestECDSA_SigningInvalidParams),
    NL_TEST_DEF("Test ECDSA signature validation invalid parameters", TestECDSA_ValidationInvalidParam),
    NL_TEST_DEF("Test Hash SHA 256", TestHash_SHA256),
    NL_TEST_DEF("Test Hash SHA 256 Stream", TestHash_SHA256_Stream),
    NL_TEST_DEF("Test HKDF SHA 256", TestHKDF_SHA256),
    NL_TEST_DEF("Test DRBG invalid inputs", TestDRBG_InvalidInputs),
    NL_TEST_DEF("Test DRBG output", TestDRBG_Output),
    NL_TEST_DEF("Test ECDH derive shared secret", TestECDH_EstablishSecret),
    NL_TEST_DEF("Test adding entropy sources", TestAddEntropySources),
    NL_TEST_DEF("Test PBKDF2 SHA256", TestPBKDF2_SHA256_TestVectors),
    NL_TEST_DEF("Test P256 Keygen", TestP256_Keygen),
    NL_TEST_DEF("Test CSR Generation", TestCSR_Gen),
    NL_TEST_DEF("Test Keypair Serialize", TestKeypair_Serialize),
    NL_TEST_DEF("Test Spake2p_spake2p FEMul", TestSPAKE2P_spake2p_FEMul),
    NL_TEST_DEF("Test Spake2p_spake2p FELoad/FEWrite", TestSPAKE2P_spake2p_FELoadWrite),
    NL_TEST_DEF("Test Spake2p_spake2p Mac", TestSPAKE2P_spake2p_Mac),
    NL_TEST_DEF("Test Spake2p_spake2p PointMul", TestSPAKE2P_spake2p_PointMul),
    NL_TEST_DEF("Test Spake2p_spake2p PointMulAdd", TestSPAKE2P_spake2p_PointMulAdd),
    NL_TEST_DEF("Test Spake2p_spake2p PointLoad/PointWrite", TestSPAKE2P_spake2p_PointLoadWrite),
    NL_TEST_DEF("Test Spake2p_spake2p PointIsValid", TestSPAKE2P_spake2p_PointIsValid),
    NL_TEST_DEF("Test Spake2+ against RFC test vectors", TestSPAKE2P_RFC),
    NL_TEST_SENTINEL()
};

int TestCHIPCryptoPAL(void)
{
    // clang-format off
    if (mbedtls_platform_setup(&pctx) != 0) { IFX_DBG("Error!! - Could not init OPTIGA M \n"); exit(-1);}; 
    nlTestSuite theSuite =
    {
        "CHIP Crypto PAL tests",
        &sTests[0],
        nullptr,
        nullptr
    };
    // clang-format on
    // Run test suit againt one context.
    nlTestRunner(&theSuite, nullptr);
    mbedtls_platform_teardown(&pctx);
    return (nlTestRunnerStats(&theSuite));
}

CHIP_REGISTER_TEST_SUITE(TestCHIPCryptoPAL)