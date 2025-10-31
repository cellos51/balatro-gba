// This should only be directly accessed by joker_effects.c, but was put in a
// separate file because the effects are already big enough as they are now

#include "game.h"
#include "joker.h"
#include "util.h"
#include "hand_analysis.h"
#include <stdlib.h>


// ON JOKER CREATED

// For Jokers that don't need to do anything when created
void on_joker_created_noop(Joker *joker) {}

void hanging_chad_on_joker_created(Joker *joker)
{
    joker->data = 2; // retriggers left, reset to 2 at round end
}

void dusk_on_joker_created(Joker *joker)
{
    joker->data = UNDEFINED; // previously retriggered card index
}

void hack_on_joker_created(Joker *joker)
{
    joker->data = UNDEFINED; // previously retriggered card index
}

void photograph_on_joker_created(Joker *joker)
{
    joker->data = UNDEFINED; // First scoring face card index
}

void sock_and_buskin_on_joker_created(Joker *joker)
{
    joker->data = UNDEFINED; // previously retriggered face card index
}

void seltzer_on_joker_created(Joker *joker)
{
    joker->halves.data0 = UNDEFINED; // previously retriggered card index
    joker->halves.data1 = 10; // remaining retriggered hands
}
