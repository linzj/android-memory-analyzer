#include "StopWorld.h"
#include <signal.h>
#include <syscall.h>
#include <pthread.h>
#include <ucontext.h>
#include <unistd.h>
#include <algorithm>

void sendUcontext(int fd,void* ucontext);
void storeOthersContext(int fd);
static void   myaction(int, siginfo_t *, void *);
static struct sigaction oldAction;
static int mytid;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static void * g_context[64];
static int g_contextLen;

static void   myaction(int , siginfo_t * info, void *ucontext)
{
    if(syscall(__NR_gettid,0) == mytid)
    {
        // mytid just continue
    }
    else
    {
        pthread_mutex_lock(&mutex);
        // store the context
        if(g_contextLen != 64)
            g_context[g_contextLen++] = ucontext;        
        pthread_cond_wait(&cond,&mutex);
        pthread_mutex_unlock(&mutex);
    }
}
//this function is not thread safe!
bool stopTheWorld(void)
{
    int ret;
    struct sigaction newAction = {myaction,0,SA_SIGINFO,NULL}; 
    mytid = syscall(__NR_gettid,0);
    g_contextLen = 0;    
    ret = sigaction(47,&newAction,&oldAction);
    if(ret != 0)
    {
        return false;
    }
    kill(getpid(),-47);
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

