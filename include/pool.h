#ifndef POOL_H
#define POOL_H

#include <stdint.h>

#ifdef POOLS_TEST_ENV
#define POOLS_DEF_FILE "def_test_mempool.h"
#else
#define POOLS_DEF_FILE "def_balatro_mempool.h"
#endif

#define POOL_BITS_PER_WORD 32
#define POOL_BITMAP_BYTES   8

typedef struct PoolBitmap {
    uint32_t *w;
    uint32_t nbits;
    uint32_t nwords;
    uint32_t cap;
} PoolBitmap;

void pool_bm_clear_idx(PoolBitmap *bm, int idx);
int pool_bm_get_free_idx(PoolBitmap *bm);

#define POOL_DECLARE_TYPE(type)                                             \
    typedef struct                                                          \
    {                                                                       \
        PoolBitmap bm;                                                      \
        type *  objects;                                                    \
    } type##Pool;                                                           \
    type *pool_get_##type();                                                \
    void  pool_free_##type(type *obj);                                      \

#define POOL_DEFINE_TYPE(type, capacity)                                    \
    static type type##_storage[capacity];                                   \
    static uint32_t type##_bitmap_w[POOL_BITMAP_BYTES] = {0};               \
    static type##Pool type##_pool =                                         \
    {                                                                       \
        .bm = {                                                             \
            .w = type##_bitmap_w,                                           \
            .nbits = POOL_BITS_PER_WORD,                                    \
            .nwords = POOL_BITMAP_BYTES,                                    \
            .cap = capacity,                                                \
        },                                                                  \
        .objects = type##_storage,                                          \
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
#include POOLS_DEF_FILE
#undef POOL_ENTRY

#endif // POOL_H
