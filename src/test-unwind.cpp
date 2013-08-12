#include <unwind.h>
#include <stdio.h>

// copy from  ChunkInfo.cpp
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
        return static_cast<_Unwind_Reason_Code>(1); // force break
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

static void recursive(int a)
{
    if(a != 100)
    {
        recursive(a+1);
    }
    else
    {
        const void * addr[20];
        int count = backtrace(addr,0,20);
        for(int i = 0 ; i < count ; ++i)
        {
            printf("%08lx\n",reinterpret_cast<unsigned long>(addr[i]));
        }
    }
}

int main()
{
    recursive(0);
}
