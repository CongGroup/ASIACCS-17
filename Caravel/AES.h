#ifndef __CARAVEL_AES_H__
#define __CARAVEL_AES_H__

#include <stdint.h>
#include <string>
#include <openssl/evp.h>
#include <openssl/aes.h>

using namespace std;

namespace caravel {

	class AES
	{
	public:
		AES(void);
		~AES(void);

		static size_t CbcMaxsize(uint32_t uiSize);

		static size_t CbcEncrypt256(const char *pIn, int iInLen, char *pOut, char *pKey, char *pIv);

		static size_t CbcDecrypt256(const char *pIn, int iInLen, char *pOut, char *pKey, char *pIv);

		static size_t CbcEncrypt256(const char *pIn, int iInLen, char *pOut, char *pKey);

		static size_t CbcDecrypt256(const char *pIn, int iInLen, char *pOut, char *pKey);

	};

}

#endif
