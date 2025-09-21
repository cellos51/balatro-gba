#ifndef POOL_H
#define POOL_H

#include <tonc.h>

#define POOL_BITS_PER_WORD 32
#define POOL_MAX_CAPACITY 128

typedef struct PoolBitmap {
    u32 *w;
    u32 nbits;
    u32 nwords;
} PoolBitmap;

void pool_bm_clear_idx(PoolBitmap *bm, int idx);
int pool_bm_get_free_idx(PoolBitmap *bm);

#define POOL_DECLARE_TYPE(type)                                             \
    typedef struct                                                          \
    {                                                                       \
        PoolBitmap bm;                                                      \
        type *  objects;                                                    \
        u32 max_entries;                                                    \
    } type##Pool;                                                           \
    type *pool_get_##type();                                                \
    void  pool_free_##type(type *obj);                                      \

#define POOL_DEFINE_TYPE(type, capacity)                                    \
    static type type##_storage[capacity];                                   \
    static u32 type##_bitmap_w[sizeof(u32)] = {0};                          \
    static type##Pool type##_pool =                                         \
    {                                                                       \
        .bm = {                                                             \
            .w = type##_bitmap_w,                                           \
            .nbits = POOL_BITS_PER_WORD,                                    \
            .nwords = sizeof(u32)},                                         \
        .objects = type##_storage,                                          \
        .max_entries = capacity,                                            \
    };                                                                      \
    type * pool_get_##type()                                                \
    {                                                                       \
        int free_offset = pool_bm_get_free_idx(&type##_pool.bm);            \
        if(free_offset == -1) return NULL;                                  \
        return &type##_pool.objects[free_offset];                           \
    }                                                                       \
    void pool_free_##type(type *entry)                                      \
    {                                                                       \
        if(entry == NULL) return;                                           \
        int offset = entry - &type##_pool.objects[0];                       \
        pool_bm_clear_idx(&type##_pool.bm, offset);                         \
    }

#define POOL_GET(type) pool_get_##type()
#define POOL_FREE(type, obj) pool_free_##type(obj)

#define POOL_ENTRY(name, capacity) \
POOL_DECLARE_TYPE(name);
#include "pools.def"
#undef POOL_ENTRY

#endif // POOL_H
