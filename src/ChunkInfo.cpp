#include <stddef.h>
#include <string.h>
#include "backtrace.h"
#include <stdint.h>
#include "ChunkInfo.h"
#include "LinLog.h"
#ifdef __arm__
#define FAST_MODE 0
#elif defined(__i386__) || defined(__x86_64)
#define FAST_MODE 1
#endif

#if !defined(FAST_MODE) || FAST_MODE == 0
typedef struct {
    size_t count;
    size_t ignore;
    const void** addrs;
} stack_crawl_state_t;

static int trace_function(uintptr_t _ip, void* arg)
{
    stack_crawl_state_t* state = (stack_crawl_state_t*)arg;
    if (state->count) {
        void* ip = (void*)_ip;
        if (ip) {
            if (state->ignore) {
                state->ignore--;
            } else {
                state->addrs[0] = ip;
                state->addrs++;
                state->count--;
            }
        }
    } else {
        return BACKTRACE_ABORT; // force break
    }
    return BACKTRACE_CONTINUE;
}

#ifdef __i386__
static int fast_backtrace(const void** addrs, size_t ignore, size_t size)
{
    uintptr_t ebp;
    uintptr_t esp;
    int oldSize = size;
    asm("mov %%ebp,%0\n"
        : "=r"(ebp)
        :
        :);
    asm("mov %%esp,%0\n"
        : "=r"(esp)
        :
        :);

    while (ebp && size && (ebp >= esp) && (ebp <= (esp + 4096 * 100))) {
        do {
            if (ignore) {
                ignore--;
                break;
            }
            // next to ebp is the ip
            uintptr_t ip = reinterpret_cast<uintptr_t*>(ebp)[1];
            addrs[0] = reinterpret_cast<const void*>(ip);
            addrs++;
            size--;
        } while (0);
        ebp = *reinterpret_cast<uintptr_t*>(ebp);
    }
    return oldSize - size;
}
#endif //__i386__

#ifdef __arm__

static int fast_backtrace(const void** addrs, size_t ignore, size_t size)
{
    uintptr_t fp;
    uintptr_t sp;
    int oldSize = size;
    asm("mov %0,%%fp\n"
        : "=r"(fp)
        :
        :);
    asm("mov %0,%%sp\n"
        : "=r"(sp)
        :
        :);

    while (fp && size && (fp >= sp) && (fp <= (sp + 4096 * 100))) {
        do {
            if (ignore) {
                ignore--;
                break;
            }
            // next to fp is the ip
            uintptr_t ip = reinterpret_cast<uintptr_t*>(fp)[1];
            addrs[0] = reinterpret_cast<const void*>(ip);
            addrs++;
            size--;
        } while (0);
        fp = *reinterpret_cast<uintptr_t*>(fp);
    }
    return oldSize - size;
}
#endif //__arm__

static int backtrace(const void** addrs, size_t ignore, size_t size)
{
    stack_crawl_state_t state;
    state.count = size;
    state.ignore = ignore;
    state.addrs = addrs;
    mybacktrace(trace_function, (void*)&state);
    int unwind_count = size - state.count;
    if (unwind_count == 0) {
        // special handle this case.
        fast_backtrace(addrs, ignore, size);
    }
    return size - state.count;
}
#else

static int backtrace(const void** addrs, size_t ignore, size_t size)
{
    return fast_backtrace(addrs, ignore, size);
}
#endif //FAST_MODE

void ChunkInfo::get(ChunkInfo& info, void*)
{
    memset(&info, 0, sizeof(ChunkInfo));
    int Len = backtrace(info.m_backtraces, 2, ChunkInfo::MAX_BACKTRACES);
    info.m_backtracesLen = Len;
}
