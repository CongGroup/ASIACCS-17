#include "ClientCpp.h"

#include <iostream>
#include <string.h>
#include <string>
#include <stdint.h>
#include <vector>

#include "../Caravel/Digest.h"
#include "../Caravel/PRF.h"
#include "../Caravel/AES.h"
#include "../fastore/OREHelper.h"

using namespace std;
using namespace caravel;

ClientCpp::ClientCpp()
{
}

ClientCpp::~ClientCpp()
{
}

void ClientCpp::InitKey(string stKey, uint32_t uiBlockSizeInBits)
{

	//init security key

	string stMkey;

	stMkey = stKey + "PK1";
	Digest::Sha256(stMkey, m_szPk1, SHA256_DIGEST_LENGTH);

	stMkey = stKey + "PK2";
	Digest::Sha256(stMkey, m_szPk2, SHA256_DIGEST_LENGTH);

	stMkey = stKey + "PK3";
	Digest::Sha256(stMkey, m_szPk3, SHA256_DIGEST_LENGTH);

	stMkey = stKey + "PK4";
	Digest::Sha256(stMkey, m_szPk4, SHA256_DIGEST_LENGTH);

	stMkey = stKey + "PK5";
	Digest::Sha256(stMkey, m_szPk5, SHA256_DIGEST_LENGTH);

	stMkey = stKey + "PK6";
	Digest::Sha256(stMkey, m_szPk6, SHA256_DIGEST_LENGTH);

	stMkey = stKey + "PK7";
	Digest::Sha256(stMkey, m_szPk7, SHA256_DIGEST_LENGTH);

	stMkey = stKey + "PK8";
	Digest::Sha256(stMkey, m_szPk8, SHA256_DIGEST_LENGTH);

	stMkey = stKey + "PK9";
	Digest::Sha256(stMkey, m_szPk9, SHA256_DIGEST_LENGTH);

	stMkey = stKey + "PK10";
	Digest::Sha256(stMkey, m_szPk10, SHA256_DIGEST_LENGTH);

	m_oreHelper.Init(stKey + "ORE", uiBlockSizeInBits);

}

void ClientCpp::Get(string &_retVal, string stTable, string stKey, string stCol)
{
	//Generate Trapdoor

	string stTmp = stTable + stKey + stCol;
	char szTd[SHA256_DIGEST_LENGTH];
	PRF::Sha256(m_szPk1, SHA256_DIGEST_LENGTH, (char*)stTmp.c_str(), stTmp.length(), szTd, SHA256_DIGEST_LENGTH);

	//Set Trapdoor to String
	string strTrapdoor;
	strTrapdoor.assign(szTd, SHA256_DIGEST_LENGTH);

	//Check the Router to choose a server to send request.
	uint32_t uiKey = *(uint32_t*)szTd;

	ThriftAdapt<TProxyServiceClient> *pThriftAdapt = *(m_SimConHash.Query(uiKey));
	TProxyServiceClient* pClient = pThriftAdapt->GetClient();

	//Send the request to remote server
	string strRet;
	pClient->ProxyGet(strRet, strTrapdoor);

	//If strRet.length == 0 then the key does not exist.
	if (strRet.length() != 0)
	{
		//Decrypt the data
		m_Decrypt(strRet, _retVal);
	}


}

