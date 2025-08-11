#ifndef GAME_H
#define GAME_H

#define MAX_HAND_SIZE 16
#define MAX_DECK_SIZE 52
#define MAX_JOKERS_HELD_SIZE 5 // This doesn't account for negatives right now.
#define MAX_SHOP_JOKERS 2 // TODO: Make this dynamic and allow for other items besides jokers
#define MAX_SELECTION_SIZE 5
#define FRAMES(x) (((x) + game_speed - 1) / game_speed)

// TODO: Turn into enum?
#define BG_ID_CARD_SELECTING 1
#define BG_ID_CARD_PLAYING 2
#define BG_ID_ROUND_END 3
#define BG_ID_SHOP 4
#define BG_ID_BLIND_SELECT 5

// Input bindings
#define SELECT_CARD KEY_A
#define DESELECT_CARDS KEY_B
#define PEEK_DECK KEY_L // Not implemented
#define SORT_HAND KEY_R
#define PAUSE_GAME KEY_START // Not implemented

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

// Utility functions for other files
typedef struct CardObject CardObject; // forward declaration, actually declared in card.h
typedef struct JokerObject JokerObject;
CardObject**    get_hand_array(void);
int             get_hand_top(void);
CardObject**    get_played_array(void);
int             get_played_top(void);
JokerObject**   get_jokers(void);
int             get_jokers_top(void);

int get_deck_top(void);
int get_num_discards_remaining(void);
int get_money(void);

// joker specific functions
bool is_shortcut_joker_active(void);
int get_straight_and_flush_size(void);

#endif // GAME_H