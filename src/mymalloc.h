#ifndef MYMALLOC_H
#define MYMALLOC_H
#include <stddef.h>
#include <stdint.h>
typedef uint32_t uptr;
typedef void* (*pfnmalloc)(uptr bytes);
typedef void (*pfnfree)(void* data);
typedef void* (*pfncalloc)(uptr n_elements, uptr elem_size);
typedef void* (*pfnrealloc)(void* oldMem, uptr bytes);
typedef void* (*pfnmemalign)(uptr alignment, uptr bytes);
typedef size_t (*pfnmalloc_usable_size)(const void* ptr);

extern pfnmalloc mymalloc;
extern pfnfree myfree;
extern pfncalloc mycalloc;
extern pfnrealloc myrealloc;
extern pfnmemalign mymemalign;
extern pfnmalloc_usable_size mymalloc_usable_size;
void initMyMalloc(void);
#endif /* MYMALLOC_H */