void ClientCpp::Put(string stTable, string stKey, string stCol, char *pVal, uint32_t uiLen, bool bColumnIndex, bool bEqualIndexV1, bool bEqualIndexV2, bool bOreIndex)
{
	//Generate the SendKey
	string stTmp = stTable + stKey + stCol;
	char szTd[SHA256_DIGEST_LENGTH];
	PRF::Sha256(m_szPk1, SHA256_DIGEST_LENGTH, (char*)stTmp.c_str(), stTmp.length(), szTd, SHA256_DIGEST_LENGTH);
	string strTrapdoor;
	strTrapdoor.assign(szTd, SHA256_DIGEST_LENGTH);

	//Just for constant empty string
	string strEmpty = "";

	//Encrypt the data using AES
	string strSendVal;
	m_Encrypt(strSendVal, pVal, uiLen);

	//Check the Dispatcher to choose a server to send request.
	uint32_t uiKey = *(uint32_t*)szTd ^ *(uint32_t*)(szTd + 4) ^ *(uint32_t*)(szTd + 8) ^ *(uint32_t*)(szTd + 16) ^ *(uint32_t*)(szTd + 20) ^ *(uint32_t*)(szTd + 24) ^ *(uint32_t*)(szTd + 28);

	int iID;
	ThriftAdapt<TProxyServiceClient> *pThriftAdapt = *(m_SimConHash.Query(uiKey, &iID));
	TProxyServiceClient* pClient = pThriftAdapt->GetClient();

	//If also insert the Index
	string strIndexVal = "";
	string strIndexTrapdoor = "";

	//Column Get Every
	if (bColumnIndex)
	{

		//Generate the Index Trapdoor
		stTmp = stTable + stCol;

		const uint32_t kIndexValSize = SHA256_DIGEST_LENGTH + sizeof(uint32_t);
		char szIndexTd[kIndexValSize];
		PRF::Sha256(m_szPk3, SHA256_DIGEST_LENGTH, (char*)stTmp.c_str(), stTmp.length(), szIndexTd, SHA256_DIGEST_LENGTH);
		strIndexTrapdoor.assign(szIndexTd, SHA256_DIGEST_LENGTH);

		//Get the Counter
		uint32_t uiCounter = m_mapColumnCounter[iID][stTmp]++;

		PRF::Sha256((char*)&uiCounter, sizeof(uiCounter), (char*)strIndexTrapdoor.c_str(), strIndexTrapdoor.length(), szIndexTd, SHA256_DIGEST_LENGTH);
		strIndexTrapdoor.assign(szIndexTd, SHA256_DIGEST_LENGTH);

		//Generate 
		PRF::Sha256(m_szPk4, SHA256_DIGEST_LENGTH, (char*)stTmp.c_str(), stTmp.length(), szIndexTd, SHA256_DIGEST_LENGTH);
		strIndexVal.assign(szIndexTd, SHA256_DIGEST_LENGTH);

		PRF::Sha256((char*)&uiCounter, sizeof(uiCounter), (char*)strIndexVal.c_str(), strIndexVal.length(), szIndexTd, SHA256_DIGEST_LENGTH);
		//padding the last 4 bytes
		*(uint32_t*)(szIndexTd + SHA256_DIGEST_LENGTH) = 0;

		//for the first 4 bytes flag
		*(uint32_t*)szIndexTd ^= INDEX_STOP_FLAG;
		//for the last 32 bytes
		const char *pTrapdoor = strTrapdoor.c_str();
		char *pMask = szIndexTd + sizeof(uint32_t);
		for (uint32_t uiCur = 0; uiCur < SHA256_DIGEST_LENGTH; uiCur++)
		{
			pMask[uiCur] ^= pTrapdoor[uiCur];
		}

		strIndexVal.assign(szIndexTd, kIndexValSize);

		pClient->ProxyPut(strIndexTrapdoor, strIndexVal, strEmpty, strEmpty);

	}

	//For equal index strategy I
	if (bEqualIndexV1)
	{

		//Generate the Index Trapdoor
		stTmp = stTable + stCol;

		//Half for check, half for key back
		const uint32_t kIndexValSize = SHA256_DIGEST_LENGTH + SHA256_DIGEST_LENGTH;
		char szIndexTd[kIndexValSize];
		PRF::Sha256(m_szPk6, SHA256_DIGEST_LENGTH, (char*)stTmp.c_str(), stTmp.length(), szIndexTd, SHA256_DIGEST_LENGTH);
		strIndexTrapdoor.assign(szIndexTd, SHA256_DIGEST_LENGTH);


		//Get the Counter
		uint32_t uiCounter = m_mapEqualV1Counter[iID][stTmp]++;

		PRF::Sha256((char*)&uiCounter, sizeof(uiCounter), (char*)strIndexTrapdoor.c_str(), strIndexTrapdoor.length(), szIndexTd, SHA256_DIGEST_LENGTH);
		strIndexTrapdoor.assign(szIndexTd, SHA256_DIGEST_LENGTH);

		//Generate 

		uint32_t uiTDValueLen = stTmp.length();
		memcpy(m_szTDValue, stTmp.c_str(), stTmp.length());
		memcpy(m_szTDValue + uiTDValueLen, pVal, uiLen);
		uiTDValueLen += uiLen;

		PRF::Sha256(m_szPk7, SHA256_DIGEST_LENGTH, m_szTDValue, uiTDValueLen, szIndexTd, SHA256_DIGEST_LENGTH);
		strIndexVal.assign(szIndexTd, SHA256_DIGEST_LENGTH);

		PRF::Sha256((char*)&uiCounter, sizeof(uiCounter), (char*)strIndexVal.c_str(), strIndexVal.length(), szIndexTd, SHA256_DIGEST_LENGTH);
		//Copy the trapdoor into the last 32 bits
		memcpy(szIndexTd + SHA256_DIGEST_LENGTH, strTrapdoor.c_str(), SHA256_DIGEST_LENGTH);

		strIndexVal.assign(szIndexTd, kIndexValSize);

		pClient->ProxyPut(strIndexTrapdoor, strIndexVal, strEmpty, strEmpty);

	}

	//For equal index strategy II
	if (bEqualIndexV2)
	{

		//Generate the Index Trapdoor
		stTmp = stTable + stCol;

		uint32_t uiTDValueLen = stTmp.length();
		memcpy(m_szTDValue, stTmp.c_str(), stTmp.length());
		memcpy(m_szTDValue + uiTDValueLen, pVal, uiLen);
		uiTDValueLen += uiLen;

		//Half for check, half for key back
		const uint32_t kIndexValSize = SHA256_DIGEST_LENGTH + SHA256_DIGEST_LENGTH;
		char szIndexTd[kIndexValSize];

		PRF::Sha256(m_szPk8, SHA256_DIGEST_LENGTH, m_szTDValue, uiTDValueLen, szIndexTd, SHA256_DIGEST_LENGTH);
		strIndexTrapdoor.assign(szIndexTd, SHA256_DIGEST_LENGTH);

		//Get the Counter
		uint32_t uiCounter = m_mapEqualV2Counter[iID][strIndexTrapdoor]++;

		PRF::Sha256((char*)&uiCounter, sizeof(uiCounter), (char*)strIndexTrapdoor.c_str(), strIndexTrapdoor.length(), szIndexTd, SHA256_DIGEST_LENGTH);
		strIndexTrapdoor.assign(szIndexTd, SHA256_DIGEST_LENGTH);

		//Generate 

		PRF::Sha256(m_szPk9, SHA256_DIGEST_LENGTH, m_szTDValue, uiTDValueLen, szIndexTd, SHA256_DIGEST_LENGTH);
		strIndexVal.assign(szIndexTd, SHA256_DIGEST_LENGTH);

		PRF::Sha256((char*)&uiCounter, sizeof(uiCounter), (char*)strIndexVal.c_str(), strIndexVal.length(), szIndexTd, SHA256_DIGEST_LENGTH);
		//Copy the trapdoor into the last 32 bits
		memcpy(szIndexTd + SHA256_DIGEST_LENGTH, strTrapdoor.c_str(), SHA256_DIGEST_LENGTH);

		strIndexVal.assign(szIndexTd, kIndexValSize);

		pClient->ProxyPut(strIndexTrapdoor, strIndexVal, strEmpty, strEmpty);

	}

	if (bOreIndex)
	{
		//Generate the Index Trapdoor
		stTmp = stTable + stCol;

		//Half for check, half for key back
		const uint32_t kIndexValSize = SHA256_DIGEST_LENGTH + SHA256_DIGEST_LENGTH;
		char szIndexTd[kIndexValSize];
		PRF::Sha256(m_szPk10, SHA256_DIGEST_LENGTH, (char*)stTmp.c_str(), stTmp.length(), szIndexTd, SHA256_DIGEST_LENGTH);
		strIndexTrapdoor.assign(szIndexTd, SHA256_DIGEST_LENGTH);

		//Get the Counter
		uint32_t uiCounter = m_mapOrderCounter[iID][stTmp]++;

		PRF::Sha256((char*)&uiCounter, sizeof(uiCounter), (char*)strIndexTrapdoor.c_str(), strIndexTrapdoor.length(), szIndexTd, SHA256_DIGEST_LENGTH);
		strIndexTrapdoor.assign(szIndexTd, SHA256_DIGEST_LENGTH);

		//Generate 

		memcpy(m_szOreRight, strTrapdoor.c_str(), strTrapdoor.length());
		char *pOreRight = m_szOreRight + strTrapdoor.length();

		uint32_t uiOreRightLen = m_oreHelper.CreateRight(pOreRight, DEF_ORERIGHT_MAXSIZE, stTable, stCol, uiCounter, *(uint32_t*)pVal);

		strIndexVal.assign(m_szOreRight, SHA256_DIGEST_LENGTH + uiOreRightLen);

		pClient->ProxyPut(strIndexTrapdoor, strIndexVal, strEmpty, strEmpty);

	}

	//Send the request to server

	//The first pair is fixed insert, however, the second is conditioned that only insert if not empty
	pClient->ProxyPut(strTrapdoor, strSendVal, strEmpty, strEmpty);

}

