#include "TimeDiff.h"
#include <time.h>

namespace caravel {

uint32_t TimeDiff::DiffTimeInMicroSecond() {

    static timeval t_start, t_end;
    gettimeofday(&t_end, NULL);
    uint32_t uiTimeInterval = 1000000 * (t_end.tv_sec - t_start.tv_sec) + t_end.tv_usec - t_start.tv_usec;
    gettimeofday(&t_start, NULL);

    return uiTimeInterval;
}

uint32_t TimeDiff::DiffTimeInSecond() {

    static time_t t_cur;
    uint32_t ui_Time = time(NULL) - t_cur;
    t_cur = time(NULL);

    return ui_Time;
}


}
