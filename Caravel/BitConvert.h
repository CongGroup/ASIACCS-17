#ifndef __BITCONVERT_H__
#define __BITCONVERT_H__

#include <string.h>
#include <iostream>
#include <string>
#include <stdint.h>

using namespace std;

namespace caravel {

#define DEF_BITCONVERT_BUF_LEN 1000

	class BitConvert
	{

	public:
		BitConvert(void);
		~BitConvert(void);

		static void toString(char *pDes, uint32_t uiDesLen, unsigned char *pSrc, uint32_t uiSrcLen, string sFormat, uint32_t uiFormatLen);

		static void toString(char *pSrc, uint32_t uiLen, string &sOut, string sFormat, uint32_t uiFormatLen);

	};

}

#endif
