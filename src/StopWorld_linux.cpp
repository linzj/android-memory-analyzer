#include "StopWorld.h"
#include <signal.h>
#include <sys/syscall.h>
#include <pthread.h>
#include <unistd.h>
#include <algorithm>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>

void sendUcontext(int fd,void* ucontext);
void storeOthersContext(int fd);
static void   myaction(int, siginfo_t *, void *);
static struct sigaction oldAction;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static void * g_context[64];
static int g_contextLen;
static bool g_shouldSet;

static void   myaction(int , siginfo_t * info, void *ucontext)
{
    pthread_mutex_lock(&mutex);
    if(!g_shouldSet)
    {
        pthread_mutex_unlock(&mutex);
        return;
    }
    // store the context
    g_context[g_contextLen++] = ucontext;        
    pthread_cond_wait(&cond,&mutex);
    pthread_mutex_unlock(&mutex);
}

static bool boardcastSignal(int num,int selfTid,int selfPid,int maxSendCount)
{
    int fd = open("/proc/self/task",O_RDONLY|O_DIRECTORY);
    if(fd == -1)
    {
        return false;
    }
    struct dirent    _DIR_buff[15];
    int sendCount = 0;
    while(sendCount != maxSendCount)
    {
        int ret = getdents(fd,_DIR_buff,sizeof(_DIR_buff));
        if(ret <= 0)
        {
            return true;
        }
        struct dirent * iterator = _DIR_buff;
        while(ret)
        {
            ret -= iterator->d_reclen;
            struct dirent * cur = iterator;
            iterator = reinterpret_cast<struct dirent*>(reinterpret_cast<char*>(iterator) + cur->d_reclen);
            if(!strcmp(cur->d_name,".") 
                || !strcmp(cur->d_name,".."))
            {
                continue;
            }
            int curPid = static_cast<int>(strtoul(cur->d_name,NULL,10));
            if(curPid != selfTid)
            {
                syscall(__NR_tgkill,selfPid,curPid,47);
            }
            if(++sendCount == maxSendCount)
            {
                break;
            }
        }
    }
    return true;
}


//this function is not thread safe!
bool stopTheWorld(void)
{
    int ret;
    struct sigaction newAction ;  
    newAction.sa_sigaction = myaction;
    newAction.sa_mask = 0;
    newAction.sa_flags = SA_SIGINFO;
    newAction.sa_restorer = NULL; 
    int mytid = syscall(__NR_gettid,0);
    g_contextLen = 0;    
    ret = sigaction(47,&newAction,&oldAction);
    if(ret != 0)
    {
        return false;
    }
    g_shouldSet = true;
    if(boardcastSignal(47,mytid,getpid(),64))
    {
    }
    sleep(1);
    pthread_mutex_lock(&mutex);
    g_shouldSet = false;
    pthread_mutex_unlock(&mutex);
    return true;
}

void restartTheWorld(void)
{
    sigaction(47,&oldAction,NULL);
    pthread_mutex_lock(&mutex);
    pthread_cond_broadcast(&cond);
    pthread_mutex_unlock(&mutex);
}

int getUserContext(void ** buf,int len)
{
    int storeLen = std::min(len,g_contextLen);
    for(int i = 0 ; i < storeLen; ++i)
    {
        buf[i] = g_context[i];
    }
    return storeLen;
}

