#include "PRF.h"
#include <stdint.h>
#include <string.h>
#include <openssl/sha.h>
#include <openssl/hmac.h>

using namespace std;


namespace caravel {

    void PRF::Sha256(char *pKey, uint32_t uiKeyLen, char *pData, uint32_t uiDataLen, char *pOut, uint32_t uiOutLen)
    {
        memset(pOut, 0, uiOutLen);
        HMAC(EVP_sha256(), pKey, uiKeyLen, (unsigned char*)pData, uiDataLen, (unsigned char*)pOut, &uiOutLen);
    }

}