void ClientCpp::PutPlainText(string stTable, string stKey, string stCol, char *pVal, uint32_t uiLen, bool bColumnIndex, bool bEqualIndexV1, bool bEqualIndexV2, bool bOreIndex)
{
	//Choose a server by same path in Enc type.
	string stTmp = stTable + stKey + stCol;
	char szTd[SHA256_DIGEST_LENGTH];
	PRF::Sha256(m_szPk1, SHA256_DIGEST_LENGTH, (char*)stTmp.c_str(), stTmp.length(), szTd, SHA256_DIGEST_LENGTH);
	string strsendKey;

	//Just for constant empty string
	string strEmpty = "";

	//Check the Dispatcher to choose a server to send request.
	uint32_t uiKey = *(uint32_t*)szTd ^ *(uint32_t*)(szTd + 4) ^ *(uint32_t*)(szTd + 8) ^ *(uint32_t*)(szTd + 16) ^ *(uint32_t*)(szTd + 20) ^ *(uint32_t*)(szTd + 24) ^ *(uint32_t*)(szTd + 28);

	int iID;
	ThriftAdapt<TProxyServiceClient> *pThriftAdapt = *(m_SimConHash.Query(uiKey, &iID));
	TProxyServiceClient* pClient = pThriftAdapt->GetClient();

	string strSendVal;
	strSendVal.assign(to_string(*(int*)pVal));

	//Then, put send request to db

	//For equal index strategy I
	if (bEqualIndexV1)
	{
		strsendKey.assign(stKey);

		pClient->ProxyPut(strsendKey, strSendVal, strEmpty, strEmpty);
	}

	//For equal index strategy II
	if (bEqualIndexV2)
	{
		strsendKey.assign(stKey);

		vector<string> cmd;
		cmd.push_back("ZADD");
		cmd.push_back("index");
		cmd.push_back(strSendVal);
		cmd.push_back(strsendKey);
		vector<string> res;
		pClient->RunCommand(res, cmd);
		pClient->ProxyPut(strsendKey, strSendVal, strEmpty, strEmpty);
	}

	if (bOreIndex)
	{
		strsendKey.assign(stKey);
		pClient->ProxyPut(strsendKey, strSendVal, strEmpty, strEmpty);
	}
}

