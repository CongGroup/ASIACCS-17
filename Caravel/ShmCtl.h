#ifndef __SHMCTL_H__
#define __SHMCTL_H__
#include <sys/shm.h>

namespace caravel {

	class ShmCtl
	{
	public:

		static bool GetShm(void **pShm, key_t kKey, size_t siLen);

	};

}

#endif
