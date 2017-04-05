#include <redis3m/redis3m.hpp>
#include <iostream>
#include <cstring>

using namespace redis3m;
connection::ptr_t c;
void Open()
{
	c = connection::create();
}

void Close()
{

}

uint32_t Get(char *pKey, uint32_t uiKeyLen, char *pOut, uint32_t uiOutLen)
{
	std::string sKey, sVal;
	sKey.append(pKey);
	reply r = c->run(command("GET") << sKey);
	sVal = r.str();
	uint32_t len = (unsigned)sVal.size();
	if (len > uiOutLen)
	{
		return 0;
	}
	else
	{
		std::copy(sVal.begin(), sVal.end(), pOut);
		pOut[sVal.size()] = '\0';
		std::cout << pOut << std::endl;
		return len;
	}
}

void Put(char *pKey, uint32_t uiKeyLen, char *pVal, uint32_t uiValLen)
{
	std::string sKey, sVal;
	sKey.append(pKey);
	sVal.append(pVal);
	c->run(command("SET") << sKey << sVal);
	std::cout << uiKeyLen << " " << uiValLen << std::endl;
	return;
}


int main(int argc, char **argv)
{
	Open();
	char* tkey = "Hi";
	char* tvalue = "Redis";
	Put(tkey, (unsigned)2, tvalue, (unsigned)5);
	char* rvalue = new char[10];
	uint32_t uiOutLen = 10;
	std::cout << Get(tkey, 2, rvalue, uiOutLen) << std::endl;

}
