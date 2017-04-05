#include <cstdio>
#include "../fastore/OREHelper.h"
using namespace std;

int main() {
	printf("This is test file 1 \n");
	OREHelper* ph;
	char s[128];
	//sprintf()
	for (int i = 0; i < 100000; ++i)
	{
		ph = new OREHelper();
		sprintf(s, "%s:%d.", "Hi:", i);
		ph->Init(s, 8);
		delete ph;
	}
	printf("Done!\n");
	return 0;
}