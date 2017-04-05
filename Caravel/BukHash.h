#ifndef __BUKHASH_H__
#define __BUKHASH_H__

#include <string.h>
#include <stdint.h>
#include <iostream>
#include <string>

#include "ShmCtl.h"

using namespace std;

namespace caravel {

	template <class K, class T>
	class BukHash
	{

	public:
		BukHash(void) {};
		~BukHash(void) {};

		string GetErr()
		{
			return sErr;
		}

		bool Create(key_t kKey, uint32_t uiW, uint32_t uiL, bool bZero = true)
		{
			if (!Create((char*)NULL, uiW, uiL, bZero))
			{
				return false;
			}
			m_ShmKey = kKey;
			//Get Shm Memory
			if (!ShmCtl::GetShm(&m_pHead, kKey, m_sizMem))
			{
				sErr = "ERROR : Get Shm False!";
				return false;
			}
			//Init Data
			if (bZero)
			{
				memset(m_pHead, 0, m_sizMem);
			}
			m_pCur = m_pHead;
			return true;
		}

		bool Create(char *pHead, uint32_t uiW, uint32_t uiL, bool bZero = true)
		{
			//Set Vars
			m_uiW = uiW;
			m_uiL = uiL;
			m_ShmKey = 0;
			//Init Prime
			uint32_t uiSum = InitPrime();
			if (0 == uiSum)
			{
				//�޷�������������
				sErr = "ERROR : Can 't Create Prime Array.";
				return false;
			}
			m_uiAllNum = uiSum;
			m_sizMem = uiSum * sizeof(_Item);
			//Init Head Pointer
			m_pHead = (_Item*)pHead;
			//Init Data
			if (pHead != NULL)
			{
				if (bZero)
				{
					memset(pHead, 0, m_sizMem);
				}
				m_pCur = m_pHead;
			}
			m_uiNum = 0;
			return true;
		}

		//���bNew == true ������Key�����ڣ������¿ռ䲢���ص�ַ��
		//���bNew == false ����Key���ڣ��򷵻�ָ��T��
		//������ڴ���䣬�򷵻�NULL������bNewΪ��ֵ��
		T *New(K key, bool &bOut)
		{
			_Item *pItem = NULL;
			_Item *pTemp = m_pHead;
			for (uint32_t uiCur = 0; uiCur < m_uiL; uiCur++)
			{
				pItem = pTemp + key % m_arPrime[uiCur];
				if (pItem->key == key)
				{
					bOut = false;
					return &pItem->value;
				}
				if (pItem->key == 0)
				{
					bOut = true;
					pItem->key = key;
					m_uiNum++;
					return &pItem->value;
				}
				pTemp += m_arPrime[uiCur];
			}
			return NULL;
		}

		//���Key���ڣ��򷵻�ָ�롣
		//���Key�����ڣ���bNewΪtrue���򴴽�����Ϊfalse���򷵻�NULL��
		//���δ�пռ䣬�򷵻�NULL��
		T *Get(K key, bool bNew)
		{
			_Item *pItem = NULL;
			_Item *pTemp = m_pHead;
			for (uint32_t uiCur = 0; uiCur < m_uiL; uiCur++)
			{
				pItem = pTemp + key % m_arPrime[uiCur];
				//If Key Equal
				if (pItem->key == key)
				{
					return &pItem->value;
				}
				//If Empty
				if (pItem->key == 0)
				{
					if (bNew)
					{
						pItem->key = key;
						m_uiNum++;
						return &pItem->value;
					}
					else
					{
						return NULL;
					}
				}
				pTemp += m_arPrime[uiCur];
			}
			return NULL;
		}

		//���key���ڣ�����val�滻����������ڣ������һ���val��
		//����true��ʾ�����ɹ�������false��ʾ���ڴ�����ʧ�ܡ�
		bool Set(K key, T *pVal)
		{
			_Item *pItem = NULL;
			_Item *pTemp = m_pHead;
			for (uint32_t uiCur = 0; uiCur < m_uiL; uiCur++)
			{
				pItem = pTemp + key % m_arPrime[uiCur];
				if (pItem->key == key)
				{
					memcpy(&pItem->value, pVal, sizeof(T));
					return true;
				}
				if (pItem->key == 0)
				{
					pItem->key = key;
					m_uiNum++;
					memcpy(&pItem->value, pVal, sizeof(T));
					return true;
				}
				pTemp += m_arPrime[uiCur];
			}
			return false;
		}

