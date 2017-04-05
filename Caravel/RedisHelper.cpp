#include "RedisHelper.h"

#include <redis3m/redis3m.hpp>
#include <string>
#include <string.h>
#include <iostream>
#include <stdint.h>
#include <map>

using namespace redis3m;
using namespace std;

#define DEF_REP_TIMES 10

namespace caravel {

    RedisHelper::RedisHelper()
    {
    }


    RedisHelper::~RedisHelper()
    {
    }


    void RedisHelper::OpenClusterPool(const std::string& host, const unsigned int port)
    {
        m_ptrClusterPool = simple_pool::create(host, port);
    }


    void RedisHelper::CloseClusterPool()
    {

    }


    uint32_t RedisHelper::ClusterPoolGet(const string &strKey, string &strVal)
    {
        //The first step

        m_ptrClusterPool->run_with_connection<void>([&](connection::ptr_t conn)
        {
            strVal = conn->run(command("GET")(strKey)).str();
        }, DEF_REP_TIMES);

        if (strVal.compare(0, 6, "MOVED ") == 0)
        {
            //Second step
            uint32_t uiIPBeg = strVal.find_last_of(' ') + 1;
            uint32_t uiIPEnd = strVal.find_last_of(':');
            string strIP = strVal.substr(uiIPBeg, uiIPEnd - uiIPBeg);
            string strPort = strVal.substr(uiIPEnd + 1, strVal.length() - uiIPEnd);
            uint32_t uiPort;
            sscanf(strPort.c_str(), "%u", &uiPort);

            simple_pool::ptr_t ptrPool;

            //find the simple_pool
            if (m_mapPtrPool.find(strIP) != m_mapPtrPool.end())
            {
                //find connect
                ptrPool = m_mapPtrPool[strIP];
            }
            else
            {
                ptrPool = simple_pool::create(strIP.c_str(), uiPort);
                m_mapPtrPool[strIP] = ptrPool;
            }

            ptrPool->run_with_connection<void>([&](connection::ptr_t conn)
            {
                strVal = conn->run(command("GET")(strKey)).str();
            }, DEF_REP_TIMES);

            return strVal.length();

        }
        else
        {
            return strVal.length();
        }

    }


    void RedisHelper::ClusterPoolPut(const string &strKey, const string &strVal)
    {
        m_ptrClusterPool->run_with_connection<void>([&](connection::ptr_t conn)
        {
            conn->run(command("SET")(strKey)(strVal));
        }, DEF_REP_TIMES);
    }


    void RedisHelper::OpenPool(const std::string& host, const unsigned int port)
    {
        m_ptrPool = simple_pool::create(host, port);
    }

    void RedisHelper::ClosePool()
    {

    }


    uint32_t RedisHelper::PoolGet(const string &strKey, string &strVal)
    {
        m_ptrPool->run_with_connection<void>([&](connection::ptr_t conn)
        {
            strVal = conn->run(command("GET")(strKey)).str();
        }, DEF_REP_TIMES);

        return strVal.length();

    }

    void RedisHelper::PoolPut(const string &strKey, const string &strVal)
    {
        m_ptrPool->run_with_connection<void>([&](connection::ptr_t conn)
        {
            conn->run(command("SET")(strKey)(strVal));
        }, DEF_REP_TIMES);
    }

	void ins(vector<string>& res, reply rep)
	{
		switch (rep.type())
		{
		case reply::type_t::INTEGER:
		{
			res.push_back(std::to_string(rep.integer()));
			break;
		}
		case reply::type_t::STRING:
		{
			res.push_back(rep.str());
			break;
		}
		case reply::type_t::ARRAY:
		{
			vector<reply> arrReply = rep.elements();

			for (vector<reply>::iterator i = arrReply.begin(); i != arrReply.end(); i++)
			{
				ins(res, *i);
			}
			break;
		}
		default:
		{
			break;
		}
		}
	}

	void RedisHelper::PoolRun(const vector<string>& cmd, vector<string>& res)
	{
		m_ptrPool->run_with_connection<void>([&](connection::ptr_t conn)
		{
			reply rep = conn->run(cmd);
			res.clear();
			ins(res, rep);
		}, DEF_REP_TIMES);
	}


    void RedisHelper::Open(const std::string& host, const unsigned int port)
    {
        cout << "Begin Open" << endl;
        m_ptrConnection = connection::create(host, port);
        cout << "End Open" << endl;
    }

    void RedisHelper::Close()
    {

    }

    uint32_t RedisHelper::Get(const string &strKey, string &strVal)
    {

        reply oReply = m_ptrConnection->run(command("GET") << strKey);
        strVal = oReply.str();

        if (strVal.compare(0, 6, "MOVED ") == 0)
        {
            //Second step
            uint32_t uiIPBeg = strVal.find_last_of(' ') + 1;
            uint32_t uiIPEnd = strVal.find_last_of(':');
            string strIP = strVal.substr(uiIPBeg, uiIPEnd - uiIPBeg);
            string strPort = strVal.substr(uiIPEnd + 1, strVal.length() - uiIPEnd);
            uint32_t uiPort;
            sscanf(strPort.c_str(), "%u", &uiPort);

            connection::ptr_t ptrConnection;

            //find the simple_pool
            if (m_mapPtrConnection.find(strIP) != m_mapPtrConnection.end())
            {
                //find connect
                ptrConnection = m_mapPtrConnection[strIP];
            }
            else
            {
                ptrConnection = connection::create(strIP.c_str(), uiPort);
                m_mapPtrConnection[strIP] = ptrConnection;
            }

            strVal = ptrConnection->run(command("GET") << strKey).str();

            return strVal.length();

        }
        else
        {
            return strVal.length();
        }
    }

    void RedisHelper::Put(const string &strKey, const string &strVal)
    {

        m_ptrConnection->run(command("SET") << strKey << strVal);

#ifdef DEBUG_REDIS_HELPER

        std::cout << strKey << " " << strVal << std::endl;

#endif

        return;
    }


}