uint32_t ClientCpp::Search(string stTable, string stCol, char *pVal, uint32_t uiLen, bool bIndexV2)
{

	string strIndexTrapdoor;
	string strIndexVal;

	//Half for check, half for key back
	const uint32_t kIndexValSize = SHA256_DIGEST_LENGTH + SHA256_DIGEST_LENGTH;
	char szIndexTd[kIndexValSize];

	if (!bIndexV2)
	{

		//Generate the Index Trapdoor
		string stTmp = stTable + stCol;

		PRF::Sha256(m_szPk6, SHA256_DIGEST_LENGTH, (char*)stTmp.c_str(), stTmp.length(), szIndexTd, SHA256_DIGEST_LENGTH);
		strIndexTrapdoor.assign(szIndexTd, SHA256_DIGEST_LENGTH);

		//Generate Check Value

		uint32_t uiTDValueLen = stTmp.length();
		memcpy(m_szTDValue, stTmp.c_str(), stTmp.length());
		memcpy(m_szTDValue + uiTDValueLen, pVal, uiLen);
		uiTDValueLen += uiLen;

		PRF::Sha256(m_szPk7, SHA256_DIGEST_LENGTH, m_szTDValue, uiTDValueLen, szIndexTd, SHA256_DIGEST_LENGTH);
		strIndexVal.assign(szIndexTd, SHA256_DIGEST_LENGTH);

	}
	else
	{
		//Generate the Index Trapdoor
		string stTmp = stTable + stCol;

		uint32_t uiTDValueLen = stTmp.length();
		memcpy(m_szTDValue, stTmp.c_str(), stTmp.length());
		memcpy(m_szTDValue + uiTDValueLen, pVal, uiLen);
		uiTDValueLen += uiLen;

		PRF::Sha256(m_szPk8, SHA256_DIGEST_LENGTH, m_szTDValue, uiTDValueLen, szIndexTd, SHA256_DIGEST_LENGTH);
		strIndexTrapdoor.assign(szIndexTd, SHA256_DIGEST_LENGTH);

		//Generate 

		PRF::Sha256(m_szPk9, SHA256_DIGEST_LENGTH, m_szTDValue, uiTDValueLen, szIndexTd, SHA256_DIGEST_LENGTH);
		strIndexVal.assign(szIndexTd, SHA256_DIGEST_LENGTH);

	}

	//Get the clients
	ThriftAdapt<TProxyServiceClient> **parThriftAdapt = m_SimConHash.GetArray();
	uint32_t uiServerNum = m_SimConHash.GetNodeNum();

	//Send the request to all servers and collect the results
	vector<string> vecRet;
	vector<string> vecAll;
	for (uint32_t uiCur = 0; uiCur < uiServerNum; uiCur++)
	{

		TProxyServiceClient* pClient = parThriftAdapt[uiCur]->GetClient();
		if (!bIndexV2)
		{
			pClient->EqualSearch1(vecRet, strIndexTrapdoor, strIndexVal);
		}
		else
		{
			pClient->EqualSearch2(vecRet, strIndexTrapdoor, strIndexVal);
		}
		vecAll.insert(vecAll.end(), vecRet.begin(), vecRet.end());
	}

	return vecAll.size();

}

