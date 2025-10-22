#include "pool.h"
#include "util.h"

void pool_bm_clear_idx(PoolBitmap *bm, int idx)
{
    uint32_t i = idx / POOL_BITS_PER_WORD;
    uint32_t b = idx % POOL_BITS_PER_WORD;

    // Below are the "fast" forms of the above operations, respectively.
    // These are more efficient, but removed for readability
    // See: https://github.com/cellos51/balatro-gba/pull/132#discussion_r2365966071
    // Divide by 32 to get the word index
    //uint32_t i = idx >> 5;
    // Get last 5-bits, same as a modulo (% 32) operation on positive numbers
    //uint32_t b = idx & 0x1F;
    bm->w[i] &= ~((uint32_t)1 << b);
}

int pool_bm_get_free_idx(PoolBitmap *bm)
{
    for (uint32_t i = 0; i < bm->nwords; i++)
    {
        uint32_t inv = ~bm->w[i];

        // guard so we don't call `ctz` with 0, since __builtin_ctz(0) is undefined
        // https://gcc.gnu.org/onlinedocs/gcc/Bit-Operation-Builtins.html#index-_005f_005fbuiltin_005fctz
        //
        // By using the bitwise inverse of the word, you can skip words that are full
        // quickly (where the value is 0 or 'false' since all bits are '1', or 'in use').  Any value greater
        // than 0 indicates there is a free slot. Then, when counting the trailing 0's, you can test very quickly
        // where the first free slot is. This operation prevents looping through every bit of filled flags, and
        // will instead operate only on the first word with free slots.
        if (inv)
        {
            int bit = __builtin_ctz(inv);
            bm->w[i] |= ((uint32_t)1 << bit);
            int idx = i * POOL_BITS_PER_WORD + bit;
            return (idx < bm->cap) ? idx : UNDEFINED;
        }
    }

    return UNDEFINED;
}


#define POOL_ENTRY(name, capacity) \
POOL_DEFINE_TYPE(name, capacity);
#include POOLS_DEF_FILE
#undef POOL_ENTRY
