/**
  ******************************************************************************
  * @file    AES128_CTR/main.c
  * @author  MCD Application Team
  * @version V2.0.6
  * @date    25-June-2013
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2013 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include "main.h"
#include "sha256.h"

bool getHash(uint8_t *text, uint32_t sizeoftext, uint8_t *hash, uint32_t sizeofhash)
{
    BootSig bootsig;
    
    if((sizeoftext % 4 != 0) || (sizeofhash % 4 != 0))
    {
        return false;
    }
    
    BSP_BootSignature(&bootsig);
    sprintf(text, "%d", bootsig.lengthofapplication);
    BSP_BootSignatureHash(bootsig.lengthofapplication, hash, sizeofhash);
    
    return true;
}

int cryptoCheck()
{
    BYTE buf[SHA256_BLOCK_SIZE];
    SHA256_CTX ctx;
    int idx;
    int pass = 1;
 
#if 0    
    const uint8_t text1[] =  "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq";
    const uint8_t hash1[] =
    {
      0x24, 0x8d, 0x6a, 0x61, 0xd2, 0x06, 0x38, 0xb8,
      0xe5, 0xc0, 0x26, 0x93, 0x0c, 0x3e, 0x60, 0x39,
      0xa3, 0x3c, 0xe4, 0x59, 0x64, 0xff, 0x21, 0x67,
      0xf6, 0xec, 0xed, 0xd4, 0x19, 0xdb, 0x06, 0xc1
    };
#else    
    uint8_t text1[12];
    uint8_t hash1[32];
    memset(text1, 0, sizeof(text1));
    memset(hash1, 0, sizeof(hash1));
    
    getHash(text1, 12, hash1, 32);
#endif 
    
    sha256_init(&ctx);
    sha256_update(&ctx, text1, strlen(text1));
    sha256_final(&ctx, buf);

    return pass = pass && !memcmp(hash1, buf, SHA256_BLOCK_SIZE);
}