uint32_t ClientCpp::RangeQuery(string stTable, string stCol, int iCmp, uint32_t uiVal)
{

	string strIndexTrapdoor;
	string strIndexVal;

	//Generate the Index Trapdoor
	string stTmp = stTable + stCol;

	//Half for check, half for key back
	const uint32_t kIndexValSize = SHA256_DIGEST_LENGTH + SHA256_DIGEST_LENGTH;
	char szIndexTd[kIndexValSize];

	PRF::Sha256(m_szPk10, SHA256_DIGEST_LENGTH, (char*)stTmp.c_str(), stTmp.length(), szIndexTd, SHA256_DIGEST_LENGTH);
	strIndexTrapdoor.assign(szIndexTd, SHA256_DIGEST_LENGTH);

	uint32_t uiLeftLen = m_oreHelper.CreateLeft(m_szOreLeft, sizeof(m_szOreLeft), iCmp, stTable, stCol, uiVal);
	strIndexVal.assign(m_szOreLeft, uiLeftLen);

	//Get the clients
	ThriftAdapt<TProxyServiceClient> **parThriftAdapt = m_SimConHash.GetArray();
	uint32_t uiServerNum = m_SimConHash.GetNodeNum();

	//Send the request to all servers and collect the results
	vector<string> vecRet;
	vector<string> vecAll;
	for (uint32_t uiCur = 0; uiCur < uiServerNum; uiCur++)
	{
		TProxyServiceClient* pClient = parThriftAdapt[uiCur]->GetClient();
		pClient->OrderSearch(vecRet, strIndexTrapdoor, strIndexVal);
		vecAll.insert(vecAll.begin(), vecRet.begin(), vecRet.end());
	}

	return vecAll.size();
}

