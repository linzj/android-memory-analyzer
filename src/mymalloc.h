#ifndef MYMALLOC_H
#define MYMALLOC_H
#include <stddef.h>
#include <stdint.h>
#include <sys/mman.h>
typedef uint32_t uptr;
typedef void* (*pfnmalloc)(uptr bytes);
typedef void (*pfnfree)(void* data);
typedef void* (*pfncalloc)(uptr n_elements, uptr elem_size);
typedef void* (*pfnrealloc)(void* oldMem, uptr bytes);
typedef void* (*pfnmemalign)(uptr alignment, uptr bytes);
typedef size_t (*pfnmalloc_usable_size)(const void* ptr);
typedef void* (*pfnmmap)(void* addr, size_t length, int prot, int flags, int fd, off_t offset);
typedef int (*pfnmunmap)(void* addr, size_t length);

extern pfnmalloc mymalloc;
extern pfnfree myfree;
extern pfncalloc mycalloc;
extern pfnrealloc myrealloc;
extern pfnmemalign mymemalign;
extern pfnmalloc_usable_size mymalloc_usable_size;
extern pfnmmap mymmap;
extern pfnmunmap mymunmap;
void initMyMalloc(void);
#endif /* MYMALLOC_H */
