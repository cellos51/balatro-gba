#ifndef BITSET_H
#define BITSET_H

#include <stdint.h>
#include <stdbool.h>

#define BITSET_BITS_PER_WORD 32
#define BITSET_ARRAY_SIZE     8

typedef struct Bitset
{
    uint32_t *w;
    uint32_t nbits;
    uint32_t nwords;
    uint32_t cap;
} Bitset;

typedef struct
{
    const Bitset *bitset;
    int itr; // the iteration we are on
} BitsetItr;

void bitset_set_idx(Bitset *bitset, int idx, bool on);
bool bitset_get_idx(Bitset *bm, int idx);
int bitset_get_free_idx(Bitset *bitset);
void bitset_clear(Bitset *bitset);
bool bitset_is_empty(Bitset *bitset);
int bitset_num_set_bits(Bitset *bitset);

int bitset_find_of_nth_set(const Bitset *bitset, int n);
// temporary function for testing iterator
int bitset_get_next_free_idx(const Bitset *bitset, int start);
BitsetItr bitset_itr_new(const Bitset* bitset);
int bitset_itr_next(BitsetItr* itr);

#define BITSET_DEFINE(name, capacity)                         \
    static uint32_t name##_bitset_w[BITSET_ARRAY_SIZE] = {0}; \
    static Bitset name =                                      \
    {                                                         \
        .w = name##_bitset_w,                                 \
        .nbits = BITSET_BITS_PER_WORD,                        \
        .nwords = BITSET_ARRAY_SIZE,                          \
        .cap = capacity,                                      \
    };


#endif // BITSET_H
