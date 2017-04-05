#ifndef __REDIS_HELPER_H__
#define __REDIS_HELPER_H__


#include <string>
#include <string.h>
#include <iostream>
#include <stdint.h>
#include <map>
#include <vector>

#include <redis3m/redis3m.hpp>

using namespace std;
using namespace redis3m;

namespace caravel {

    class RedisHelper
    {
    public:
        RedisHelper();
        ~RedisHelper();

        /*Multi Connection Pool*/
        void OpenClusterPool(const std::string& host = "localhost", const unsigned int port = 6379);
        void CloseClusterPool();

        uint32_t ClusterPoolGet(const string &strKey, string &strVal);
        void ClusterPoolPut(const string &strKey, const string &strVal);

        /*Connection Pool*/

        void OpenPool(const std::string& host = "localhost", const unsigned int port = 6379);
        void ClosePool();

        uint32_t PoolGet(const string &strKey, string &strVal);
        void PoolPut(const string &strKey, const string &strVal);

		void PoolRun(const vector<string>& cmd, vector<string>& res);

        /*Single Use*/

        //For open a connection to Redis server.
        void Open(const std::string& host = "localhost", const unsigned int port = 6379);

        //For close the connection to Redis server.
        void Close();

        //Get function

        uint32_t Get(const string &strKey, string &strVal);

        //Put function
        void Put(const string &strKey, const string &strVal);

    private:

        //For Single connect
        map<string, connection::ptr_t> m_mapPtrConnection;
        connection::ptr_t m_ptrConnection;

        //For simple connect pool
        simple_pool::ptr_t m_ptrPool;

        //For cluster connect pool
        map<string, simple_pool::ptr_t> m_mapPtrPool;
        simple_pool::ptr_t m_ptrClusterPool;



    };


}

#endif




