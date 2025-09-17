#include "pool.h"

void pool_clear_idx(PoolBitmap *bm, int idx) {
    u32 i = idx / POOL_BITS_PER_WORD; // word offset
    u32 b = idx % POOL_BITS_PER_WORD; // bit offset
    bm->w[i] &= ~((POOL_WORD_T)1 << b);
}

int pool_get_free_idx(PoolBitmap *bm) {
    for (u32 i = 0; i < bm->nwords; i++) {
        POOL_WORD_T inv = ~bm->w[i];

        // guard so we don't cal `ctz` with 0, since __builtin_ctz(0) is undefined
        // https://gcc.gnu.org/onlinedocs/gcc/Bit-Operation-Builtins.html#index-_005f_005fbuiltin_005fctz
        //
        // By using the bitwise inverse of the word, you can skip words that are full
        // quickly (where the value is 0 or 'false' since all bits are '1', or 'in use').  Any value greater
        // than 0 indicates there is a free slot. Then, when counting the trailing 0's, you can test very quickly
        // where the first free slot is. This operation prevents looping through every bit of filled flags, and
        // will instead operate only on the first word with free slots.
        if (inv) {
            int bit = __builtin_ctz(inv);
            bm->w[i] |= ((POOL_WORD_T)1 << bit);
            int idx = i * POOL_BITS_PER_WORD + bit;
            return idx;
        }
    }

    return -1;
}
