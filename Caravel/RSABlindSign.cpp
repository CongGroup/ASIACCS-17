#include "RSABlindSign.h"
#include "Digest.h"
#include "gmp.h"
#include <string.h>
#include <openssl/sha.h>
#include "BitConvert.h"

using namespace std;


namespace caravel{

RSABlindSign::RSABlindSign()
{
    mpz_inits(m_N, m_E, m_D, m_R, m_H, NULL);
}

RSABlindSign::~RSABlindSign()
{
    mpz_clears(m_N, m_E, m_D, m_R, m_H, NULL);
}

//Use for Client
void RSABlindSign::ReadClientConf(const string &sN, const string &sE)
{
    mpz_init_set_str(m_N, sN.c_str(), 10);
    mpz_init_set_str(m_E, sE.c_str(), 10);
}

//Use for Server
void RSABlindSign::ReadServerConf(const string &sN, const string &sD)
{
    mpz_init_set_str(m_N, sN.c_str(), 10);
    mpz_init_set_str(m_D, sD.c_str(), 10);
}

//Run on Client
void RSABlindSign::BlindHash(char *szMsg, uint32_t uiLen, string &sOut)
{
    //Hash the Msg
    char szBuf[SHA256_DIGEST_LENGTH * cm_uiMultiAdd + 1];
    char *szRet = NULL;
    //no need to memset, for Digest.Sha256 will do it.
    string arStr[] = {"0", "1", "2", "3", "4", "5", "6", "7"};
    string sMsg;
    for(uint32_t uiCur = 0; uiCur < cm_uiMultiAdd; uiCur++)
    {
        sMsg = arStr[uiCur].append(szMsg, uiLen);
        Digest::Sha256(sMsg, szBuf + uiCur * SHA256_DIGEST_LENGTH, SHA256_DIGEST_LENGTH);
    }
    szBuf[sizeof(szBuf) - 1] = '\0';
    BitConvert::toString(szBuf, sizeof(szBuf) - 1, sMsg, "%.2x", 2);
    //cout<<"8 X SHA256(Content)  "<<sMsg<<endl<<endl;
    mpz_t mMsg;
    mpz_init_set_str(mMsg, sMsg.c_str(), 16);
    mpz_mod(m_H, mMsg, m_N);
    //gmp_printf("mMsg : %s\n", mpz_get_str(NULL, 16, mMsg));
    //Generate a random R
    gmp_randstate_t rstate;
    gmp_randinit_mt(rstate);
    mpz_urandomm(m_R, rstate, m_N);
    gmp_randclear(rstate);
    //Blind it
        
    //gmp_printf("mR : %Zd\n", m_R);

    mpz_t mRet, mPowm, mMul;
    mpz_inits(mRet, mPowm, mMul, NULL);
    //r^e mod n
    mpz_powm(mPowm, m_R, m_E, m_N);
    //h * r^e mod n
    mpz_mul(mMul, mPowm, m_H);

    //gmp_printf("mMul : %Zd\n", mMul);

    mpz_mod(mRet, mMul, m_N);
    //Prepare return
    uint32_t uiSize = mpz_sizeinbase(mRet, 10) + 2;
    szRet = new char[uiSize];
    memset(szRet, 0, uiSize);
    mpz_get_str(szRet, 10, mRet);
    sOut.assign(szRet);
    mpz_clears(mMsg, mRet, mPowm, mMul, NULL);
    //cout<<"BlindHash : "<<sOut<<endl<<endl;
    delete[] szRet;
}

//Run on Server
void RSABlindSign::SigGen(const string &sMsg, string &sOut)
{
    mpz_t mMsg;
    mpz_init_set_str(mMsg, sMsg.c_str(), 10);
    mpz_t mRet;
    mpz_init(mRet);
    //m^d mod n
    mpz_powm(mRet, mMsg, m_D, m_N);

    uint32_t uiSize = mpz_sizeinbase(mRet, 10) + 2;
    char *szRet = new char[uiSize];
    memset(szRet, 0, uiSize);
    mpz_get_str(szRet, 10, mRet);
    sOut.assign(szRet);
    cout<<"Get a Request and SigGen : "<<sOut<<endl<<endl;
    mpz_clears(mMsg, mRet, NULL);
    delete[] szRet;
}

//Run on Client
void RSABlindSign::RemoveBlinding(const string &sMsg, string &sOut)
{
    mpz_t mMsg;
    mpz_init_set_str(mMsg, sMsg.c_str(), 10);
    mpz_t mRet, mRinv, mMul;
    mpz_inits(mRet, mRinv, mMul, NULL);
    //m / r mod n
    mpz_invert(mRinv, m_R, m_N);

    mpz_mul(mMul, mMsg, mRinv);

    mpz_mod(mRet, mMul, m_N);

    uint32_t uiSize = mpz_sizeinbase(mRet, 10) + 2;
    char *szRet = new char[uiSize];
    memset(szRet, 0, uiSize);
    mpz_get_str(szRet, 10, mRet);

    sOut.assign(szRet);
    mpz_clears(mMsg, mRet, mRinv, mMul, NULL);
    delete[] szRet;

}

//Check on Client
bool RSABlindSign::CheckSign(const string &sMsg)
{
    mpz_t mMsg, mHash;
    mpz_init(mHash);
    mpz_init_set_str(mMsg, sMsg.c_str(), 10);
    //h^d^e ?== h
    mpz_powm(mHash, mMsg, m_E, m_N);

        
    /*
    gmp_printf("mMsg : %Zd\n", mMsg);
    gmp_printf("m_E : %Zd\n", m_E);
    gmp_printf("m_N : %Zd\n", m_N);

    gmp_printf("mHash : %Zd\n", mHash);

        
    gmp_printf("m_H : %Zd\n", m_H);
    */

    bool bRet = (0 == mpz_cmp(mHash, m_H));
    mpz_clears(mMsg, mHash, NULL);
    return bRet;
}


}
