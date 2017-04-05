#ifndef __THRIFTADAPT_H__
#define __THRIFTADAPT_H__

#include <string>
#include <thrift/transport/TSocket.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TBufferTransports.h>

using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace std;

namespace caravel {

	template<class T>
	class ThriftAdapt
	{
	public:
		ThriftAdapt()
		{
			m_pClient = NULL;
		}

		ThriftAdapt(string sIP, uint16_t sPort)
		{
			m_pClient = NULL;
			Init(sIP, sPort);
		}

		~ThriftAdapt(void)
		{
			delete m_pClient;
		}

		void Init(string sIP, uint16_t sPort)
		{
			m_ptrSocket = boost::shared_ptr<TTransport>(new TSocket(sIP, sPort));
			m_ptrTransport = boost::shared_ptr<TTransport>(new TBufferedTransport(m_ptrSocket));
			m_ptrProtocol = boost::shared_ptr<TProtocol>(new TBinaryProtocol(m_ptrTransport));
			m_pClient = new T(m_ptrProtocol);
		}

		void Open()
		{
			m_ptrTransport->open();
		}

		void Close()
		{
			m_ptrTransport->close();
		}

		T *GetClient()
		{
			return m_pClient;
		}

	private:

		boost::shared_ptr<TTransport> m_ptrSocket;
		boost::shared_ptr<TTransport> m_ptrTransport;
		boost::shared_ptr<TProtocol> m_ptrProtocol;

		T *m_pClient;

	};

}

#endif
