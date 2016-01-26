#include "mymalloc.h"
#include "LinLog.h"
#include <dlfcn.h>

pfnmalloc mymalloc;
pfnfree myfree;
pfncalloc mycalloc;
pfnrealloc myrealloc;
pfnmemalign mymemalign;
pfnmalloc_usable_size mymalloc_usable_size;

void initMyMalloc(void)
{
#define INIT_OR_CRASH(name) \
    my##name = reinterpret_cast<pfn##name>(dlsym(RTLD_NEXT, #name)); \
    if (my##name == NULL) { \
        LINLOG("fails to get my" #name " symbol.\n"); \
        __builtin_trap(); \
    }
    INIT_OR_CRASH(malloc);
    INIT_OR_CRASH(free);
    INIT_OR_CRASH(calloc);
    INIT_OR_CRASH(realloc);
    INIT_OR_CRASH(memalign);
    INIT_OR_CRASH(malloc_usable_size);
}
