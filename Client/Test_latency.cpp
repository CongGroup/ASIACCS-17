#include <iostream>
#include <string.h>
#include <string>
#include <stdint.h>
#include <vector>

#include "DemoConfig.h"
#include "ClientCpp.h"
#include "../Caravel/TimeDiff.h"

using namespace std;
using namespace caravel;



int main(int argc, char **argv)
{
	if (argc != 7)
	{
		cout << "usage : ./" << argv[0] << " [DataNodeNum] [BegNum] [EndNum] [Times] [QueryIndexType] [BlockSizeInBit]" << endl;
		cout << "The DataNodeNum showed that the number of cluster." << endl;
		cout << "The test number will begin at [BegNum] and end at [EndNum] ." << endl;
		cout << "The total test number will be [Times]." << endl;
		cout << "The QueryIndexType is for indicate the operation type : 0 means EqualV1, 1 means EqualV2, 2 means OrderQuery, 3 means EqualV2PlainText, 4 means OREPlainText" << endl;
		cout << "The Block_size in bit is [BlockSizeInBit]." << endl;

		return 0;
	}

	//Get the params from command line
	uint32_t uiBeg, uiEnd, uiServerNum, uiTimes, uiQueryIndexType, uiBlockSizeInBits;
	sscanf(argv[1], "%u", &uiServerNum);
	sscanf(argv[2], "%u", &uiBeg);
	sscanf(argv[3], "%u", &uiEnd);
	sscanf(argv[4], "%u", &uiTimes);
	sscanf(argv[5], "%u", &uiQueryIndexType);
	sscanf(argv[6], "%u", &uiBlockSizeInBits);

	//Set the Seed of generate random number
	srand(1);

	//Init the cache for key
	string strKey;
	string strVal;

	//Init the client to server
	ClientCpp client;
	client.InitKey(DEMO_SECURITY_KEY, uiBlockSizeInBits);
	client.InitExample(uiServerNum);
	client.Open();

	//Prepare for Query data
	uint32_t *arQueryKey = new uint32_t[uiTimes];

	if (uiBeg != uiEnd)
	{
		for (uint32_t uiCur = 0; uiCur < uiTimes; uiCur++)
		{
			arQueryKey[uiCur] = uiBeg + (rand() % (uiEnd - uiBeg));
		}

	}
	else
	{
		for (uint32_t uiCur = 0; uiCur < uiTimes; uiCur++)
		{
			arQueryKey[uiCur] = uiBeg;
		}
	}


	//Timer
	uint32_t uiTimeDiff;
	uint32_t uiSearchCount;
	TimeDiff::DiffTimeInMicroSecond();

	cout << "In Each Times get:" << endl;
	for (uint32_t uiCur = 0; uiCur < uiTimes; uiCur++)
	{
		//Equal Version 1
		if (0 == uiQueryIndexType)
		{
			uiSearchCount = client.Search("StudentScoreTable", "Score", (char*)(arQueryKey + uiCur), sizeof(uint32_t), false);
		}
		//Equal Version 2
		else if (1 == uiQueryIndexType)
		{
			uiSearchCount = client.Search("StudentScoreTable", "Score", (char*)(arQueryKey + uiCur), sizeof(uint32_t), true);
		}
		//Order Version Index
		else if (2 == uiQueryIndexType)
		{
			//Execute Order version search
			int iCmp = 1;
			uiSearchCount = client.RangeQuery("StudentScoreTable", "Score", iCmp, arQueryKey[uiCur]);
		}
		else if (3 == uiQueryIndexType)
		{
			uiSearchCount = client.SearchPlainText("StudentScoreTable", "Score", (char*)(arQueryKey + uiCur), sizeof(uint32_t), true);
		}
		else if (4 == uiQueryIndexType)
		{
			uiSearchCount = client.RangeQueryPlainText("StudentScoreTable", "Score", -1, *(arQueryKey + uiCur));
		}
		else if (5 == uiQueryIndexType)
		{
			uiSearchCount = client.SearchPlainText("StudentScoreTable", "Score", (char*)(arQueryKey + uiCur), sizeof(uint32_t), false);
		}
		cout << uiSearchCount << "\t";
	}

	uiTimeDiff = TimeDiff::DiffTimeInMicroSecond();

	cout << endl << "Use Time :" << uiTimeDiff << endl;
	cout << "-----------------------------------------------------------------------------" << endl;
	cout << "Finish Test" << endl;

	delete[] arQueryKey;

}



