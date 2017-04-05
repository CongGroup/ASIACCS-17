#ifndef __CONHASH_H__
#define __CONHASH_H__

#include <stdint.h>
#include <string>
#include <string.h>
#include <iostream>


using namespace std;

namespace caravel {

	template<class T>
	class ConHash
	{
	public:
		ConHash(void)
		{}


		~ConHash(void)
		{}

		bool Init(char* pShArray, uint8_t *pSemV, uint32_t uiAll, SemCtl *pSem, int iSem, uint32_t uiW = 0)
		{
			m_pShm = pShArray;
			m_pBlock = (T*)pShArray;
			m_uiW = uiW;
			memset(&m_Empty, 0, sizeof(m_Empty));
			m_pSemCtl = pSem;
			m_iSem = iSem;
			pSem->SetSem(iSem, 1);
			m_uiAll = uiAll;
			m_cV = pSemV;
			return true;
		}

		//UnsafeGet & Set

		T *Get(uint32_t uiOffset)
		{
			return m_pBlock + uiOffset;
		}

		T *Get(uint32_t uiL, uint32_t uiOffset)
		{
			return Get(uiL * m_uiW + uiOffset);
		}

		//SafeGet V1
		//Carefully use!
		//You must memcpy the T* to a buffer , then test (vs & 1 != 1 AND vs == **ve) , if true, can use the buffer.
		//Do not use the T* directly.

		T *SafeGet(uint32_t uiOffset, uint8_t &vs, uint8_t **ve)
		{
			T *pBlock;
			pBlock = m_pBlock + uiOffset;
			vs = m_cV[uiOffset];
			*ve = m_cV + uiOffset;
			return pBlock;
		}

		T *SafeGet(uint32_t uiL, uint32_t uiOffset, uint8_t &vs, uint8_t **ve)
		{
			return SafeGet(uiL * m_uiW + uiOffset, vs, ve);
		}

		//SafeGet V2

		bool SafeGet(uint32_t uiOffset, T *pData)
		{
			T *pBlock;
			pBlock = m_pBlock + uiOffset;
			uint8_t vs, ve;
			while (true)
			{
				vs = m_cV[uiOffset];
				if (memcmp(pBlock, &m_Empty, sizeof(T)) == 0)
				{
					//The block is empty
					return false;
				}
				memcpy(pData, pBlock, sizeof(T));
				ve = m_cV[uiOffset];
				if ((vs & 1 == 1) || (vs != ve))
				{
					continue;
				}
				return true;
			}
		}

		bool SafeGet(uint32_t uiL, uint32_t uiOffset, T *pData)
		{
			return SafeGet(uiL * m_uiW + uiOffset, pData);
		}

		//SafeSet V1
		bool SafeSet(uint32_t uiOffset, T *pData)
		{
			//lock();s
			m_pSemCtl->ModSem(m_iSem, -1);
			T *pBlock;
			pBlock = m_pBlock + uiOffset;
			if (memcmp(pBlock, &m_Empty, sizeof(T)) != 0)
			{
				//Not Empty
				//unlock();
				m_pSemCtl->ModSem(m_iSem, 1);
				return false;
			}
			m_cV[uiOffset]++;
			memcpy(pBlock, pData, sizeof(T));
			m_cV[uiOffset]++;

			//unlock();
			m_pSemCtl->ModSem(m_iSem, 1);
			return true;
		}

		bool SafeSet(uint32_t uiL, uint32_t uiOffset, T *pData)
		{
			return SafeSet(uiL * m_uiW + uiOffset, pData);
		}


	private:

		char *m_pShm;
		T *m_pBlock;
		uint32_t m_uiW;

		T m_Empty;

		SemCtl *m_pSemCtl;
		int m_iSem;

		uint8_t *m_cV;
		uint32_t m_uiAll;

	};

}


#endif
