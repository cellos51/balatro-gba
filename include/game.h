#ifndef GAME_H
#define GAME_H

#define MAX_HAND_SIZE 16
#define MAX_DECK_SIZE 52
#define MAX_SELECTION_SIZE 5
#define FRAMES(x) (((x) + game_speed - 1) / game_speed)

// TODO: Turn into enum?
#define BG_ID_CARD_SELECTING 1
#define BG_ID_CARD_PLAYING 2
#define BG_ID_ROUND_END 3
#define BG_ID_SHOP 4

enum GameState
{
    GAME_PLAYING,
    GAME_ROUND_END,
    GAME_SHOP,
    GAME_BLIND_SELECT,
    GAME_LOSE,
};

enum HandState
{
    HAND_DRAW,
    HAND_SELECT,
    HAND_SHUFFLING, // This is actually a misnomer because it's used for the deck, but it mechanically makes sense to be a state of the hand
    HAND_DISCARD,
    HAND_PLAY,
    HAND_PLAYING
};

enum PlayState
{
    PLAY_PLAYING,
    PLAY_SCORING,
    PLAY_ENDING,
    PLAY_ENDED
};

// Hand types
enum HandType
{
    NONE,
    HIGH_CARD,
    PAIR,
    TWO_PAIR,
    THREE_OF_A_KIND,
    FOUR_OF_A_KIND,
    STRAIGHT,
    FLUSH,
    FULL_HOUSE,
    STRAIGHT_FLUSH,
    ROYAL_FLUSH,
    FIVE_OF_A_KIND,
    FLUSH_HOUSE,
    FLUSH_FIVE
};

// Game functions
void game_init();
void game_update();

#endif // GAME_H