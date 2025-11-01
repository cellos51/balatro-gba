#include "splash_screen.h"

#include <tonc.h>

#include "graphic_utils.h"
#include "game.h"
#include "def_state_info_table.h" // Required for GAME_STATE_DEBUG_SCREEN
#include "maxmod.h"
#include "soundbank.h"

// Button combination to activate the debug screen.
#define DEBUG_SCREEN_ACTIVATION_COMBO (KEY_L | KEY_R | KEY_A | KEY_B)

static const Rect COUNTDOWN_TIMER_RECT = {208, 144, 240, 152};
static uint timer = 0;

void splash_screen_on_init()
{
    timer = 0;

    tte_printf("#{P:72,8; cx:0xF000}DISCLAIMER");
    tte_printf("#{P:8,24; cx:0xF000}This project is NOT endorsed \n by or affiliated with \n Playstack or LocalThunk.\n\n If you have paid for this, \n you have been scammed \n and should request a refund \n IMMEDIATELY. \n\n The only official place \n to obtain this is from: \n\n 'github.com/\n  cellos51/balatro-gba'");
    tte_printf("#{P:8,144; cx:0xF000}(Press any key to skip)");
}

void splash_screen_on_update()
{
    timer++;

    // Check for debug screen activation (L+R+A+B held down).
    // This check takes priority over the normal splash screen flow.
    if ((key_curr_state() & DEBUG_SCREEN_ACTIVATION_COMBO) == DEBUG_SCREEN_ACTIVATION_COMBO)
    {
        game_change_state(GAME_STATE_DEBUG_SCREEN);
        tte_erase_screen();
        return;
    }

    // Check for normal transition to main menu (timer expired or any key pressed).
    if (timer >= SPLASH_DURATION_FRAMES || key_hit(KEY_ANY))
    {
        game_change_state(GAME_STATE_MAIN_MENU);
        tte_erase_screen();
        return;
    }

    // If no transition occurred, update the countdown timer on screen.
    tte_erase_rect_wrapper(COUNTDOWN_TIMER_RECT);
    tte_printf("#{P:%d,%d; cx:0xF000}%d", COUNTDOWN_TIMER_RECT.left, COUNTDOWN_TIMER_RECT.top, 1 + (SPLASH_DURATION_FRAMES - timer) / SPLASH_FPS);
}

void splash_screen_on_exit()
{
    mmStart(MOD_MAIN_THEME, MM_PLAY_LOOP);
}