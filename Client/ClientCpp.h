#ifndef __CLIENT_CPP_H__
#define __CLIENT_CPP_H__

#include <iostream>
#include <string.h>
#include <string>
#include <stdint.h>
#include <vector>
#include <map>

#include "../Caravel/ThriftAdapt.h"

#include "../fastore/OREHelper.h"

#include "SimConHash.h"
#include "TProxyService.h"


#include "DemoConfig.h"

#define SHA256_DIGEST_LENGTH 32
#define INDEX_STOP_FLAG 1234567890

//This is the max size of <Table + Column + Value>
#define DEF_MAX_TDVALUE_SIZE 10240

using namespace proxyserver;
using namespace caravel;
using namespace std;

class ClientCpp
{
public:
    ClientCpp();
    ~ClientCpp();

    void InitKey(string stKey, uint32_t uiBlockSizeInBits/* = DEF_BLOCK_SIZE_INBIT*/);

    void Open();
    void Close();

    void Init(string arIP[], uint16_t arPort[], uint32_t uiServerNum);

    void InitExample(uint32_t uiServerNum = DEMO_SERVER_NUM);

    void Get(string &_retVal, string stTable, string stKey, string stCol);

	void Put(string stTable, string stKey, string stCol, char *pVal, uint32_t uiLen, bool bColumnIndex, bool bEqualIndexV1, bool bEqualIndexV2, bool bOreIndex);

    void GetCol(vector<string> &_retVal, string stTable, string stCol, uint32_t uiNum);

    uint32_t Search(string stTable, string stCol, char *pVal, uint32_t uiLen, bool bIndexV2);

    uint32_t RangeQuery(string stTable, string stCol, int iCmp, uint32_t uiVal);

	void PutPlainText(string stTable, string stKey, string stCol, char *pVal, uint32_t uiLen, bool bColumnIndex, bool bEqualIndexV1, bool bEqualIndexV2, bool bOreIndex);

	uint32_t SearchPlainText(string stTable, string stCol, char *pVal, uint32_t uiLen, bool bIndexV2);

	uint32_t RangeQueryPlainText(string stTable, string stCol, int iCmp, uint32_t uiVal);

private:

    void m_Decrypt(string &strCiphertext, string &strPlaintext);
    void m_Encrypt(string &strCiphertext, char *pPlaintext, uint32_t uiPlaintextLen);

    //counter map for common column get all
    map<string, uint32_t> m_mapColumnCounter[DEMO_SERVER_NUM];

    //counter map for equal test V1
    map<string, uint32_t> m_mapEqualV1Counter[DEMO_SERVER_NUM];

    //counter map for equal test V2
    map<string, uint32_t> m_mapEqualV2Counter[DEMO_SERVER_NUM];

    //counter map for order test
    map<string, uint32_t> m_mapOrderCounter[DEMO_SERVER_NUM];

    //For trapdoor to Key
    //<Table + Column + Key> || PK1 => Trapdoor
    char m_szPk1[SHA256_DIGEST_LENGTH];

    //For trapdoor to column counter
    char m_szPk2[SHA256_DIGEST_LENGTH];

    //For Index Column
    //<Table + Column + PK3> || Counter => ColumnKey
    char m_szPk3[SHA256_DIGEST_LENGTH];

    //For Index Column Value Mask
    //<Table + Column + PK4> || Counter => Mask
    char m_szPk4[SHA256_DIGEST_LENGTH];

    //For protect data by AES CBC 256
    char m_szPk5[SHA256_DIGEST_LENGTH];


    /*For new experiment*/

    /* For Equal Index Strategy I */
    //<Table + Column + PK6> || Counter
    char m_szPk6[SHA256_DIGEST_LENGTH];

    //<Table + Column + Value + PK7> || Counter
    char m_szPk7[SHA256_DIGEST_LENGTH];

    /* For Equal Index Strategy II */
    //<Table + Column + Value + PK8> || Counter
    char m_szPk8[SHA256_DIGEST_LENGTH];

    //<Table + Column + Value + PK9> || Counter
    char m_szPk9[SHA256_DIGEST_LENGTH];

    /* For ORE Index */
    //<Table + Column + PK10> || Counter
    char m_szPk10[SHA256_DIGEST_LENGTH];

    //Memory for store Table + Column + Value
    char m_szTDValue[DEF_MAX_TDVALUE_SIZE];

    SimConHash<ThriftAdapt<TProxyServiceClient>* > m_SimConHash;

    OREHelper m_oreHelper;

    char m_szOreLeft[DEF_ORELEFT_MAXSIZE];

    //Not only ORE Right
    //Trapdoor<KEY> + ORE_RIGHT
    char m_szOreRight[DEF_ORERIGHT_MAXSIZE + SHA256_DIGEST_LENGTH];



};





#endif
