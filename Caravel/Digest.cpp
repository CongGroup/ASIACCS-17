#include "Digest.h"
#include <string>
#include <string.h>
#include <iostream>
#include <openssl/sha.h>

using namespace std;

namespace caravel{

int Digest::Sha256(string &sMsg, char *szBuf, uint32_t uiLen)
{
    return Sha256(sMsg.c_str(), sMsg.length(), szBuf, uiLen);
}

int Digest::Sha256(const char *szMsg, uint32_t uiMsgLen, char *szBuf, uint32_t uiLen)
{
    if(uiLen < SHA256_DIGEST_LENGTH)
    {
        return -1;
    }
    //May not necessary
    memset(szBuf, 0, uiLen);
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, szMsg, uiMsgLen);
    SHA256_Final(szBuf, &sha256);
    //cout<<szBuf<<endl;
    return 0;
}

int Digest::Sha256(string &sMsg, string &sRet)
{
    char szBuf[SHA256_DIGEST_LENGTH];
    Sha256(sMsg, szBuf, SHA256_DIGEST_LENGTH);
    sRet.assign(szBuf, SHA256_DIGEST_LENGTH);
    return 0;
}

}