		//ɾ��ָ��key
		//��ɾ���򷵻�ɾ�������ݣ�ͬGet�����´η������޷��ҵ���
		//���Ҳ���Ԫ�أ��򷵻�NULL��
		T *Del(K key)
		{
			_Item *pItem = NULL;
			_Item *pTemp = m_pHead;
			for (uint32_t uiCur = 0; uiCur < m_uiL; uiCur++)
			{
				pItem = pTemp + key % m_arPrime[uiCur];
				//If Key Equal
				if (pItem->K == key)
				{
					pItem->K = 0;
					m_uiNum--;
					return pItem->value;
				}
				//If Empty
				if (pItem->K == 0)
				{
					return NULL;
				}
				pTemp += m_arPrime[uiCur];
			}
			return NULL;
		}

		uint32_t GetNum()
		{
			return m_uiNum;
		}

		void Seed(uint32_t uiPos = 0)
		{
			m_pCur = m_pHead + uiPos;
		}

		void Next(K **pKey, T **pVal)
		{
			*pKey = &m_pCur->key;
			*pVal = &m_pCur->value;
			m_pCur++;
		}

		void NextVal(K **pKey, T **pVal)
		{
			if (m_pCur - m_pHead < m_uiNum)
			{
				*pKey = NULL;
				*pVal = NULL;
			}
			while (m_pCur->key == 0)
			{
				m_pCur++;
			}
			*pKey = &m_pCur->key;
			*pVal = &m_pCur->value;
			m_pCur++;
		}

		uint32_t GetRealNum()
		{
			_Item *pItem = m_pHead;
			uint32_t uiRealCnt = 0;
			for (uint32_t uiCur = 0; uiCur < m_uiAllNum; uiCur++)
			{
				if (0 != pItem->key) {
					uiRealCnt++;
				}
				pItem++;
			}

			m_uiNum = uiRealCnt;
			return uiRealCnt;
		}

		void PrintState()
		{
			uint32_t uiCntZero = 0;
			cout << "The Bhash Has " << m_uiNum << " Elements And Load : " << endl;
			_Item *pItem = m_pHead;
			uint32_t uiAllCnt = 0;
			for (uint32_t uiCur = 0; uiCur < m_uiL; uiCur++)
			{
				uiCntZero = 0;
				for (uint32_t uiIdx = 0; uiIdx < m_arPrime[uiCur]; uiIdx++, pItem++)
				{
					if (0 == pItem->key) {
						uiCntZero++;
					}
				}
				uiCntZero = m_arPrime[uiCur] - uiCntZero;
				cout << " ***    The Floor : " << uiCur << "Real Data : " << uiCntZero << " All Place : " << m_arPrime[uiCur] << "  Data Percent : " << (double)uiCntZero / (double)m_arPrime[uiCur] << endl;
				uiAllCnt += uiCntZero;
			}
			cout << "The Bhash Has Real Elements : " << uiAllCnt << endl;
		}


	private:

		typedef struct
		{
			K key;
			T value;
		}_Item;

		uint32_t m_uiNum;

		uint32_t m_uiW;
		uint32_t m_uiL;
		size_t m_sizMem;

		key_t m_ShmKey;

		_Item *m_pHead;

		_Item *m_pCur;

		uint32_t *m_arPrime;

		string sErr;

		uint32_t m_uiAllNum;

		bool bPrime(uint32_t uiBase)
		{
			if (uiBase < 9)
			{
				return false;
			}
			uint32_t i = 3;
			while (i * i < uiBase)
			{
				if (uiBase % i == 0)
				{
					return false;
				}

				i += 2;
			}
			return true;
		}

		uint32_t InitPrime()
		{
			uint32_t uiSum = 0;
			uint32_t uiW = m_uiW;
			m_arPrime = new uint32_t[m_uiL];
			for (uint32_t ui = 0; ui < m_uiL;)
			{
				if (uiW % 2 != 0 && bPrime(uiW))
				{
					m_arPrime[ui] = uiW;
					uiSum += uiW;
					ui++;
				}
				uiW--;
				if (uiW == 0)
				{
					return 0;
				}
			}
			return uiSum;
		}


	};


}


#endif
