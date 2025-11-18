/**
 * @file audio_utils.h
 *
 * @brief Utilities for using maxmod to play sound effects
 */
#ifndef AUDIO_UTILS_H
#define AUDIO_UTILS_H

#include <mm_types.h>

/**
 * @def MM_FULL_VOLUME
 * @brief The maxium volume for maxmod
 *
 * Represented as the max value of a unsigned char.
 */
#define MM_FULL_VOLUME 255

/**
 * @def MM_PAN_CENTER
 * @brief The center pan of stereo audio for maxmode
 *
 * Represented as half the max value of an unsigned char.
 */
#define MM_PAN_CENTER 128

/**
 * @def MM_BASE_PITCH_RATE
 * @brief The default pitch rate for the sound effect played.
 *
 * Increasing the rate increases the pitch, while decreasing lowers the pitch.
 */
#define MM_BASE_PITCH_RATE 1024

/**
 * @def SFX_DEFAULT_VOLUME
 * @brief Default volume for sound effects (loudest by default)
 */
#define SFX_DEFAULT_VOLUME MM_FULL_VOLUME

/**
 * @def SFX_DEFAULT_PAN
 * @brief The default stereo pan, will always be center pan.
 */
#define SFX_DEFAULT_PAN MM_PAN_CENTER

/**
 * @brief Play a sound effect
 *
 * @param id the sound id to play, name an all caps derivation of the
 *             associated in the `audio/` directory. E.g. `audio/card_draw.wav`
 *             becomes SFX_CARD_DRAW
 * @param rate the pitch rate, the default value is @ref MM_BASE_PTCH_RATE
 */
void play_sfx(mm_word id, mm_word rate);

#endif
