#include "splash_screen.h"

#include "font.h"
#include "game.h"
#include "graphic_utils.h"
#include "maxmod.h"
#include "soundbank.h"

#include <tonc.h>

static const Rect COUNTDOWN_TIMER_RECT = {208, 144, 240, 152};
static uint timer = 0;

void splash_screen_on_init()
{
    timer = 0;

    tte_printf("#{P:88,8; cx:0xF000}ВНИМАНИЕ");
    tte_printf(
        "#{P:8,24; cx:0xF000}Этот проект НЕ одобрен \n и не связан с Playstack \n и LocalThunk.\n\n "
        "Если вы заплатили за это, \n вас обманули. Вы должны \n потребовать возврат \n "
        "СРАЗУ ЖЕ. \n\n Единственный официальный \n источник игры: \n\n "
        "'github.com/\n  GBALATRO/balatro-gba'"
    );
    tte_printf("#{P:8,144; cx:0xF000}(Нажмите любую кнопку)");
}

void splash_screen_on_update()
{
    timer++;

    if (timer < SPLASH_DURATION_FRAMES)
    {
        tte_erase_rect_wrapper(COUNTDOWN_TIMER_RECT);
        tte_printf(
            "#{P:%d,%d; cx:0xF000}%d",
            COUNTDOWN_TIMER_RECT.left,
            COUNTDOWN_TIMER_RECT.top,
            1 + (SPLASH_DURATION_FRAMES - timer) / SPLASH_FPS
        );

        if (!key_hit(KEY_ANY))
        {
            return;
        }
    }

    game_change_state(GAME_STATE_MAIN_MENU);
    tte_erase_screen();
}

void splash_screen_on_exit()
{
    mmStart(MOD_MAIN_THEME, MM_PLAY_LOOP);
}
