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

// Set the flag at the 'idx' index in the bitset
void bitset_set_idx(Bitset *bitset, int idx, bool on);
// Get the flag at the 'idx' index in the bitset
bool bitset_get_idx(Bitset *bm, int idx);
// Get the next free (set to 0) index in the bitset.
// It also sets the bit which it maybe should do... It really shouldn't do two things
// But it's such a fast operation idk. // TODO: decide what you wanna do
int bitset_get_free_idx(Bitset *bitset);
// Clear the entire bitset
void bitset_clear(Bitset *bitset);
// Returns true if no bits are set
bool bitset_is_empty(Bitset *bitset);
// Returns the number of set bits
int bitset_num_set_bits(Bitset *bitset);
// Find the index at the nth set bit
int bitset_find_idx_of_nth_set(const Bitset *bitset, int n);
// Get a new bitset iterator
BitsetItr bitset_itr_new(const Bitset* bitset);
// Get the next set bit index at the next iteration. If it's at the end, return UNDEFINED instead.
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
