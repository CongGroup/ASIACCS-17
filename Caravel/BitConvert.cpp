#include "BitConvert.h"
#include <string>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <iostream>

using namespace std;

namespace caravel{

BitConvert::BitConvert(void)
{
}


BitConvert::~BitConvert(void)
{
}


void BitConvert::toString(char *pDes, uint32_t uiDesLen, unsigned char *pSrc, uint32_t uiSrcLen, string sFormat, uint32_t uiFormatLen)
{
    for(uint32_t uiCur = 0; uiCur < uiSrcLen; uiCur++)
    {
        snprintf(pDes + uiCur * uiFormatLen, uiFormatLen + 1, sFormat.c_str(), *pSrc++);
    }
}


void BitConvert::toString(char *pSrc, uint32_t uiLen, string &sOut, string sFormat, uint32_t uiFormatLen)
{
    uint32_t uiRealLen = uiLen * uiFormatLen;
    char *pBuf = new char[uiRealLen + 1];
    toString(pBuf, uiRealLen, (unsigned char*)pSrc, uiLen, sFormat, uiFormatLen);
    pBuf[uiRealLen] = '\0';
    sOut.assign(pBuf);
    delete[] pBuf;
}

}
