#ifndef AUDIO_UTILS_H
#define AUDIO_UTILS_H

#include <mm_types.h>

#define MM_FULL_VOLUME      255
#define MM_PAN_CENTER       128
#define MM_BASE_PITCH_RATE  1024

#define SFX_DEFAULT_VOLUME  MM_FULL_VOLUME
#define SFX_DEFAULT_PAN     MM_PAN_CENTER
#define SFX_DEFAULT_HANDLE  0

void play_sfx(mm_word id, mm_word rate);

#endif