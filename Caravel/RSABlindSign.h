#ifndef __RSA_BLIND_SIGN__
#define __RSA_BLIND_SIGN__

#include <string.h>
#include <string>
#include <stdint.h>
#include "gmp.h"

using namespace std;


namespace caravel {

	class RSABlindSign
	{
	public:

		//N = p * q
		//e = PubKey
		//d = PriKey 
		//r = one of random in [0...N)
		//Return H *  r ^ e

		RSABlindSign();
		~RSABlindSign();

		//Use for Client
		void ReadClientConf(const string &sN, const string &sE);

		//Use for Server
		void ReadServerConf(const string &sN, const string &sD);

		//Run on Client
		void BlindHash(char *szMsg, uint32_t uiLen, string &sOut);

		//Run on Server
		void SigGen(const string &sMsg, string &sOut);

		//Run on Client
		void RemoveBlinding(const string &sMsg, string &sOut);

		//Check on Client
		bool CheckSign(const string &sMsg);


	private:

		mpz_t m_N;
		mpz_t m_E;
		mpz_t m_D;
		mpz_t m_R;
		mpz_t m_H;

		const uint32_t cm_uiMultiAdd = 8;

	};

}

#endif
