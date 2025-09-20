#ifndef POOL_H
#define POOL_H

#include <tonc.h>

#define POOL_BITS_PER_WORD 32
#define POOL_WORD_T u32
#define POOL_MAX_CAPACITY 128

typedef struct PoolBitmap {
    u32 *w;
    u32 nbits;
    u32 nwords;
} PoolBitmap;

void pool_clear_idx(PoolBitmap *bm, int idx);
int pool_get_free_idx(PoolBitmap *bm);

#define DECLARE_POOL_TYPE(type)                                             \
    typedef struct type##Pool {                                             \
        PoolBitmap bm;                                                      \
        type *  objects;                                                    \
        u32 max_entries;                                                    \
    } type##Pool;                                                           \
    type *pool_get_##type(type##Pool* pool);                                \
    void  pool_free_##type(type##Pool *pool, type *obj);                    \
    void  pool_init_##type(type##Pool *pool);

#define DEFINE_POOL_TYPE(type, capacity)                                    \
    static type type##_storage[capacity];                                   \
    static u32 type##_bitmap_w[sizeof(POOL_WORD_T)] = {0};                  \
    static type##Pool type##_pool = {                                       \
        .bm = {                                                             \
            .w = type##_bitmap_w,                                           \
            .nbits = POOL_BITS_PER_WORD,                                    \
            .nwords = sizeof(POOL_WORD_T)},                                 \
        .objects = type##_storage,                                          \
        .max_entries = capacity,                                            \
    };                                                                      \
    type * pool_get_##type(type##Pool *pool) {                              \
        int free_offset = pool_get_free_idx(&pool->bm);                     \
        if(free_offset == -1) return NULL;                                  \
        return &pool->objects[free_offset];                                 \
    }                                                                       \
    void pool_free_##type(type##Pool *pool, type *entry) {                  \
        if(entry == NULL) return;                                           \
        int offset = entry - &pool->objects[0];                             \
        pool_clear_idx(&pool->bm, offset);                                  \
    }

#define POOL_GET(type) pool_get_##type(&type##_pool)
#define POOL_FREE(type, obj) pool_free_##type(&type##_pool, obj)

#endif