uint32_t ClientCpp::SearchPlainText(string stTable, string stCol, char *pVal, uint32_t uiLen, bool bIndexV2)
{
	string key;
	string empty = "";

	//Half for check, half for key back
	key.assign(to_string(*(int*)pVal));

	//Get the clients
	ThriftAdapt<TProxyServiceClient> **parThriftAdapt = m_SimConHash.GetArray();
	uint32_t uiServerNum = m_SimConHash.GetNodeNum();

	//Generate a random sequence to access all servers
	int serverID[16], sid;
	memset(serverID, 0xff, sizeof(serverID));
	for (int i = 0; i < uiServerNum; i++)
	{
		while (serverID[i] < 0)
		{
			sid = rand() % uiServerNum;
			for (int j = 0; j < i; j++)
			{
				if (serverID[j] == sid)
				{
					sid = -1;
					break;
				}
			}
			serverID[i] = sid;
		}
	}

	//Send the request to all servers and collect the results
	vector<string> vecRet;
	vector<string> vecAll;
	for (uint32_t uiCur = 0; uiCur < uiServerNum; uiCur++)
	{
		TProxyServiceClient* pClient = parThriftAdapt[serverID[uiCur]]->GetClient();
		if (bIndexV2)
		{
			pClient->EqualSearch2PlainText(vecRet, key, empty);
		}
		else
		{
			pClient->EqualSearch1PlainText(vecRet, key, empty);
		}
		vecAll.insert(vecAll.end(), vecRet.begin(), vecRet.end());
	}

	return vecAll.size();

}

uint32_t ClientCpp::RangeQueryPlainText(string stTable, string stCol, int iCmp, uint32_t uiVal)
{

	string key;
	string cmp;

	key.assign(to_string(uiVal));

	cmp.assign(to_string(iCmp));


	//Get the clients
	ThriftAdapt<TProxyServiceClient> **parThriftAdapt = m_SimConHash.GetArray();
	uint32_t uiServerNum = m_SimConHash.GetNodeNum();

	//Generate a random sequence to access all servers
	int serverID[16], sid;
	memset(serverID, 0xff, sizeof(serverID));
	for (int i = 0; i < uiServerNum; i++)
	{
		while (serverID[i] < 0)
		{
			sid = rand() % uiServerNum;
			for (int j = 0; j < i; j++)
			{
				if (serverID[j] == sid)
				{
					sid = -1;
					break;
				}
			}
			serverID[i] = sid;
		}
	}

	//Send the request to all servers and collect the results
	vector<string> vecRet;
	vector<string> vecAll;
	for (uint32_t uiCur = 0; uiCur < uiServerNum; uiCur++)
	{
		TProxyServiceClient* pClient = parThriftAdapt[serverID[uiCur]]->GetClient();
		pClient->OrderSearchPlainText(vecRet, key, cmp);
		vecAll.insert(vecAll.begin(), vecRet.begin(), vecRet.end());
	}

	return vecAll.size();

}

void ClientCpp::GetCol(vector<string> &_retVal, string stTable, string stCol, uint32_t uiNum)
{

	string stTmp = stTable + stCol;

	char szIndexTd[SHA256_DIGEST_LENGTH];
	PRF::Sha256(m_szPk3, SHA256_DIGEST_LENGTH, (char*)stTmp.c_str(), stTmp.length(), szIndexTd, SHA256_DIGEST_LENGTH);

	//Generate Index Trapdoor
	string strIndexTrapdoor;
	strIndexTrapdoor.assign(szIndexTd, SHA256_DIGEST_LENGTH);

	//Generate 
	PRF::Sha256(m_szPk4, SHA256_DIGEST_LENGTH, (char*)stTmp.c_str(), stTmp.length(), szIndexTd, SHA256_DIGEST_LENGTH);
	string strIndexMask;
	strIndexMask.assign(szIndexTd, SHA256_DIGEST_LENGTH);

	//Get the clients
	ThriftAdapt<TProxyServiceClient> **parThriftAdapt = m_SimConHash.GetArray();
	uint32_t uiServerNum = m_SimConHash.GetNodeNum();

	//Send the request to all servers and collect the results
	vector<string> vecRet;
	vector<string> vecCiphertext;
	for (uint32_t uiCur = 0; uiCur < uiServerNum; uiCur++)
	{
		TProxyServiceClient* pClient = parThriftAdapt[uiCur]->GetClient();
		pClient->ProxyGetColumn(vecRet, strIndexTrapdoor, strIndexMask, uiNum / uiServerNum);

		vecCiphertext.insert(vecCiphertext.begin(), vecRet.begin(), vecRet.end());
	}

	//AES Decryption
	string strPlaintext;
	for (vector<string>::iterator it = vecCiphertext.begin(); it != vecCiphertext.end(); it++)
	{
		m_Decrypt(*it, strPlaintext);
		_retVal.push_back(strPlaintext);
	}

}

