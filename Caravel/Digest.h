#ifndef __DIGEST_H__
#define __DIGEST_H__

#include <string>
#include <stdint.h>
#include <openssl/sha.h>

using namespace std;

namespace caravel {

	class Digest
	{
	public:

		static int Sha256(string &sMsg, char *szBuf, uint32_t uiLen);

		static int Sha256(const char *szMsg, uint32_t uiMsgLen, char *szBuf, uint32_t uiLen);

		static int Sha256(string &sMsg, string &sRet);
	};

}

#endif
