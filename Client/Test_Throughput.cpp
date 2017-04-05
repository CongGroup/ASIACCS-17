#include <iostream>
#include <string.h>
#include <string>
#include <stdint.h>
#include <vector>

#include "DemoConfig.h"
#include "ClientCpp.h"
#include "../Caravel/TimeDiff.h"
#include "../fastore/OREHelper.h"

using namespace std;
using namespace caravel;

int main(int argc, char **argv)
{

	if (argc != 8)
	{
		cout << "usage : ./" << argv[0] << " [DataNodeNum] [A] [B] [QueryIndexType] [C] [D] [BlockSizeInBit]" << endl;
		cout << "The DataNodeNum showed that the number of cluster." << endl;
		cout << "The test will begin at [A] ." << endl;
		cout << "The test will consist [B] seconds" << endl;
		cout << "The QueryIndexType is for indicate the operation type : 0 means EqualV1, 1 means EqualV2, 2 means OrderQuery, 3 means EqualV2PlainText, 4 means OREPlainText" << endl;
		cout << "The Seed is [C] ." << endl;
		cout << "The Max Boundary to gen query is [D]." << endl;
		cout << "The Block_size in bit is [BlockSizeInBit]." << endl;
		return 0;
	}


	//Get the params from command line
	int iMaxBoundary;
	uint32_t uiBeg, uiTime, uiSeed, uiServerNum, uiQueryIndexType, uiBlockSizeInBits;
	sscanf(argv[1], "%u", &uiServerNum);
	sscanf(argv[2], "%u", &uiBeg);
	sscanf(argv[3], "%u", &uiTime);
	sscanf(argv[4], "%u", &uiQueryIndexType);
	sscanf(argv[5], "%u", &uiSeed);
	sscanf(argv[6], "%u", &iMaxBoundary);
	sscanf(argv[7], "%u", &uiBlockSizeInBits);



	//Compute the params
	uint32_t uiCurTime;
	uint32_t uiBegTime = uiBeg;
	uint32_t uiEndTime = uiBegTime + uiTime;

	//Set the Seed of generate random number
	srand(uiSeed);

	//Init the cache for key
	string strKey;
	string strVal;

	//Init the client to server
	ClientCpp client;
	client.InitKey(DEMO_SECURITY_KEY, uiBlockSizeInBits);
	client.InitExample(uiServerNum);
	client.Open();

	uint32_t uiCnt = 0;
	while (true)
	{
		uiCurTime = time(NULL);

		if (uiCurTime < uiBegTime)
		{
			continue;
		}

		if (uiCurTime >= uiEndTime)
		{
			break;
		}

		vector<string> vecRet;
		vector<string> vecAll;

		int uiRandNum;



		if (iMaxBoundary == 0)
		{
			uiRandNum = 0;
		}
		else
		{
			if (iMaxBoundary > 0)
			{
				uiRandNum = rand() % iMaxBoundary;
			}
			else
			{
				uiRandNum = -iMaxBoundary;
			}
		}


		//Equal Version 1
		if (0 == uiQueryIndexType)
		{
			uint32_t uiSearchCount = client.Search("StudentScoreTable", "Score", (char*)&uiRandNum, sizeof(uint32_t), false);
		}
		//Equal Version 2
		else if (1 == uiQueryIndexType)
		{
			uint32_t uiSearchCount = client.Search("StudentScoreTable", "Score", (char*)&uiRandNum, sizeof(uint32_t), true);
		}
		//Order Version Index
		else if (2 == uiQueryIndexType)
		{
			//Execute Order version search
			int iCmp = uiRandNum % 2 == 0 ? 1 : -1;
			if (iMaxBoundary < 0)
			{
				iCmp = 1;
			}
			uint32_t uiSearchCount = client.RangeQuery("StudentScoreTable", "Score", iCmp, uiRandNum + 1);
		}
		else if (3 == uiQueryIndexType)
		{
			uint32_t uiSearchCount = client.SearchPlainText("StudentScoreTable", "Score", (char*)&uiRandNum, sizeof(uint32_t), true);
		}
		else if (4 == uiQueryIndexType)
		{
			int iCmp = uiRandNum % 2 == 0 ? 1 : -1;
			if (iMaxBoundary < 0)
			{
				iCmp = 1;
			}
			uint32_t uiSearchCount = client.RangeQueryPlainText("StudentScoreTable", "Score", iCmp, uiRandNum + 1);
		}
		else if (5 == uiQueryIndexType)
		{
			uint32_t uiSearchCount = client.SearchPlainText("StudentScoreTable", "Score", (char*)&uiRandNum, sizeof(uint32_t), false);
		}
		else
		{
			cout << "Error Operation !" << endl;
			return 0;
		}

		uiCnt++;
	}

	cout << uiCnt << endl;

	client.Close();

	return 0;
}




