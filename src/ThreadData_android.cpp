#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <pthread.h>
#include "HeapServer.h"
typedef struct pthread_internal_t
{
    struct pthread_internal_t*  next;
    struct pthread_internal_t** prev;
    pthread_attr_t              attr;
    pid_t                       kernel_id;
    pthread_cond_t              join_cond;
    int                         join_count;
    void*                       return_value;
    int                         internal_flags;
    __pthread_cleanup_t*        cleanup_stack;
    void**                      tls;         /* thread-local storage area */
} pthread_internal_t;

static void sendOne(int fd,pthread_internal_t const * t);
static void sendNext(int fd,pthread_internal_t const * internal)
{
    pthread_internal_t const * cur = internal;
    for(;cur;cur = cur->next)
    {
        sendOne(fd,cur);
    }
}

static void sendOne(int fd,pthread_internal_t const * t)
{
    pthread_t cur = reinterpret_cast<pthread_t>(const_cast<pthread_internal_t*>(t));
    pthread_attr_t sattr;
    pthread_attr_init(&sattr);
    pthread_getattr_np(cur, &sattr);
    void * stackBase = NULL;
    size_t stackSize = 0;
    int rc = pthread_attr_getstack(&sattr, &stackBase, &stackSize);
    size_t guard_size = 0;
    pthread_attr_getguardsize(&sattr,&guard_size);
    pthread_attr_destroy(&sattr);
    stackBase = static_cast<char*>(stackBase) + guard_size;
    SendOnceGeneral once = { stackBase,stackSize, 0x80000000 };
    fprintf(stderr,"LIN:sending thread %p,%d\n",once.m_chunk,once.m_len);
    sendTillEnd(fd,reinterpret_cast<const char *>(&once),sizeof(once));
    sendTillEnd(fd,reinterpret_cast<const char*>(stackBase),stackSize);
}

void sendThreadData(int fd)
{
    pthread_t current =  pthread_self();
    pthread_internal_t * current_internal = reinterpret_cast<pthread_internal_t*>(current);
    // find first
    while(current_internal->prev[0] != current_internal)
    {
        current_internal = current_internal->prev[0];
    }
    sendNext(fd,current_internal);
}

