#ifndef SPLASH_SCREEN_H
#define SPLASH_SCREEN_H

#include <tonc.h>

#define SPLASH_FPS              60
#define SPLASH_DURATION_SECONDS 10
#define SPLASH_DURATION_FRAMES  (SPLASH_FPS * SPLASH_DURATION_SECONDS)

void splash_screen_on_init();
void splash_screen_on_update();
void splash_screen_on_exit();

#endif // SPLASH_SCREEN_H