void ClientCpp::Open()
{
	ThriftAdapt<TProxyServiceClient> **parThriftAdapt = m_SimConHash.GetArray();
	uint32_t uiNodeNum = m_SimConHash.GetNodeNum();
	for (uint32_t uiCur = 0; uiCur < uiNodeNum; uiCur++)
	{
		parThriftAdapt[uiCur]->Open();
	}
}

void ClientCpp::Close()
{
	ThriftAdapt<TProxyServiceClient> **parThriftAdapt = m_SimConHash.GetArray();
	uint32_t uiNodeNum = m_SimConHash.GetNodeNum();
	for (uint32_t uiCur = 0; uiCur < uiNodeNum; uiCur++)
	{
		parThriftAdapt[uiCur]->Close();
	}
}

void ClientCpp::Init(string arIP[], uint16_t arPort[], uint32_t uiServerNum)
{
	//Set the ServerConfig
	const uint32_t uiMax = (uint32_t)-1;
	uint32_t uiInterval = uiMax / uiServerNum - 5;

	for (uint32_t uiCur = 0; uiCur < uiServerNum; uiCur++)
	{
		ThriftAdapt<TProxyServiceClient> *pThriftAdapt = new ThriftAdapt<TProxyServiceClient>();
		pThriftAdapt->Init(arIP[uiCur], arPort[uiCur]);
		//Add a node to SimConHash
		m_SimConHash.InsertNode(uiInterval * (uiCur + 1), pThriftAdapt);

	}
}

void ClientCpp::InitExample(uint32_t uiServerNum)
{
	//Set the ServerName and Init the Examples
	const uint32_t uiMax = (uint32_t)-1;
	uint32_t uiInterval = uiMax / uiServerNum - 5;

	for (uint32_t uiCur = 0; uiCur < uiServerNum; uiCur++)
	{
		ThriftAdapt<TProxyServiceClient> *pThriftAdapt = new ThriftAdapt<TProxyServiceClient>();
		pThriftAdapt->Init(kDemoServerIP[uiCur], kDemoServerPort[uiCur]);
		//Add a node to SimConHash
		m_SimConHash.InsertNode(uiInterval * (uiCur + 1), pThriftAdapt);

	}

}

void ClientCpp::m_Decrypt(string &strCiphertext, string &strPlaintext)
{
	//AES Decryption
	char *pDec = new char[AES::CbcMaxsize(strCiphertext.length())];

	memset(pDec, 0, strCiphertext.length());

	uint32_t uiSize = AES::CbcDecrypt256(strCiphertext.c_str(), strCiphertext.length(), pDec, m_szPk5);

	strPlaintext.assign(pDec, uiSize);

	delete[] pDec;

}

void ClientCpp::m_Encrypt(string &strCiphertext, char *pPlaintext, uint32_t uiPlaintextLen)
{
	//AES Encryption

	char *pEnc = new char[AES::CbcMaxsize(uiPlaintextLen)];

	memset(pEnc, 0, AES::CbcMaxsize(uiPlaintextLen));

	uint32_t uiEncLen = AES::CbcEncrypt256(pPlaintext, uiPlaintextLen, pEnc, m_szPk5);

	strCiphertext.assign(pEnc, uiEncLen);

	delete[] pEnc;
}
