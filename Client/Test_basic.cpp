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
	if (argc != 3)
	{
		cout << "usage : ./" << argv[0] << " [DataNodeNum] [uiBlockSizeInBits]" << endl;
		cout << "The [DataNodeNum] showed that the number of cluster." << endl;
		cout << "The Block_size in bit is [uiBlockSizeInBits]." << endl;
		return 0;
	}

	//Get the params from command line
	uint32_t uiServerNum, uiBlockSizeInBits;
	sscanf(argv[1], "%u", &uiServerNum);
	sscanf(argv[2], "%u", &uiBlockSizeInBits);


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

	//Order Helper
	OREHelper oreHelper;

	strKey = DEMO_SECURITY_KEY;
	oreHelper.Init(strKey + "ORE", DEF_BLOCK_SIZE_INBIT);

	uint32_t uiCnt = 0;
	//Timer
	uint32_t uiTimeDiff;

	vector<string> vecResult;

	TimeDiff::DiffTimeInMicroSecond();

	int iCmp = -1;
	uint32_t uiSearchCount = client.RangeQuery("StudentScoreTable", "Score", iCmp, 50);

	uiTimeDiff = TimeDiff::DiffTimeInMicroSecond();

	cout << "Time Cost " << uiTimeDiff << "And find result " << uiSearchCount << endl;

	client.Close();

	cout << "-----------------------------------------------------------------------------" << endl;
	cout << "Finish Test" << endl;


}
