#include <unwind.h>
#include <stdio.h>
#include <stdlib.h>
#include  <stdint.h>

#define WRAP(x) _##x

typedef uint32_t uptr;
struct MallocDebug {
  void* (*malloc)(uptr bytes);
  void  (*free)(void* mem);
  void* (*calloc)(uptr n_elements, uptr elem_size);
  void* (*realloc)(void* oldMem, uptr bytes);
  void* (*memalign)(uptr alignment, uptr bytes);
};
static void* _malloc(uptr bytes);
static void  _free(void* data);
static void* _calloc(uptr n_elements, uptr elem_size);
static void* _realloc(void* oldMem, uptr bytes);
static void* _memalign(uptr alignment, uptr bytes);
static const MallocDebug * g_old;

const MallocDebug _malloc_dispatch __attribute__((aligned(32))) = {
  WRAP(malloc), WRAP(free), WRAP(calloc), WRAP(realloc), WRAP(memalign)
};

static const MallocDebug * overrideMalloc()
{
    extern const MallocDebug* __libc_malloc_dispatch;
    const MallocDebug * old = __libc_malloc_dispatch;
    __libc_malloc_dispatch = &_malloc_dispatch;
    return old;
}

static void restoreMalloc()
{
    extern const MallocDebug* __libc_malloc_dispatch;

    __libc_malloc_dispatch = g_old;
}

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
static void printUnwind()
{
    const void * addr[20];
    int count = backtrace(addr,2,20);
    printf("got %d backtrace\n",count);
    for(int i = 0 ; i < count ; ++i)
    {
        printf("%08lx\n",reinterpret_cast<unsigned long>(addr[i]));
    }
}

static void recursive(int a)
{
    if(a != 10)
    {
        recursive(a+1);
    }
    else
    {
        restoreMalloc();
        printUnwind();
        overrideMalloc();
    }
}

int main()
{
    g_old = overrideMalloc();
    // recursive(0);
    void * a = malloc(10);
    free(a);
    return 0;
}

static void* _malloc(uptr bytes)
{
    restoreMalloc();
    printf("malloc:\n");
    printUnwind();
    void* ret = g_old->malloc(bytes);
    overrideMalloc();
    return ret;
}
static void  _free(void* data)
{
    restoreMalloc();
    printf("free:\n");
    printUnwind();
    g_old->free(data);
    overrideMalloc();
}
static void* _calloc(uptr n_elements, uptr elem_size)
{
    restoreMalloc();
    printf("malloc:\n");
    printUnwind();
    void * ret = g_old->calloc(n_elements,elem_size);
    overrideMalloc();
    return ret;
}
static void* _realloc(void* oldMem, uptr bytes)
{
    restoreMalloc();
    printf("malloc:\n");
    printUnwind();
    void * ret = g_old->realloc(oldMem,bytes);
    overrideMalloc();
    return ret;
}

static void* _memalign(uptr alignment, uptr bytes)
{
    restoreMalloc();
    printf("malloc:\n");
    printUnwind();
    void * ret = g_old->memalign(alignment,bytes);

    overrideMalloc();
    return ret;
}
