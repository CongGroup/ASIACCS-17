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
	if (argc != 8)
	{
		cout << "usage : ./" << argv[0] << " [DataNodeNum] [BegNum] [EndNum] [EqualIndexVersion] [OrderIndex] [BlockSizeInBit] [ModNum]" << endl;
		cout << "The BegNum is the min number to insert ." << endl;
		cout << "The EndNum is the max number to insert ." << endl;
		cout << "The EqualIndexVersion: 0 means NO index, 1 means version 1 equal index, 2 means version 2 equal index, 3 means version 2 PlainText equal index." << endl;
		cout << "The OrderIndex: 0 means NO index, 1 means Order Index, 2 means PlainText Order Index." << endl;
		cout << "The Block_size in bit is [BlockSizeInBit]." << endl;
		cout << "The real number to insert will mod the [ModNum]." << endl;
		return 0;
	}

	//Get the params from command line
	uint32_t uiServerNum, uiEndNum, uiEqualIndex, uiOrderIndex, uiBlockSizeInBits, uiModNum;
	int iBegNum;

	sscanf(argv[1], "%u", &uiServerNum);
	sscanf(argv[2], "%d", &iBegNum);
	sscanf(argv[3], "%u", &uiEndNum);
	sscanf(argv[4], "%u", &uiEqualIndex);
	sscanf(argv[5], "%u", &uiOrderIndex);
	sscanf(argv[6], "%u", &uiBlockSizeInBits);
	sscanf(argv[7], "%u", &uiModNum);

	if (uiModNum == 0)
	{
		uiModNum = iBegNum > 0 ? uiEndNum : -iBegNum > uiEndNum ? -iBegNum : uiEndNum;
	}


	//Init the client to server
	ClientCpp client;
	client.InitKey(DEMO_SECURITY_KEY, uiBlockSizeInBits);
	client.InitExample(uiServerNum);
	client.Open();

	//Index Option
	bool EqualIndex1 = uiEqualIndex == 1;
	bool EqualIndex2 = uiEqualIndex == 2;
	bool OrderIndex = uiOrderIndex == 1;
	bool EqualIndex1Plaintext = uiEqualIndex == 4;
	bool EqualIndex2Plaintext = uiEqualIndex == 3;
	bool OrderIndexPlaintext = uiOrderIndex == 2;

	char szBuf[DEF_PRF_OUTPUT_SIZE];

	string strKey;

	TimeDiff::DiffTimeInMicroSecond();


	if (iBegNum >= 0)
	{
		uint32_t uiBegNum = iBegNum;

		for (uint32_t uiCur = uiBegNum; uiCur < uiEndNum; uiCur++)
		{

			snprintf(szBuf, sizeof(szBuf), "%u", uiCur);
			strKey.assign(szBuf);

			uint32_t realNum = uiCur%uiModNum;

			//If it is plainText insert, do not put Enc Value to database
			if (EqualIndex1 || EqualIndex2 || OrderIndex || !(EqualIndex1Plaintext || EqualIndex2Plaintext || OrderIndexPlaintext))
			{
				client.Put("StudentScoreTable", strKey, "Score", (char*)&realNum, sizeof(uint32_t), false, EqualIndex1, EqualIndex2, OrderIndex);
			}
			if (EqualIndex1Plaintext || EqualIndex2Plaintext || OrderIndexPlaintext)
			{
				client.PutPlainText("StudentScoreTable", strKey, "Score", (char*)&realNum, sizeof(uint32_t), false, EqualIndex1Plaintext, EqualIndex2Plaintext, OrderIndexPlaintext);
			}
		}

	}
	else
	{

		uint32_t uiBegNum = 0 - iBegNum;


		for (uint32_t uiCur = 0; uiCur < uiBegNum; uiCur++)
		{
			snprintf(szBuf, sizeof(szBuf), "D%uD%u", uiBegNum, uiCur);
			strKey.assign(szBuf);
			if (EqualIndex1 || EqualIndex2 || OrderIndex)
			{
				client.Put("StudentScoreTable", strKey, "Score", (char*)&uiBegNum, sizeof(uint32_t), false, EqualIndex1, EqualIndex2, OrderIndex);
			}
			client.PutPlainText("StudentScoreTable", strKey, "Score", (char*)&uiBegNum, sizeof(uint32_t), false, EqualIndex1Plaintext, EqualIndex2Plaintext, OrderIndexPlaintext);
		}

		for (uint32_t uiCur = 0; uiCur < uiEndNum; uiCur++)
		{
			snprintf(szBuf, sizeof(szBuf), "%u", uiCur);
			strKey.assign(szBuf);

			uint32_t realNum = uiCur%uiModNum;

			if (uiCur != uiBegNum)
			{
				if (EqualIndex1 || EqualIndex2 || OrderIndex)
				{
					client.Put("StudentScoreTable", strKey, "Score", (char*)&realNum, sizeof(uint32_t), false, EqualIndex1, EqualIndex2, OrderIndex);
				}
				client.PutPlainText("StudentScoreTable", strKey, "Score", (char*)&realNum, sizeof(uint32_t), false, EqualIndex1Plaintext, EqualIndex2Plaintext, OrderIndexPlaintext);
			}
		}

	}



	uint32_t uiTimeDiff = TimeDiff::DiffTimeInMicroSecond();

	cout << "Cost Microsecond = " << uiTimeDiff << endl;
	cout << "Cost Millisecond = " << uiTimeDiff / 1000 << endl;

	return 0;

}
