#ifndef __TIMEDIFF_H__
#define __TIMEDIFF_H__

#include <stdint.h>
#include <time.h>
#include <sys/time.h>

namespace caravel {

	class TimeDiff
	{
	public:

		static uint32_t DiffTimeInMicroSecond();

		static uint32_t DiffTimeInSecond();

	};

}

#endif

