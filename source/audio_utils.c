#include "audio_utils.h"

#include <maxmod.h>

void play_sfx(mm_word id, mm_word rate, mm_byte volume)
{
    mm_sound_effect sfx = {
        {id},
        rate,
        0,
        volume,
        SFX_DEFAULT_PAN,
    };
    mmEffectEx(&sfx);
}
