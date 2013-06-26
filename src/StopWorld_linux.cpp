#include "StopWorld.h"
#include <signal.h>
#include <sys/syscall.h>
#include <pthread.h>
#include <unistd.h>
#include <algorithm>
#include <errno.h>
#include <android/log.h>

void sendUcontext(int fd,void* ucontext);
void storeOthersContext(int fd);
static void   myaction(int, siginfo_t *, void *);
static struct sigaction oldAction;
static int mytid;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static void * g_context[64];
static int g_contextLen;
static bool g_shouldSet;

static void   myaction(int , siginfo_t * info, void *ucontext)
{
    __android_log_print(ANDROID_LOG_DEBUG,"LIN","thread %d enter myaction\n",syscall(__NR_gettid,0));
    // if(syscall(__NR_gettid,0) == mytid)
    // {
    //     // mytid just continue
    //     // wait for other threads to proceed
    //     sleep(1);
    //     pthread_mutex_lock(&mutex);
    //     g_shouldSet = false;
    //     pthread_mutex_unlock(&mutex);
    // }
    // else
    // {
    //     pthread_mutex_lock(&mutex);
    //     if(!g_shouldSet)
    //     {
    //         pthread_mutex_unlock(&mutex);
    //         return;
    //     }
    //     // store the context
    //     if(g_contextLen != 64)
    //         g_context[g_contextLen++] = ucontext;        
    //     pthread_cond_wait(&cond,&mutex);
    //     pthread_mutex_unlock(&mutex);
    // }
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
    mytid = syscall(__NR_gettid,0);
    g_contextLen = 0;    
    ret = sigaction(47,&newAction,&oldAction);
    if(ret != 0)
    {
        return false;
    }
    g_shouldSet = true;
    if(kill(getpid(),47))
    {
        __android_log_print(ANDROID_LOG_ERROR,"LIN","failed to send signal:%s\n",strerror(errno));
    }
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

