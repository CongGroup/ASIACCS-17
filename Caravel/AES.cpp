#include "AES.h"

#include <stdint.h>
#include <string>
#include <string.h>
#include <openssl/aes.h>

#include <openssl/err.h>
#include <iostream>

#include <stdlib.h>
#include <time.h>

using namespace std;

namespace caravel{

AES::AES(void)
{
}


AES::~AES(void)
{
}


size_t AES::CbcMaxsize(uint32_t uiSize)
{
    uint32_t uiMod = uiSize % AES_BLOCK_SIZE;
    if(uiMod == 0)
    {
        //IV + Item
        return uiSize + 2 * AES_BLOCK_SIZE;
    }
    else
    {
        return uiSize + (AES_BLOCK_SIZE - uiMod) + AES_BLOCK_SIZE;
    }
}

size_t AES::CbcEncrypt256(const char *pIn, int iInLen, char *pOut, char *pKey)
{
    char *pData = pOut + AES_BLOCK_SIZE;
    memset(pOut, 0, AES_BLOCK_SIZE);
    uint32_t *pui = (uint32_t*)pOut;
    srand(time(NULL));
    for(uint32_t uiCur = 0; uiCur < AES_BLOCK_SIZE / sizeof(uint32_t); uiCur++)
    {
        *pui++ = rand();
    }
    return AES_BLOCK_SIZE + CbcEncrypt256(pIn, iInLen, pData, pKey, pOut);
}


size_t AES::CbcDecrypt256(const char *pIn, int iInLen, char *pOut, char *pKey)
{
    const char *pData = pIn + AES_BLOCK_SIZE;
    return CbcDecrypt256(pData, iInLen - AES_BLOCK_SIZE, pOut, pKey, (char*)pIn);
}



size_t AES::CbcEncrypt256(const char *pIn, int iInLen, char *pOut, char *pKey, char *pIv)
{
    EVP_CIPHER_CTX *ctx;

    if(!(ctx = EVP_CIPHER_CTX_new())) 
    {
        //Error for create
        return 0;
    }

    //Init Encrypt
    if(1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, pKey, pIv))
    {
        //Error
        return 0;
    }
    int iLen, iCipherLen;
    if(1 != EVP_EncryptUpdate(ctx, pOut, &iLen, pIn, iInLen))
    {
        return 0;
    }
    iCipherLen = iLen;
    if(1 != EVP_EncryptFinal_ex(ctx, pOut + iCipherLen, &iLen))
    {
        return 0;
    }

    iCipherLen += iLen;

    EVP_CIPHER_CTX_free(ctx);

    return iCipherLen;
}


size_t AES::CbcDecrypt256(const char *pIn, int iInLen, char *pOut, char *pKey, char *pIv)
{
    EVP_CIPHER_CTX *ctx;

    if(!(ctx = EVP_CIPHER_CTX_new())) 
    {
        //Error for create
        return 0;
    }
    //Init Encrypt
    if(1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, pKey, pIv))
    {
        return 0;
    }
    int iLen, iPlainLen;
    if(1 != EVP_DecryptUpdate(ctx, pOut, &iLen, pIn, iInLen))
    {
        return 0;
    }
    iPlainLen = iLen;


    if(1 != EVP_DecryptFinal_ex(ctx, pOut + iPlainLen, &iLen))
    {
        return 0;
    }

    iPlainLen += iLen;

    EVP_CIPHER_CTX_free(ctx);

    return iPlainLen;

}



}
