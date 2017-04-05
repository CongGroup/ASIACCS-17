#include "ShmCtl.h"
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h> 
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <assert.h>

//#define DEF_CARAVEL_DEBUG

using namespace std;

namespace caravel{

bool ShmCtl::GetShm(void **pShm, key_t kKey, size_t siLen)
{
    int iShmID;
    if((iShmID = shmget(kKey, siLen, IPC_CREAT | IPC_EXCL | 0666)) < 0)
    {
#ifdef DEF_CARAVEL_DEBUG
        cout<<"ERROR CREATE"<<endl;
#endif
        if(errno == EEXIST)
        {
#ifdef DEF_CARAVEL_DEBUG
            cout<<"EEXISTS"<<endl;
#endif
            if((iShmID = shmget(kKey, siLen, IPC_CREAT | 0666)) < 0) {
                struct shmid_ds stShmDs;
                if(shmctl(iShmID, IPC_STAT, &stShmDs) < 0)
                {
#ifdef DEF_CARAVEL_DEBUG
                    cout<<"IPC_STAT"<<endl;
#endif
                    return false;
                }
                else
                {
                    if(stShmDs.shm_segsz != siLen)
                    {
#ifdef DEF_CARAVEL_DEBUG
                        cout<<"LEN not equal !"<<endl;
#endif
                        return false;
                    }
                }
            }
        }
        else
        {
#ifdef DEF_CARAVEL_DEBUG
            cout<<"Other ERROR"<<endl;
#endif
            return false;
        }
    }
#ifdef DEF_CARAVEL_DEBUG
    cout<<"ID:"<<iShmID<<endl;
#endif
    *pShm = shmat(iShmID, NULL, 0);
    if(*pShm == (void *)-1)
    {
#ifdef DEF_CARAVEL_DEBUG
        cout<<errno<<endl;
        cout<<"Return -1"<<endl;
#endif
        return false;
    }
   return true;
}

}
