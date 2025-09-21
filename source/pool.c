#include "pool.h"

void pool_bm_clear_idx(PoolBitmap *bm, int idx)
{
    // Divide by 32 to get the word index
    u32 i = idx >> 5;
    // Get last 5-bits, same as a modulo (% 32) operation on positive numbers
    u32 b = idx & 31;
    bm->w[i] &= ~((u32)1 << b);
}

int pool_bm_get_free_idx(PoolBitmap *bm)
{
    for (u32 i = 0; i < bm->nwords; i++)
    {
        u32 inv = ~bm->w[i];

        // guard so we don't cal `ctz` with 0, since __builtin_ctz(0) is undefined
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
            bm->w[i] |= ((u32)1 << bit);
            int idx = i * POOL_BITS_PER_WORD + bit;
            return idx;
        }
    }

    return -1;
}


#define POOL_ENTRY(name, capacity) \
POOL_DEFINE_TYPE(name, capacity);
#include "sprite.h"
#include "pools.def"
#undef POOL_ENTRY
