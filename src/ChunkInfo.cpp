#include <stddef.h>
#include <string.h>
#include <unwind.h>
#include "ChunkInfo.h"

typedef struct {
    size_t count;
    size_t ignore;
    const void** addrs;
} stack_crawl_state_t;

static
_Unwind_Reason_Code trace_function(_Unwind_Context *context, void *arg)
{
    stack_crawl_state_t* state = (stack_crawl_state_t*)arg;
    if (state->count) {
        void* ip = (void*)_Unwind_GetIP(context);
        if (ip) {
            if (state->ignore) {
                state->ignore--;
            } else {
                state->addrs[0] = ip; 
                state->addrs++;
                state->count--;
            }
        }
    }
    else
    {
        return _URC_FAILURE; // force break
    }
    return _URC_NO_REASON;
}

static
int backtrace(const void** addrs, size_t ignore, size_t size)
{
    stack_crawl_state_t state;
    state.count = size;
    state.ignore = ignore;
    state.addrs = addrs;
    _Unwind_Backtrace(trace_function, (void*)&state);
    return size - state.count;
}

void ChunkInfo::get(ChunkInfo & info,void * )
{
    memset(&info,0,sizeof(ChunkInfo));
    int Len = backtrace(info.m_backtraces,1,ChunkInfo::MAX_BACKTRACES);
    info.m_backtracesLen = Len;
}

