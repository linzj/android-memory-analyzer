#ifndef GHASH_H
#define GHASH_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
void* g_memdup(const void* src, size_t s);
typedef struct _GHashTable GHashTable;
typedef uint8_t gboolean;
typedef char gchar;
typedef int gint;
typedef void* gpointer;
typedef unsigned guint;
typedef uint32_t guint32;
typedef int64_t gint64;
typedef double gdouble;
typedef size_t gsize;
typedef unsigned long gulong;
typedef const void* gconstpointer;
typedef guint (*GHashFunc)(gconstpointer key);
typedef gboolean (*GEqualFunc)(gconstpointer a,
    gconstpointer b);
typedef void (*GDestroyNotify)(gpointer data);
typedef gboolean (*GHRFunc)(gpointer key,
    gpointer value,
    gpointer user_data);
typedef void (*GHFunc)(gpointer key,
    gpointer value,
    gpointer user_data);

void* g_hash_table_lookup(GHashTable* hash_table,
    const void* key);
extern gboolean g_hash_table_insert(GHashTable* hash_table,
    gpointer key,
    gpointer value);
extern void
g_hash_table_foreach(GHashTable* hash_table,
    GHFunc func,
    gpointer user_data);
extern GHashTable* g_hash_table_new_full(GHashFunc hash_func,
    GEqualFunc key_equal_func,
    GDestroyNotify key_destroy_func,
    GDestroyNotify value_destroy_func);
extern GHashTable*
g_hash_table_new(GHashFunc hash_func,
    GEqualFunc key_equal_func);
extern gpointer
g_hash_table_lookup(GHashTable* hash_table,
    gconstpointer key);
extern void
g_hash_table_destroy(GHashTable* hash_table);
extern gboolean
g_hash_table_remove (GHashTable    *hash_table,
                     gconstpointer  key);
#define G_PASTE_ARGS(identifier1, identifier2) identifier1##identifier2
#define G_PASTE(identifier1, identifier2) G_PASTE_ARGS(identifier1, identifier2)
#if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ > 4)
#define G_GNUC_PRINTF(format_idx, arg_idx) \
    __attribute__((__format__(__printf__, format_idx, arg_idx)))
#define G_GNUC_SCANF(format_idx, arg_idx) \
    __attribute__((__format__(__scanf__, format_idx, arg_idx)))
#define G_GNUC_FORMAT(arg_idx) \
    __attribute__((__format_arg__(arg_idx)))
#define G_GNUC_NORETURN \
    __attribute__((__noreturn__))
#define G_GNUC_CONST \
    __attribute__((__const__))
#define G_GNUC_UNUSED \
    __attribute__((__unused__))
#define G_GNUC_NO_INSTRUMENT \
    __attribute__((__no_instrument_function__))
#else /* !__GNUC__ */
#define G_GNUC_PRINTF(format_idx, arg_idx)
#define G_GNUC_SCANF(format_idx, arg_idx)
#define G_GNUC_FORMAT(arg_idx)
#define G_GNUC_NORETURN
#define G_GNUC_CONST
#define G_GNUC_UNUSED
#define G_GNUC_NO_INSTRUMENT
#endif /* !__GNUC__ */

#define G_STATIC_ASSERT(expr) typedef char G_PASTE(_GStaticAssertCompileTimeAssertion_, __COUNTER__)[(expr) ? 1 : -1] G_GNUC_UNUSED
#if defined(__GNUC__)
#define _g_alignof(type) (__alignof__(type))
#else
#define _g_alignof(type) (G_STRUCT_OFFSET(struct { char a; type b; }, b))
#endif

#define G_STRINGIFY(macro_or_string) G_STRINGIFY_ARG (macro_or_string)
#define G_STRINGIFY_ARG(contents) #contents

#define g_new(struct_type, n_structs) _G_NEW(struct_type, n_structs, malloc)
#define g_new0(struct_type, n_structs) _G_NEW(struct_type, n_structs, malloc)

#define _G_NEW(struct_type, n_structs, func) \
    ((struct_type*)g_##func##_n((n_structs), sizeof(struct_type)))
#define _G_RENEW(struct_type, mem, n_structs, func) \
    ((struct_type*)g_##func##_n(mem, (n_structs), sizeof(struct_type)))

void g_free(gpointer mem);
gpointer
g_malloc_n(gsize n_blocks,
    gsize n_block_bytes);
guint g_int_hash(gconstpointer v);
gboolean
g_int_equal(gconstpointer v1,
    gconstpointer v2);

#define G_UNLIKELY(expr) (__builtin_expect(!!(expr), 0))
#if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 8)
#define G_GNUC_EXTENSION __extension__
#else
#define G_GNUC_EXTENSION
#endif
#ifdef __cplusplus
}
#endif
#endif /* GHASH_H */
