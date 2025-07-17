#include "game.h"

#include <maxmod.h>
#include <tonc.h>
#include <string.h>
#include <stdlib.h>

#include "util.h"
#include "sprite.h"
#include "card.h"
#include "blind.h"
#include "graphic_utils.h"

#include "background_gfx.h"
#include "background_round_end_gfx.h"
#include "background_shop_gfx.h"
#include "background_blind_select_gfx.h"

#include "soundbank.h"

static int timer = 0; // This might already exist in libtonc but idk so i'm just making my own
static int game_speed = 1;
static int background = 0;

static enum GameState game_state = GAME_BLIND_SELECT; // The current game state, this is used to determine what the game is doing at any given time
static enum HandState hand_state = HAND_DRAW;
static enum PlayState play_state = PLAY_PLAYING;

static enum HandType hand_type = NONE;

static Sprite *playing_blind_token = NULL; // The sprite that displays the blind when in "GAME_PLAYING/GAME_ROUND_END" state
static Sprite *round_end_blind_token = NULL; // The sprite that displays the blind when in "GAME_ROUND_END" state

static Sprite *blind_select_tokens[MAX_BLINDS] = {NULL}; // The sprites that display the blinds when in "GAME_BLIND_SELECT" state

static int current_blind = SMALL_BLIND;
static enum BlindState blinds[MAX_BLINDS] = {BLIND_CURRENT, BLIND_UPCOMING, BLIND_UPCOMING}; // The current state of the blinds, this is used to determine what the game is doing at any given time

static int hands = 4;
static int discards = 4;

static int ante = 1;
static int money = 4;
static int score = 0;
static int temp_score = 0; // This is the score that shows in the same spot as the hand type.
static FIXED lerped_score = 0;
static FIXED lerped_temp_score = 0;

static int chips = 0;
static int mult = 0;

static int hand_size = 8; // Default hand size is 8
static int cards_drawn = 0;
static int hand_selections = 0;

static int card_focused = 0;

static bool sort_by_suit = false;

// Stacks
static CardObject *played[MAX_SELECTION_SIZE] = {NULL};
static int played_top = -1;

static CardObject *hand[MAX_HAND_SIZE] = {NULL};
static int hand_top = -1;

static Card *deck[MAX_DECK_SIZE] = {NULL};
static int deck_top = -1;

static Card *discard_pile[MAX_DECK_SIZE] = {NULL};
static int discard_top = -1;

// Played stack
static inline void played_push(CardObject *card_object)
{
    if (played_top >= MAX_SELECTION_SIZE - 1) return;
    played[++played_top] = card_object;
}

static inline CardObject *played_pop()
{
    if (played_top < 0) return NULL;
    return played[played_top--];
}

// Deck stack
static inline void deck_push(Card *card)
{
    if (deck_top >= MAX_DECK_SIZE - 1) return;
    deck[++deck_top] = card;
}

static inline Card *deck_pop()
{
    if (deck_top < 0) return NULL;
    return deck[deck_top--];
}

// Discard stack
static inline void discard_push(Card *card)
{
    if (discard_top >= MAX_DECK_SIZE - 1) return;
    discard_pile[++discard_top] = card;
}

static inline Card *discard_pop()
{
    if (discard_top < 0) return NULL;
    return discard_pile[discard_top--];
}


// Consts

// Rects                                       left     top     right   bottom
// Screenblock rects
static const Rect ROUND_END_MENU_RECT       = {9,       7,      24,     21 }; 

// TODO: Currently unused, remove?
//static const Rect POP_MENU_ANIM_RECT_SRC    = {9,       19,     24,     31 };
//static const Rect POP_MENU_ANIM_RECT_DEST   = {9,       7,      24,     19 };

static const Rect POP_MENU_ANIM_RECT        = {9,       7,      24,     31 };
// The rect for popping menu animations (round end, shop, blinds) 
// - extends beyond the visible screen to the end of the screenblock
// It includes both the target and source position rects. 
// This is because when popping, the target position is blank so we just animate 
// the whole rect so we don't have to track its position

static const Rect SINGLE_BLIND_SELECT_RECT  = {9,       7,      13,     31 };
static const Rect BLIND_SELECT_BTN_PREANIM_RECT = {10,  20,     12,     20 };
static const Rect BLIND_SKIP_BTN_GRAY_RECT  = {0,       24,     4,      27 };
static const Rect BLIND_SKIP_BTN_PREANIM_DEST_RECT = {9,29,     19,     31 };
// preanim - pre-animation rects for before the pop-up animation

static const Rect HAND_BG_RECT_SELECTING    = {9,       11,     24,     15 };
// TODO: Currently unused, remove?
//static const Rect HAND_BG_RECT_PLAYING      = {9,       14,     24,     18 };

static const Rect TOP_LEFT_ITEM_SRC_RECT    = {0,       20,     8,      25 };
static const Rect TOP_LEFT_PANEL_RECT       = {0,       0,      8,      5  };
static const Rect TOP_LEFT_PANEL_ANIM_RECT  = {0,       0,      8,      4  };
/* Contains the shop icon/current blind etc. 
 * The difference between TOP_LEFT_PANEL_ANIM_RECT and TOP_LEFT_PANEL_RECT 
 * is due to an overlap between the bottom of the top left panel
 * and the top of the score panel in the tiles connecting them.
 * TOP_LEFT_PANEL_ANIM_RECT should be used for animations, 
 * TOP_LEFT_PANEL_RECT for copies etc. but mind the overlap
 */
static const Rect TOP_LEFT_BLIND_TITLE_RECT = {0,       1,      8,      1 };
static const Rect BIG_BLIND_TITLE_SRC_RECT  = {0,       26,     8,      26 };
static const Rect BOSS_BLIND_TITLE_SRC_RECT = {0,       27,     8,      27 };

// Rects for TTE (in pixels)
static const Rect HAND_SIZE_RECT            = {128,     128,    152,    160 }; // Seems to include both SELECT and PLAYING
static const Rect HAND_SIZE_RECT_SELECT     = {128,     128,    152,    136 };
static const Rect HAND_SIZE_RECT_PLAYING    = {128,     152,    152,    160 };
static const Rect HAND_TYPE_RECT            = {8,       64,     64,     72  };
// Score displayed in the same place as the hand type
static const Rect TEMP_SCORE_RECT           = {8,       64,     64,     72  }; 

static const Rect PLAYED_CARDS_SCORES_RECT  = {72,      48,     240,    56  };
static const Rect BLIND_TOKEN_TEXT_RECT     = {80,      72,     200,    160 };
static const Rect MONEY_TEXT_RECT           = {8,       120,    64,     120 };
static const Rect CHIPS_TEXT_RECT           = {8,       80,     32,     88  };
static const Rect MULT_TEXT_RECT            = {40,      80,     64,     88  };
static const Rect BLIND_REWARD_RECT         = {40,      32,     64,     40  };

// Rects with UNDEFINED are only used in tte_printf, they need to be fully defined
// to be used with tte_erase_rect_wrapper()
static const Rect HANDS_TEXT_RECT           = {16,      104,    UNDEFINED, UNDEFINED };
static const Rect DISCARDS_TEXT_RECT        = {48,      104,    UNDEFINED, UNDEFINED };
static const Rect BLIND_REQ_TEXT_RECT       = {40,      24,     UNDEFINED, UNDEFINED };
static const Rect DECK_SIZE_RECT            = {200,     152,    UNDEFINED, UNDEFINED };
static const Rect ROUND_TEXT_RECT           = {48,      144,    UNDEFINED, UNDEFINED };
static const Rect ANTE_TEXT_RECT            = {8,       144,    UNDEFINED, UNDEFINED };
static const Rect ROUND_END_BLIND_REQ_RECT  = {112,     96,     UNDEFINED, UNDEFINED };
static const Rect ROUND_END_BLIND_REWARD_RECT = { 168,  96,     UNDEFINED, UNDEFINED };
static const Rect ROUND_END_NUM_HANDS_RECT  = {88,      116,    UNDEFINED, UNDEFINED };
static const Rect HAND_REWARD_RECT          = {168,     UNDEFINED, UNDEFINED, UNDEFINED };
static const Rect CASHOUT_RECT              = {88,      72,     UNDEFINED, UNDEFINED };

//TODO: Properly define and use
#define MENU_POP_OUT_ANIM_FRAMES 20

// General functions
void sort_hand_by_suit()
{
    for (int a = 0; a < hand_top; a++)
    {
        for (int b = a + 1; b <= hand_top; b++)
        {
            if (hand[a] == NULL || (hand[b] != NULL && (hand[a]->card->suit > hand[b]->card->suit || (hand[a]->card->suit == hand[b]->card->suit && hand[a]->card->rank > hand[b]->card->rank))))
            {
                CardObject* temp = hand[a];
                hand[a] = hand[b];
                hand[b] = temp;
            }
        }
    }
}

void sort_hand_by_rank()
{
    for (int a = 0; a < hand_top; a++)
    {
        for (int b = a + 1; b <= hand_top; b++)
        {
            if (hand[a] == NULL || (hand[b] != NULL && hand[a]->card->rank > hand[b]->card->rank))
            {
                CardObject* temp = hand[a];
                hand[a] = hand[b];
                hand[b] = temp;
            }
        }
    }
}

void sort_cards()
{
    if (sort_by_suit)
    {
        sort_hand_by_suit();
    }
    else
    {
        sort_hand_by_rank();
    }

    // Update the sprites in the hand by destroying them and creating new ones in the correct order
    // (This is feels like a diabolical solution but like literally how else would you do this)
    for (int i = 0; i <= hand_top; i++)
    {
        if (hand[i] != NULL)
        {
            sprite_destroy(&hand[i]->sprite);
        }
    }

    for (int i = 0; i <= hand_top; i++)
    {
        if (hand[i] != NULL)
        {
            //hand[i]->sprite = sprite_new(ATTR0_SQUARE | ATTR0_4BPP | ATTR0_AFF, ATTR1_SIZE_32, card_sprite_lut[hand[i]->card->suit][hand[i]->card->rank], 0, i);
            card_object_set_sprite(hand[i], i); // Set the sprite for the card object
            sprite_position(hand[i]->sprite, fx2int(hand[i]->x), fx2int(hand[i]->y));
        }
    }
}

enum HandType hand_get_type()
{
    enum HandType res_hand_type = NONE;

    // Idk if this is how Balatro does it but this is how I'm doing it
    if (hand_selections == 0 || hand_state == HAND_DISCARD)
    {
        res_hand_type = NONE;
        return res_hand_type;
    }

    res_hand_type = HIGH_CARD;

    u8 suits[NUM_SUITS] = {0};
    u8 ranks[NUM_RANKS] = {0};

    for (int i = 0; i <= hand_top; i++)
    {
        if (hand[i] != NULL && hand[i]->selected)
        {
            suits[hand[i]->card->suit]++;
            ranks[hand[i]->card->rank]++;
        }
    }

    // Check for flush
    for (int i = 0; i < NUM_SUITS; i++)
    {
        if (suits[i] >= MAX_SELECTION_SIZE) // if i add jokers just MAX_SELECTION_SIZE - 1 for four fingers
        {
            res_hand_type = FLUSH;
            break;
        }
    }

    // Check for straight
    for (int i = 0; i < NUM_RANKS - 4; i++)
    {
        if (ranks[i] && ranks[i + 1] && ranks[i + 2] && ranks[i + 3] && ranks[i + 4])
        {
            if (res_hand_type == FLUSH)
            {
                res_hand_type = STRAIGHT_FLUSH;
            }
            else
            {
                res_hand_type = STRAIGHT;
            }
            break;
        }
    }

    // Check for ace low straight
    if (ranks[ACE] && ranks[TWO] && ranks[THREE] && ranks[FOUR] && ranks[FIVE])
    {
        res_hand_type = STRAIGHT;
    }

    // Check for royal flush
    if (res_hand_type == STRAIGHT_FLUSH && ranks[TEN] && ranks[JACK] && ranks[QUEEN] && ranks[KING] && ranks[ACE])
    {
        res_hand_type = ROYAL_FLUSH;
        return res_hand_type;
    }

    // Check for straight flush
    if (res_hand_type == STRAIGHT_FLUSH)
    {
        res_hand_type = STRAIGHT_FLUSH;
        return res_hand_type;
    }

    for (int i = 0; i < NUM_RANKS; i++)
    {
        if (ranks[i] >= 5)
        {
            if (res_hand_type == FLUSH)
            {
                res_hand_type = FLUSH_FIVE;
                return res_hand_type;
            }
            else
            {
                res_hand_type = FIVE_OF_A_KIND;
                return res_hand_type;
            }
        }
        else if (ranks[i] == 4)
        {
            res_hand_type = FOUR_OF_A_KIND;
            return res_hand_type;
        }
        else if (ranks[i] == 3)
        {   
            if (res_hand_type == PAIR)
            {
                res_hand_type = FULL_HOUSE;
                return res_hand_type;
            }
            else
            {
                res_hand_type = THREE_OF_A_KIND;
            }
        }
        else if (ranks[i] == 2)
        {
            if (res_hand_type == THREE_OF_A_KIND)
            {
                res_hand_type = FULL_HOUSE;
                return res_hand_type;
            }
            else if (res_hand_type == PAIR)
            {
                res_hand_type = TWO_PAIR;
                return res_hand_type;
            }
            else
            {
                res_hand_type = PAIR;
            }
        }
    }

    return res_hand_type;
}

/* Copies the appropriate item into the top left panel (blind/shop icon)
 * from where it was put outside the screenview
 */
void bg_copy_current_item_to_top_left_panel()
{
    main_bg_se_copy_rect(TOP_LEFT_PANEL_RECT, TOP_LEFT_ITEM_SRC_RECT);
}

void change_background(int id)
{
    if (background == id)
    {
        return;
    }
    else if (id == BG_ID_CARD_SELECTING)
    {
        REG_DISPCNT |= DCNT_WIN0; // Enable window 0 to make hand background transparent
        // Load the tiles and palette
        // Background
        memcpy(pal_bg_mem, background_gfxPal, 64); // This '64" isn't a specific number, I'm just using it to prevent the text colors from being overridden
        GRIT_CPY(&tile8_mem[MAIN_BG_CBB], background_gfxTiles); // Deadass i have no clue how any of these memory things work but I just messed with them until stuff worked
        GRIT_CPY(&se_mem[MAIN_BG_SBB], background_gfxMap);

        tte_erase_rect_wrapper(HAND_SIZE_RECT_PLAYING);

        /* TODO: The blind panel should come in with an animation when it's selected, 
         * probably should happen elsewhere
         */ 
        bg_copy_current_item_to_top_left_panel();
        
        // TODO: These cases are still untested
        if (current_blind == BIG_BLIND) // Change text and palette depending on blind type
        {
            main_bg_se_copy_rect(TOP_LEFT_BLIND_TITLE_RECT, BIG_BLIND_TITLE_SRC_RECT);
        }
        else if (current_blind == BOSS_BLIND)
        {
            main_bg_se_copy_rect(TOP_LEFT_BLIND_TITLE_RECT, BOSS_BLIND_TITLE_SRC_RECT);
        }

        // This would change the palette of the background to match the blind, but the backgroun doesn't use the blind token's exact colors so a different approach is required
        // memset16(&pal_bg_mem[17], blind_get_color(current_blind, BLIND_HIGHLIGHT_COLOR_INDEX), 1);
        // memset16(&pal_bg_mem[5], blind_get_color(current_blind, BLIND_MAIN_COLOR_INDEX), 1);
        // memset16(&pal_bg_mem[13], blind_get_color(current_blind, BLIND_SHADOW_COLOR_INDEX), 1);
    }
    else if (id == BG_ID_CARD_PLAYING)
    {
        // REG_DISPCNT |= DCNT_WIN0;
        // memcpy(pal_bg_mem, background_gfxPal, 64);
        // GRIT_CPY(&tile8_mem[MAIN_BG_CBB], background_gfxTiles);
        // GRIT_CPY(&se_mem[MAIN_BG_SBB], background_gfxMap);

        if (background != BG_ID_CARD_SELECTING)
        {
            change_background(BG_ID_CARD_SELECTING);
            background = BG_ID_CARD_PLAYING;
        }

        Rect curr_hand_bg_rect = HAND_BG_RECT_SELECTING;
        for (int i = 0; i <= 2; i++)
        {
            main_bg_se_move_rect_1_tile_vert(curr_hand_bg_rect, SE_DOWN);
            curr_hand_bg_rect.top++;
            curr_hand_bg_rect.bottom++;
        }        

        tte_erase_rect_wrapper(HAND_SIZE_RECT_SELECT);
    }
    else if (id == BG_ID_ROUND_END)
    {
        REG_DISPCNT &= ~DCNT_WIN0; // Disable window 0 so it doesn't make the cashout menu transparent

        memcpy(pal_bg_mem, background_round_end_gfxPal, 64);
        memcpy(&tile_mem[MAIN_BG_CBB], background_round_end_gfxTiles, background_round_end_gfxTilesLen);
        memcpy(&se_mem[MAIN_BG_SBB], background_round_end_gfxMap, background_round_end_gfxMapLen);

        // 1024 0x0400 is when sprites are flipped horizontally, 2048 0x0800 is when they are flipped vertically, 3072 0x0C00 is when they are flipped both horizontally and vertically
        // Not sure why this comment is here but note there are bit macros SE_HFLIP and SE_VFLIP for those.

        // Incoming hack! Clear the round end menu so that we can slowly display it with an animation later. The reason this isn't optimal is because the full background is already loaded into the vram at this point.
        // I'm just doing it this way because it's easier than doing some weird shit with Grit in order to get a proper tilemap. I'm not the biggest fan of Grit.
        // TODO: Remove this if game_round_end() is fixed to use main_bg_se_copy/move_rect_1_tile_vert() for the pop menu
        main_bg_se_clear_rect(ROUND_END_MENU_RECT);
        
        //tte_erase_rect(0, 0, 64, 48); // Clear top left corner where the blind stats are displayed
        tte_erase_rect_wrapper(HAND_SIZE_RECT);

    }
    else if (id == BG_ID_SHOP)
    {
        REG_DISPCNT &= ~DCNT_WIN0;

        memcpy(pal_bg_mem, background_shop_gfxPal, 64);
        GRIT_CPY(&tile_mem[MAIN_BG_CBB], background_shop_gfxTiles);
        GRIT_CPY(&se_mem[MAIN_BG_SBB], background_shop_gfxMap);

        // Set the outline colors for the shop background
        memset16(&pal_bg_mem[27], 0x213D, 1);
        memset16(&pal_bg_mem[6], 0x10B4, 1);
        
        memset16(&pal_bg_mem[14], 0x32BE, 1); // Reset the shop lights to correct colors
        memset16(&pal_bg_mem[17], 0x4B5F, 1);
        memset16(&pal_bg_mem[22], 0x5F9F, 1);
        memset16(&pal_bg_mem[8], 0xFFFF, 1);

        memcpy16(&pal_bg_mem[7], &pal_bg_mem[3], 1); // Disable the button highlight colors
        memcpy16(&pal_bg_mem[5], &pal_bg_mem[16], 1);
    }
    else if (id == BG_ID_BLIND_SELECT)
    {
        obj_unhide(blind_select_tokens[SMALL_BLIND]->obj, 0);
        obj_unhide(blind_select_tokens[BIG_BLIND]->obj, 0);
        obj_unhide(blind_select_tokens[BOSS_BLIND]->obj, 0);

        const int default_y = 89 + (TILE_SIZE * 12); // Default y position for the blind select tokens. 8 is the size of a tile and 12 is the amound of tiles the background is shifted down by
        sprite_position(blind_select_tokens[SMALL_BLIND], 80, default_y);
        sprite_position(blind_select_tokens[BIG_BLIND], 120, default_y);
        sprite_position(blind_select_tokens[BOSS_BLIND], 160, default_y);

        REG_DISPCNT &= ~DCNT_WIN0;

        memcpy(pal_bg_mem, background_blind_select_gfxPal, 64);
        GRIT_CPY(&tile_mem[MAIN_BG_CBB], background_blind_select_gfxTiles);
        GRIT_CPY(&se_mem[MAIN_BG_SBB], background_blind_select_gfxMap);

        // Disable the button highlight colors
        // Select button PID is 15 and the outline is 18
        memcpy16(&pal_bg_mem[18], &pal_bg_mem[15], 1);
        // Skip button PID is 10 and the outline is 5
        memcpy16(&pal_bg_mem[10 ], &pal_bg_mem[5], 1);

        for (int i = 0; i < MAX_BLINDS; i++)
        {
            Rect curr_blind_rect = SINGLE_BLIND_SELECT_RECT;

            // There's no gap between them
            curr_blind_rect.left += i * rect_width(&SINGLE_BLIND_SELECT_RECT);
            curr_blind_rect.right += i * rect_width(&SINGLE_BLIND_SELECT_RECT);

            if (blinds[i] != BLIND_CURRENT && (i == SMALL_BLIND || i == BIG_BLIND)) // Make the skip button gray
            {
                Rect skip_blind_btn_dest_rect = BLIND_SKIP_BTN_PREANIM_DEST_RECT;
                skip_blind_btn_dest_rect.left = curr_blind_rect.left;
                skip_blind_btn_dest_rect.right = curr_blind_rect.right; 
                Rect skip_blind_btn_src_rect = BLIND_SKIP_BTN_GRAY_RECT;
                skip_blind_btn_src_rect.top += i * rect_height(&BLIND_SKIP_BTN_GRAY_RECT);
                skip_blind_btn_src_rect.bottom += i * rect_height(&BLIND_SKIP_BTN_GRAY_RECT);

                main_bg_se_copy_rect(skip_blind_btn_dest_rect, skip_blind_btn_src_rect);
            }

            if (blinds[i] == BLIND_CURRENT) // Raise the blind panel up a bit
            {
                int x_from = 0;
                int y_from = 27;

                main_bg_se_copy_rect_1_tile_vert(curr_blind_rect, SE_UP);

                int x_to = curr_blind_rect.left;
                int y_to = 31;

                if (i == BIG_BLIND)
                {
                    y_from = 31;
                }
                else if (i == BOSS_BLIND)
                {
                    x_from = x_to;
                    y_from = 30;
                }

                // TODO: What is this doing? More importantly, why?
                memcpy16(&se_mem[MAIN_BG_SBB][x_to + 32 * y_to], &se_mem[MAIN_BG_SBB][x_from + 32 * y_from], 5);

                sprite_position(blind_select_tokens[i], blind_select_tokens[i]->pos.x, blind_select_tokens[i]->pos.y - TILE_SIZE); // Move token up by a tile
            }
            else if (blinds[i] == BLIND_UPCOMING) // Change the select icon to "NEXT" 
            {
                int x_from = 0;
                int y_from = 20;

                int x_to = 10 + (i * rect_width(&SINGLE_BLIND_SELECT_RECT));
                int y_to = 20;

                memcpy16(&se_mem[MAIN_BG_SBB][x_to + 32 * y_to], &se_mem[MAIN_BG_SBB][x_from + 32 * y_from], 3);
            }
            else if (blinds[i] == BLIND_SKIPPED) // Change the select icon to "SKIP"
            {
                int x_from = 3;
                int y_from = 20;

                int x_to = 10 + (i * 5);
                int y_to = 20;

                memcpy16(&se_mem[MAIN_BG_SBB][x_to + 32 * y_to], &se_mem[MAIN_BG_SBB][x_from + 32 * y_from], 3);
            }
            else if (blinds[i] == BLIND_DEFEATED) // Change the select icon to "DEFEATED"
            {
                int x_from = 6;
                int y_from = 20;

                int x_to = 10 + (i * 5);
                int y_to = 20;

                memcpy16(&se_mem[MAIN_BG_SBB][x_to + 32 * y_to], &se_mem[MAIN_BG_SBB][x_from + 32 * y_from], 3);
            }
        }
    }
    else
    {
        return; // Invalid background ID
    }

    background = id;
}

void set_temp_score(int value)
{
    int x_offset = 40 - get_digits_even(value) * 8;
    tte_erase_rect_wrapper(TEMP_SCORE_RECT);
    tte_printf("#{P:%d,%d; cx:0xF000}%d", x_offset, TEMP_SCORE_RECT.top, value);
}

void set_score(int value)
{
    int x_offset = 32;
    tte_printf("#{P:%d,48; cx:0xF000}%d", x_offset, value);
}

void set_money(int value)
{
    int x_offset = 32 - get_digits_odd(value) * 8;
    tte_erase_rect_wrapper(MONEY_TEXT_RECT);
    tte_printf("#{P:%d,%d; cx:0xC000}$%d", x_offset, MONEY_TEXT_RECT.top, value);
}

void set_chips(int value)
{
    int x_offset = 32 - get_digits(value) * 8; // Adjust the x offset based on the number of digits in chips
    tte_erase_rect_wrapper(CHIPS_TEXT_RECT);
    tte_printf("#{P:%d,%d; cx:0xF000;}%d", x_offset, CHIPS_TEXT_RECT.top, value);
}

void set_mult(int value)
{
    tte_erase_rect_wrapper(MULT_TEXT_RECT);
    tte_printf("#{P:%d,%d; cx:0xF000;}%d", MULT_TEXT_RECT.left, MULT_TEXT_RECT.top, value); // Mult
}

static void print_hand_type(const char* hand_type_str)
{
    if (hand_type_str == NULL)
        return; // NULL-checking paranoia
    tte_printf("#{P:%d,%d;}%s", HAND_TYPE_RECT.left, HAND_TYPE_RECT.top, hand_type_str);
}

void set_hand()
{
    tte_erase_rect_wrapper(HAND_TYPE_RECT);
    hand_type = hand_get_type();
    switch (hand_type)
    {
    case HIGH_CARD:
        print_hand_type("HIGH C");
        chips = 5;
        mult = 1;
        break;
    case PAIR:
        print_hand_type("PAIR");
        chips = 10;
        mult = 2;
        break;
    case TWO_PAIR:
        print_hand_type("2 PAIR");
        chips = 20;
        mult = 2;
        break;
    case THREE_OF_A_KIND:
        print_hand_type("3 OAK");
        chips = 30;
        mult = 3;
        break;
    case STRAIGHT:
        print_hand_type("STRT");
        chips = 30;
        mult = 4;
        break;
    case FLUSH:
        print_hand_type("FLUSH");
        chips = 35;
        mult = 4;
        break;
    case FULL_HOUSE:
        print_hand_type("FULL H");
        chips = 40;
        mult = 4;
        break;
    case FOUR_OF_A_KIND:
        print_hand_type("4 OAK");
        chips = 60;
        mult = 7;
        break;
    case STRAIGHT_FLUSH:
        print_hand_type("STRT F");
        chips = 100;
        mult = 8;
        break;
    case ROYAL_FLUSH:
        print_hand_type("ROYAL F");
        chips = 100;
        mult = 8;
        break;
    case FIVE_OF_A_KIND:
        print_hand_type("5 OAK");
        chips = 120;
        mult = 12;
        break;
    case FLUSH_HOUSE:
        print_hand_type("FLUSH H");
        chips = 140;
        mult = 14;
        break;
    case FLUSH_FIVE:
        print_hand_type("FLUSH 5");
        chips = 160;
        mult = 16;
        break;
    case NONE:
        chips = 0;
        mult = 0;
        break;
    }

    set_chips(chips);
    set_mult(mult);
}

void card_draw()
{
    if (deck_top < 0 || hand_top >= hand_size - 1 || hand_top >= MAX_HAND_SIZE - 1) return;

    CardObject *card_object = card_object_new(deck_pop());

    const FIXED deck_x = int2fx(208);
    const FIXED deck_y = int2fx(110);

    card_object->x = deck_x;
    card_object->y = deck_y;

    hand[++hand_top] = card_object;

    // Sort the hand after drawing a card
    sort_cards();

    const int pitch_lut[MAX_HAND_SIZE] = {1024, 1048, 1072, 1096, 1120, 1144, 1168, 1192, 1216, 1240, 1264, 1288, 1312, 1336, 1360, 1384};
    mm_sound_effect sfx_draw = {{SFX_CARD_DRAW}, pitch_lut[cards_drawn], 0, 255, 128,};
    mmEffectEx(&sfx_draw);
}

void hand_set_focus(int index)
{
    if (index < 0 || index > hand_top || hand_state != HAND_SELECT) return;
    card_focused = index;

    mm_sound_effect sfx_focus = {{SFX_CARD_FOCUS}, 1024 + rand() % 512, 0, 255, 128,};
    mmEffectEx(&sfx_focus);
}

int hand_get_focus()
{
    return card_focused;
}

void hand_select()
{
    if (hand_state != HAND_SELECT || hand[card_focused] == NULL) return;

    if (hand[card_focused]->selected)
    {
        hand[card_focused]->selected = false;
        hand_selections--;

        mm_sound_effect sfx_select = {{SFX_CARD_SELECT}, 1024, 0, 255, 128,};
        mmEffectEx(&sfx_select);
    }
    else if (hand_selections < MAX_SELECTION_SIZE)
    {
        hand[card_focused]->selected = true;
        hand_selections++;

        mm_sound_effect sfx_deselect = {{SFX_CARD_DESELECT}, 1024, 0, 255, 128,};
        mmEffectEx(&sfx_deselect);
    }
}

void hand_change_sort()
{
    sort_by_suit = !sort_by_suit;
    sort_cards();
}

int hand_get_size()
{
    return hand_top + 1;
}

int hand_get_max_size()
{
    return hand_size;
}

bool hand_discard()
{
    if (hand_state != HAND_SELECT || hand_selections == 0) return false;
    hand_state = HAND_DISCARD;
    card_focused = 0;
    return true;
}

bool hand_play()
{
    if (hand_state != HAND_SELECT || hand_selections == 0) return false;
    hand_state = HAND_PLAY;
    card_focused = 0;
    return true;
}

int deck_get_size()
{
    return deck_top + 1;
}

int deck_get_max_size()
{
    return hand_top + played_top + deck_top + discard_top + 4; // This is the max amount of cards that the player currently has in their possession
}

void deck_shuffle()
{
    if (hand_state == HAND_SHUFFLING) return; // Prevent multiple shuffles at the same time
    hand_state = HAND_SHUFFLING;
    card_focused = 0;
}

void increment_blind(enum BlindState increment_reason)
{
    current_blind++;
    if (current_blind >= MAX_BLINDS)
    {
        current_blind = 0;
        blinds[0] = BLIND_CURRENT; // Reset the blinds to the first one
        blinds[1] = BLIND_UPCOMING; // Set the next blind to upcoming
        blinds[2] = BLIND_UPCOMING; // Set the next blind to upcoming
    }
    else
    {
        blinds[current_blind] = BLIND_CURRENT;
        blinds[current_blind - 1] = increment_reason; 
    }
}

void game_round_init()
{
    playing_blind_token = blind_token_new(current_blind, 8, 18, 33); // Create the blind token sprite at the top left corner
    // TODO: Hide blind token and display it after sliding blind rect animation
    //if (playing_blind_token != NULL)
    //{
    //    obj_hide(playing_blind_token->obj); // Hide the blind token sprite for now
    //}
    round_end_blind_token = blind_token_new(current_blind, 82, 86, 34); // Create the blind token sprite for round end
    if (round_end_blind_token != NULL)
    {
        obj_hide(round_end_blind_token->obj); // Hide the blind token sprite for now
    }

    tte_printf("#{P:%d,%d; cx:0xE000}%d", BLIND_REQ_TEXT_RECT.left, BLIND_REQ_TEXT_RECT.top, blind_get_requirement(current_blind, ante)); // Blind requirement
    tte_printf("#{P:%d,%d; cx:0xC000}$%d", BLIND_REWARD_RECT.left, BLIND_REWARD_RECT.top, blind_get_reward(current_blind)); // Blind reward
    // TODO: Add round variable for current round
    tte_printf("#{P:%d,%d; cx:0xC000}%d", ROUND_TEXT_RECT.left, ROUND_TEXT_RECT.top, 1); // Round
}

void init_game_state(enum GameState game_state_to_init)
{
    // Switch written out, add init for states as needed
    switch (game_state_to_init)
    {
    case GAME_PLAYING:
        game_round_init();
        break;
    case GAME_ROUND_END:
        break;
    case GAME_SHOP:
        break;
    case GAME_BLIND_SELECT:
        break;
    case GAME_LOSE:
        break;
    default:
        break;
    }
}

// Game functions
void game_set_state(enum GameState new_game_state)
{
    timer = 0; // Reset the timer
    init_game_state(new_game_state);
    game_state = new_game_state;
}

void game_init()
{
    blind_select_tokens[SMALL_BLIND] = blind_token_new(SMALL_BLIND, 8, 18, 35);
    blind_select_tokens[BIG_BLIND] = blind_token_new(BIG_BLIND, 8, 18, 36);
    blind_select_tokens[BOSS_BLIND] = blind_token_new(BOSS_BLIND, 8, 18, 37);

    obj_hide(blind_select_tokens[SMALL_BLIND]->obj);
    obj_hide(blind_select_tokens[BIG_BLIND]->obj);
    obj_hide(blind_select_tokens[BOSS_BLIND]->obj);

    // Fill the deck with all the cards. Later on this can be replaced with a more dynamic system that allows for different decks and card types.
    for (int suit = 0; suit < NUM_SUITS; suit++)
    {
        for (int rank = 0; rank < NUM_RANKS; rank++)
        {
            Card *card = card_new(suit, rank);
            deck_push(card);
        }
    }

    // TODO: Move to external function
    // Shuffle the deck
    for (int i = 0; i < MAX_DECK_SIZE; i++)
    {
        int j = rand() % MAX_DECK_SIZE;
        Card *temp = deck[i];
        deck[i] = deck[j];
        deck[j] = temp;
    }

    change_background(BG_ID_BLIND_SELECT);

    tte_printf("#{P:%d,%d; cx:0xF000}%d/%d", DECK_SIZE_RECT.left, DECK_SIZE_RECT.top, deck_get_size(), deck_get_max_size()); // Deck size/max size
    
    set_score(score); // Set the score display

    set_chips(chips); // Set the chips display
    set_mult(mult); // Set the multiplier display

    tte_printf("#{P:%d,%d; cx:0xD000}%d", HANDS_TEXT_RECT.left, HANDS_TEXT_RECT.top, hands); // Hand
    tte_printf("#{P:%d,%d; cx:0xE000}%d", DISCARDS_TEXT_RECT.left, DISCARDS_TEXT_RECT.top, discards); // Discard

    //tte_printf("#{P:24,120; cx:0xC000}$%d", money); // Money
    set_money(money); // Set the money display

    tte_printf("#{P:%d,%d; cx:0xC000}%d#{cx:0xF000}/%d", ANTE_TEXT_RECT.left, ANTE_TEXT_RECT.top, ante, MAX_ANTE); // Ante
}

static void game_playing_process_input_and_state()
{
    if (hand_state == HAND_SELECT)
    {
        if (key_hit(KEY_LEFT))
        {
            hand_set_focus(hand_get_focus() + 1); // The reason why this adds 1 is because the hand is drawn from right to left. There is no particular reason for this, it's just how I did it.
        }
        else if (key_hit(KEY_RIGHT))
        {
            hand_set_focus(hand_get_focus() - 1);
        }

        if (key_hit(KEY_A))
        {
            hand_select();
            set_hand();
        }

        if (key_hit(KEY_B))
        {
            hand_change_sort();
        }

        if (key_hit(KEY_SELECT) && discards > 0 && hand_discard())
        {
            set_hand();
            discards--;
            tte_printf("#{P:%d,%d; cx:0xE000}%d", DISCARDS_TEXT_RECT.left, DISCARDS_TEXT_RECT.top, discards);
        }

        if (key_hit(KEY_START) && hands > 0 && hand_play())
        {
            hands--;
            tte_printf("#{P:%d,%d; cx:0xD000}%d", HANDS_TEXT_RECT.left, HANDS_TEXT_RECT.top, hands);
        }
    }
    else if (play_state == PLAY_ENDING)
    {
        if (mult > 0)
        {
            temp_score = chips * mult;
            lerped_temp_score = int2fx(temp_score);
            lerped_score = int2fx(score);

            set_temp_score(temp_score);

            chips = 0;
            mult = 0;
            set_mult(mult);
            set_chips(chips);
        }
    }
    else if (play_state == PLAY_ENDED)
    {
        lerped_temp_score -= int2fx(temp_score) / 40;
        lerped_score += int2fx(temp_score) / 40;

        if (lerped_temp_score > 0)
        {
            set_temp_score(fx2int(lerped_temp_score));

            // We actually don't need to erase this because the score only increases
            set_score(fx2int(lerped_score)); // Set the score display

            if (temp_score <= 0)
            {
                tte_erase_rect_wrapper(TEMP_SCORE_RECT);
            }
        }
        else
        {
            score += temp_score;
            temp_score = 0;
            lerped_temp_score = 0;
            lerped_score = 0;

            tte_erase_rect_wrapper(TEMP_SCORE_RECT); // Just erase the temp score

            set_score(score);
        }
    }
}

static void game_playing_process_card_draw()
{
    if (hand_state == HAND_DRAW && cards_drawn < hand_size)
    {
        if (timer % FRAMES(10) == 0) // Draw a card every 10 frames
        {
            cards_drawn++;
            card_draw();
        }
    }
    else if (hand_state == HAND_DRAW)
    {
        hand_state = HAND_SELECT; // Change the hand state to select after drawing all the cards
        cards_drawn = 0;
        timer = 0;
    }
}

static void game_playing_discarded_cards_loop()
{
    // Discarded cards loop (mainly for shuffling)
    if (hand_get_size() == 0 && hand_state == HAND_SHUFFLING && discard_top >= -1 && timer > FRAMES(10))
    {
        change_background(BG_ID_ROUND_END); // Change the background to the round end background. This is how it works in Balatro, so I'm doing it this way too.

        // We take each discarded card and put it back into the deck with a short animation
        static CardObject* discarded_card_object = NULL;
        if (discarded_card_object == NULL)
        {
            discarded_card_object = card_object_new(discard_pop());
            //discarded_card_object->sprite = sprite_new(ATTR0_SQUARE | ATTR0_4BPP | ATTR0_AFF, ATTR1_SIZE_32, card_sprite_lut[discarded_card_object->card->suit][discarded_card_object->card->rank], 0, 0);
            card_object_set_sprite(discarded_card_object, 0); // Set the sprite for the discarded card object
            discarded_card_object->tx = int2fx(204);
            discarded_card_object->ty = int2fx(112);
            discarded_card_object->x = int2fx(240);
            discarded_card_object->y = int2fx(80);
            discarded_card_object->vx = 0;
            discarded_card_object->vy = 0;
            discarded_card_object->scale = float2fx(1.0f);
            discarded_card_object->vscale = float2fx(0.0f);
            discarded_card_object->rotation = 0;
            discarded_card_object->vrotation = 0;

            card_object_update(discarded_card_object);
        }
        else
        {
            card_object_update(discarded_card_object);

            if (discarded_card_object->y >= discarded_card_object->ty)
            {
                deck_push(discarded_card_object->card); // Put the card back into the deck
                card_object_destroy(&discarded_card_object);

                // play draw sound
                const int pitch_lut[MAX_HAND_SIZE] = { 1024, 1048, 1072, 1096, 1120, 1144, 1168, 1192, 1216, 1240, 1264, 1288, 1312, 1336, 1360, 1384 };
                mm_sound_effect sfx_draw = { {SFX_CARD_DRAW}, pitch_lut[2], 0, 255, 128, };
                mmEffectEx(&sfx_draw);
            }
        }

        if (discard_top == -1 && discarded_card_object == NULL) // If there are no more discarded cards, stop shuffling
        {
            hand_state = HAND_SELECT; // Reset the hand state to the functional default
            game_set_state(GAME_ROUND_END); // Set the game state back to playing
        }
    }
}

static void cards_in_hand_update_loop(bool* discarded_card, int* played_selections, bool* sound_played)
{
    for (int i = hand_top + 1; i >= 0; i--) // Start from the end of the hand and work backwards because that's how Balatro does it
    {
        if (hand[i] != NULL)
        {
            const int spacing_lut[MAX_HAND_SIZE] = { 28, 28, 28, 28, 27, 21, 18, 15, 13, 12, 10, 9, 9, 8, 8, 7 }; // This is a stupid way to do this but I don't care

            FIXED hand_x = int2fx(120);
            FIXED hand_y = int2fx(90);

            switch (hand_state)
            {
            case HAND_DRAW:
                hand_x = hand_x + (int2fx(i) - int2fx(hand_top) / 2) * -spacing_lut[hand_top];
                break;
            case HAND_SELECT:
                if (i == card_focused && !hand[i]->selected)
                {
                    hand_y -= (10 << FIX_SHIFT);
                }
                else if (i != card_focused && hand[i]->selected)
                {
                    hand_y -= (15 << FIX_SHIFT);
                }
                else if (i == card_focused && hand[i]->selected)
                {
                    hand_y -= (20 << FIX_SHIFT);
                }

                if (i != card_focused && hand[i]->y > hand_y)
                {
                    hand[i]->y = hand_y;
                    hand[i]->vy = 0;
                }

                hand_x = hand_x + (int2fx(i) - int2fx(hand_top) / 2) * -spacing_lut[hand_top]; // TODO: Change this later to reference a 2D LUT of positions
                break;
            case HAND_SHUFFLING:
                /* FALL THROUGH */
            case HAND_DISCARD: // TODO: Add sound
                if (hand[i]->selected || hand_state == HAND_SHUFFLING)
                {
                    if (!*discarded_card)
                    {
                        hand_x = int2fx(240);
                        hand_y = int2fx(70);

                        if (!*sound_played)
                        {
                            const int pitch_lut[MAX_SELECTION_SIZE] = { 1024, 960, 896, 832, 768 };
                            mm_sound_effect sfx_draw = { {SFX_CARD_DRAW}, pitch_lut[cards_drawn], 0, 255, 128, };
                            mmEffectEx(&sfx_draw);
                            *sound_played = true;
                        }

                        if (hand[i]->x >= hand_x)
                        {
                            discard_push(hand[i]->card);
                            card_object_destroy(&hand[i]);
                            sort_cards();

                            hand_top--;
                            cards_drawn++; // This technically isn't drawing cards, I'm just reusing the variable
                            *sound_played = false;
                            timer = 0;

                            hand_y = hand[i]->y;
                            hand_x = hand[i]->x;
                        }

                        *discarded_card = true;
                    }
                    else
                    {
                        if (hand_state == HAND_DISCARD)
                        {
                            hand_y -= (15 << FIX_SHIFT); // Don't raise the card if we're mass discarding, it looks stupid.
                        }
                        else
                        {
                            hand_y += (24 << FIX_SHIFT);
                        }
                        hand_x = hand_x + (int2fx(i) - int2fx(hand_top) / 2) * -spacing_lut[hand_top];
                    }
                }
                else
                {
                    hand_x = hand_x + (int2fx(i) - int2fx(hand_top) / 2) * -spacing_lut[hand_top];
                }

                if (i == 0 && *discarded_card == false && timer % FRAMES(10) == 0)
                {
                    hand_state = HAND_DRAW;
                    *sound_played = false;
                    cards_drawn = 0;
                    hand_selections = 0;
                    timer = 0;
                    break;
                }

                break;
            case HAND_PLAY:
                hand_x = hand_x + (int2fx(i) - int2fx(hand_top) / 2) * -spacing_lut[hand_top];
                hand_y += (24 << FIX_SHIFT);

                if (hand[i]->selected && *discarded_card == false && timer % FRAMES(10) == 0)
                {
                    hand[i]->selected = false;
                    played_push(hand[i]);
                    sprite_destroy(&hand[i]->sprite);
                    hand[i] = NULL;
                    sort_cards();

                    const int pitch_lut[MAX_SELECTION_SIZE] = { 1024, 960, 896, 832, 768 };
                    mm_sound_effect sfx_draw = { {SFX_CARD_DRAW}, pitch_lut[cards_drawn], 0, 255, 128, };
                    mmEffectEx(&sfx_draw);

                    hand_top--;
                    hand_selections--;
                    cards_drawn++;

                    *discarded_card = true;
                }

                if (i == 0 && *discarded_card == false && timer % FRAMES(10) == 0)
                {
                    hand_state = HAND_PLAYING;
                    cards_drawn = 0;
                    hand_selections = 0;
                    timer = 0;
                    *played_selections = played_top + 1;

                    switch (hand_type) // select the cards that apply to the hand type
                    {
                    case NONE:
                        break;
                    case HIGH_CARD: // find the card with the highest rank in the hand
                        int highest_rank_index = 0;

                        for (int i = 0; i <= played_top; i++)
                        {
                            if (played[i]->card->rank > played[highest_rank_index]->card->rank)
                            {
                                highest_rank_index = i;
                            }
                        }

                        played[highest_rank_index]->selected = true;
                        break;
                    case PAIR: // find two cards with the same rank (Requires recursion)
                        for (int i = 0; i <= played_top - 1; i++)
                        {
                            for (int j = i + 1; j <= played_top; j++)
                            {
                                if (played[i]->card->rank == played[j]->card->rank)
                                {
                                    played[i]->selected = true;
                                    played[j]->selected = true;
                                    break;
                                }
                            }

                            if (played[i]->selected) break;
                        }
                        break;
                    case TWO_PAIR: // find two pairs of cards with the same rank (Requires recursion)
                        int i;

                        for (i = 0; i <= played_top - 1; i++)
                        {
                            for (int j = i + 1; j <= played_top; j++)
                            {
                                if (played[i]->card->rank == played[j]->card->rank)
                                {
                                    played[i]->selected = true;
                                    played[j]->selected = true;

                                    break;
                                }
                            }

                            if (played[i]->selected) break;
                        }

                        for (; i <= played_top - 1; i++) // Find second pair
                        {
                            for (int j = i + 1; j <= played_top; j++)
                            {
                                if (played[i]->card->rank == played[j]->card->rank && played[i]->selected == false && played[j]->selected == false)
                                {
                                    played[i]->selected = true;
                                    played[j]->selected = true;
                                    break;
                                }
                            }
                        }
                        break;
                    case THREE_OF_A_KIND: // find three cards with the same rank (requires recursion)
                        for (int i = 0; i <= played_top - 1; i++)
                        {
                            for (int j = i + 1; j <= played_top; j++)
                            {
                                if (played[i]->card->rank == played[j]->card->rank)
                                {
                                    played[i]->selected = true;
                                    played[j]->selected = true;

                                    for (int k = j + 1; k <= played_top; k++)
                                    {
                                        if (played[i]->card->rank == played[k]->card->rank && played[k]->selected == false)
                                        {
                                            played[k]->selected = true;
                                            break;
                                        }
                                    }

                                    break;
                                }
                            }

                            if (played[i]->selected) break;
                        }
                        break;
                    case FOUR_OF_A_KIND: // find four cards with the same rank (requires recursion)
                        if (played_top >= 3) // If there are 5 cards selected we just need to find the one card that doesn't match, and select the others
                        {
                            int unmatched_index = -1;

                            for (int i = 0; i <= played_top; i++)
                            {
                                if (played[i]->card->rank != played[(i + 1) % played_top]->card->rank && played[i]->card->rank != played[(i + 2) % played_top]->card->rank)
                                {
                                    unmatched_index = i;
                                    break;
                                }
                            }

                            for (int i = 0; i <= played_top; i++)
                            {
                                if (i != unmatched_index)
                                {
                                    played[i]->selected = true;
                                }
                            }
                        }
                        else // If there are only 4 cards selected we know they match
                        {
                            for (int i = 0; i <= played_top; i++)
                            {
                                played[i]->selected = true;
                            }
                        }
                        break;
                    case STRAIGHT:
                        /* FALL THROUGH */
                    case FLUSH:
                        /* FALL THROUGH */
                    case FULL_HOUSE:
                        /* FALL THROUGH */
                    case STRAIGHT_FLUSH:
                        /* FALL THROUGH */
                    case ROYAL_FLUSH:
                        /* FALL THROUGH */
                    case FIVE_OF_A_KIND:
                        /* FALL THROUGH */
                    case FLUSH_HOUSE:
                        /* FALL THROUGH */
                    case FLUSH_FIVE: // Select all played cards in the hand (This is functionally identical as the above hand types)
                        for (int i = 0; i <= played_top; i++)
                        {
                            played[i]->selected = true;
                        }
                        break;
                    }
                }

                break;
            case HAND_PLAYING: // Don't need to do anything here, just wait for the player to select cards
                hand_x = hand_x + (int2fx(i) - int2fx(hand_top) / 2) * -spacing_lut[hand_top];
                hand_y += (24 << FIX_SHIFT);
                break;
            }

            hand[i]->tx = hand_x;
            hand[i]->ty = hand_y;
            card_object_update(hand[i]);
        }
    }
}

static void played_cards_update_loop(bool* discarded_card, int* played_selections, bool* sound_played)
{
    // So this one is a bit fucking weird because I have to work kinda backwards for everything because of the order of the pushed cards from the hand to the play stack
    // (also crazy that the company that published Balatro is called "Playstack" and this is a play stack, but I digress)
    for (int i = 0; i <= played_top; i++)
    {
        if (played[i] != NULL)
        {
            if (played[i]->sprite == NULL)
            {
                //played[i]->sprite = sprite_new(ATTR0_SQUARE | ATTR0_4BPP | ATTR0_AFF, ATTR1_SIZE_32, card_sprite_lut[played[i]->card->suit][played[i]->card->rank], 0, i + MAX_HAND_SIZE);
                card_object_set_sprite(played[i], i + MAX_HAND_SIZE); // Set the sprite for the played card object
            }

            FIXED played_x = int2fx(120);
            FIXED played_y = int2fx(70);
            FIXED played_scale = float2fx(1.0f);

            played_x = played_x + (int2fx(played_top - i) - int2fx(played_top) / 2) * -27;

            switch (play_state)
            {
                case PLAY_PLAYING:
                    if (i == 0 && (timer % FRAMES(10) == 0 || played[played_top - *played_selections]->selected == false) && timer > FRAMES(40))
                    {
                        (*played_selections)--;

                        if (*played_selections == 0)
                        {
                            play_state = PLAY_SCORING;
                            timer = 0;
                        }
                    }

                    if (played[i]->selected && played_top - i >= *played_selections)
                    {
                        played_y -= (10 << FIX_SHIFT);
                    }
                    break;
                case PLAY_SCORING:
                    if (i == 0 && (timer % FRAMES(30) == 0) && timer > FRAMES(40))
                    {
                        // So pretend "played_selections" is now called "scored_cards" and it counts the number of cards that have been scored
                        int scored_cards = 0;
                        for (int j = played_top; j >= 0; j--)
                        {
                            if (played[played_top - j]->selected)
                            {
                                scored_cards++;
                                if (scored_cards > *played_selections)
                                {
                                    tte_erase_rect_wrapper(PLAYED_CARDS_SCORES_RECT);
                                    tte_set_pos(fx2int(played[played_top - j]->x) + 8, 48); // Offset of 16 pixels to center the text on the card
                                    tte_set_special(0xD000); // Set text color to blue from background memory

                                    // Write the score to a character buffer variable
                                    char score_buffer[5]; // Assuming the maximum score is 99, we need 4 characters (2 digits + null terminator)
                                    snprintf(score_buffer, sizeof(score_buffer), "+%d", card_get_value(played[played_top - j]->card));
                                    tte_write(score_buffer);

                                    *played_selections = scored_cards;
                                    //played[j]->vy += (3 << FIX_SHIFT);
                                    played[played_top - j]->vscale = float2fx(0.3f); // Scale down the card when it's scored
                                    played[played_top - j]->vrotation = float2fx(8.0f); // Rotate the card when it's scored

                                    mm_sound_effect sfx_select = {{SFX_CARD_SELECT}, 1024, 0, 255, 128,};
                                    mmEffectEx(&sfx_select);

                                    // Relocated card scoring logic here
                                    chips += card_get_value(played[played_top - j]->card);
                                    set_chips(chips);
                                    break;
                                }
                            }

                            if (j == 0 && scored_cards == *played_selections) // Check if it's the last card 
                            {
                                tte_erase_rect_wrapper(PLAYED_CARDS_SCORES_RECT);
                                play_state = PLAY_ENDING;
                                timer = 0;
                                *played_selections = played_top + 1; // Reset the played selections to the top of the played stack
                                break;
                            }
                        }
                    }

                    if (played[i]->selected)
                    {
                        played_y -= (10 << FIX_SHIFT);
                    }
                    break;
                case PLAY_ENDING: // This is the reverse of PLAY_PLAYING. The cards get reset back to their neutral position sequentially
                    if (i == 0 && (timer % FRAMES(10) == 0 || played[played_top - *played_selections]->selected == false) && timer > FRAMES(40))
                    {
                        (*played_selections)--;

                        if (*played_selections == 0)
                        {
                            play_state = PLAY_ENDED;
                            timer = 0;
                        }
                    }

                    if (played[i]->selected && played_top - i <= *played_selections - 1)
                    {
                        played_y -= (10 << FIX_SHIFT);
                    }
                    break;
                case PLAY_ENDED: // Basically a copy of HAND_DISCARD
                    if (!*discarded_card && played[i] != NULL && timer > FRAMES(40))
                    {
                        played_x = int2fx(240);
                        played_y = int2fx(70);

                        if (!*sound_played)
                        {
                            const int pitch_lut[MAX_SELECTION_SIZE] = {1024, 960, 896, 832, 768};
                            mm_sound_effect sfx_draw = {{SFX_CARD_DRAW}, pitch_lut[cards_drawn], 0, 255, 128,};
                            mmEffectEx(&sfx_draw);
                            *sound_played = true;
                        }

                        if (played[i]->x >= played_x)
                        {
                            discard_push(played[i]->card); // Push the card to the discard pile
                            card_object_destroy(&played[i]);

                            //played_top--; 
                            cards_drawn++; // This technically isn't drawing cards, I'm just reusing the variable
                            *sound_played = false;

                            if (i == played_top)
                            {
                                if (score >= blind_get_requirement(current_blind, ante))
                                {
                                    hand_state = HAND_SHUFFLING;
                                }
                                else
                                {
                                    hand_state = HAND_DRAW;
                                }

                                play_state = PLAY_PLAYING;
                                cards_drawn = 0;
                                hand_selections = 0;
                                *played_selections = 0;
                                played_top = -1; // Reset the played stack
                                timer = 0;
                                break; // Break out of the loop to avoid accessing an invalid index
                            }
                        }

                        *discarded_card = true;
                    }

                    break;
            }

            played[i]->tx = played_x;
            played[i]->ty = played_y;
            played[i]->tscale = played_scale;
            card_object_update(played[i]);
        }
    }
}

static void game_playing_ui_text_update()
{
    static int last_hand_size = 0;
    static int last_deck_size = 0;

    if (last_hand_size != hand_get_size() || last_deck_size != deck_get_size())
    {
        if (background == BG_ID_CARD_SELECTING)
        {
            tte_printf("#{P:%d,%d; cx:0xF000}%d/%d", HAND_SIZE_RECT_SELECT.left, HAND_SIZE_RECT_SELECT.top, hand_get_size(), hand_get_max_size()); // Hand size/max size
        }
        else if (background == BG_ID_CARD_PLAYING)
        {
            tte_printf("#{P:%d,%d; cx:0xF000}%d/%d", HAND_SIZE_RECT_PLAYING.left, HAND_SIZE_RECT_PLAYING.top, hand_get_size(), hand_get_max_size()); // Hand size/max size
        }

        tte_printf("#{P:%d,%d; cx:0xF000}%d/%d", DECK_SIZE_RECT.left, DECK_SIZE_RECT.top, deck_get_size(), deck_get_max_size()); // Deck size/max size

        last_hand_size = hand_get_size();
        last_deck_size = deck_get_size();
    }
}

void game_playing()
{
    // TODO: Blind rect sliding into view animation...
    if (hand_state == HAND_SELECT && hands == 0) 
    {
        // TODO: Check if it's safe to change to game_set_state() without side effects
        game_state = GAME_LOSE;
    }

    // Background logic (thissss might be moved to the card'ssss logic later. I'm a sssssnake)
    if (hand_state == HAND_DRAW || hand_state == HAND_DISCARD || hand_state == HAND_SELECT)
    {
        change_background(BG_ID_CARD_SELECTING);
    }
    else if (hand_state != HAND_SHUFFLING)
    {
        change_background(BG_ID_CARD_PLAYING);
    }

    game_playing_process_input_and_state();

    // Card logic

    game_playing_process_card_draw();

    game_playing_discarded_cards_loop();

    static int played_selections = 0;
    static bool sound_played = false;
    bool discarded_card = false;

    cards_in_hand_update_loop(&discarded_card, &played_selections, &sound_played);
	played_cards_update_loop(&discarded_card, &played_selections, &sound_played);
    
    game_playing_ui_text_update();
}

void game_round_end_cleanup()
{
    // Cleanup blind tokens from this round to avoid accumulating 
    // allocated blind sprites each round
    sprite_destroy(&playing_blind_token);
    sprite_destroy(&round_end_blind_token);
    // TODO: Reuse sprites for blind selection?
}

void game_round_end() // Writing this kind a made me want to kms. If somewone wants to rewrite this, please do so.
{
    /* TODO: I could use main_bg_se_copy_rect_1_tile_vert() to replace the menu pop up here
     * But there are a bunch of other manual hard-coded tilemap animations in here that
     * are very hard to understand, and if I change the background image for the pop up
     * it will change the tile charblock layout and screw them up so it's all or nothing...
     * - Meir
     */ 
    static int state = 0;
    static int sequence_step = 0; // Reusable variable for the animations in states

    static int blind_reward = 0;
    static int hand_reward = 0;
    static int interest_reward = 0; 

    switch (state)
    {
        case 0:
        {
            if (timer == 30) // Reset static variables to default values upon re-entering the round end state
            {   
                change_background(BG_ID_ROUND_END); // Change the background to the round end background
                state = 1; // Change the state to the next one
                sequence_step = 0;
                timer = 0; // Reset the timer
                blind_reward = blind_get_reward(current_blind);
                hand_reward = hands;
            }
            break;
        }
        case 1: // This creates the top 16 by 7 tiles of the pop up. It places it in vram, moving it up one tile each frame, not clearing the previous row of tiles so they fill the blank space as it moves up.
        {
            const int bottom_of_screen = 19;
            int y = bottom_of_screen - timer;

            // 1st row
            const unsigned short tile_map1[17] = {se_mem[MAIN_BG_SBB][8 + 32 * y], 0x0026, 0x0027, 0x0027, 0x0027, 0x0027, 0x0027, 0x0027, 0x0027, 0x0027, 0x0027, 0x0027, 0x0027, 0x0027, 0x0027, 0x0027, 0x0426};
            memcpy(&se_mem[MAIN_BG_SBB][8 + 32 * y], tile_map1, sizeof(tile_map1));

            // 2nd row
            y += 1;
            if (y > bottom_of_screen) break;
            const unsigned short tile_map2[17] = {se_mem[MAIN_BG_SBB][8 + 32 * y], 0x002A, 0x042D, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x002D, 0x042A};
            memcpy(&se_mem[MAIN_BG_SBB][8 + 32 * y], tile_map2, sizeof(tile_map2));

            // 3rd row
            y += 1;
            if (y > bottom_of_screen) break;
            unsigned short tile_map3[17] = {se_mem[MAIN_BG_SBB][8 + 32 * y], 0x002A, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x042A};
            memcpy(&se_mem[MAIN_BG_SBB][8 + 32 * y], tile_map3, sizeof(tile_map3));

            // 4th row
            y += 1;
            if (y > bottom_of_screen) break;
            tile_map3[0] = se_mem[MAIN_BG_SBB][8 + 32 * y];
            memcpy(&se_mem[MAIN_BG_SBB][8 + 32 * y], tile_map3, sizeof(tile_map3));

            // 5th row
            y += 1;
            if (y > bottom_of_screen) break;
            tile_map3[0] = se_mem[MAIN_BG_SBB][8 + 32 * y];
            memcpy(&se_mem[MAIN_BG_SBB][8 + 32 * y], tile_map3, sizeof(tile_map3));

            // 6th row
            y += 1;
            if (y > bottom_of_screen) break;
            const unsigned short tile_map4[17] = {se_mem[MAIN_BG_SBB][8 + 32 * y], 0x002A, 0x0055, 0x0056, 0x0056, 0x0056, 0x0056, 0x0056, 0x0056, 0x0056, 0x0056, 0x0056, 0x0056, 0x0056, 0x0056, 0x0455, 0x042A};
            memcpy(&se_mem[MAIN_BG_SBB][8 + 32 * y], tile_map4, sizeof(tile_map4));

            // 7th row
            y += 1;
            if (y > bottom_of_screen) break;
            const unsigned short tile_map5[17] = {se_mem[MAIN_BG_SBB][8 + 32 * y], 0x002A, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x042A};
            memcpy(&se_mem[MAIN_BG_SBB][8 + 32 * y], tile_map5, sizeof(tile_map5));

            if (timer >= 12)
            {
                state = 2;
                timer = 0;
            }
            break;
        }
        case 2: // Display the beaten blind, expand the panel border down a tile and wait until a bit until going to the next state
        {
            obj_unhide(round_end_blind_token->obj, 0);
            
            tte_printf("#{P:%d,%d; cx:0xE000}%d", ROUND_END_BLIND_REQ_RECT.left, ROUND_END_BLIND_REQ_RECT.top, blind_get_requirement(current_blind, ante));

            int y = 13;

            const unsigned short tile_map1[17] = {se_mem[MAIN_BG_SBB][8 + 32 * (y - 1)], 0x002A, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x042A};
            memcpy(&se_mem[MAIN_BG_SBB][8 + 32 * (y - 1)], tile_map1, sizeof(tile_map1));

            const unsigned short tile_map2[17] = {se_mem[MAIN_BG_SBB][8 + 32 * y], 0x002A, 0x0055, 0x0056, 0x0056, 0x0056, 0x0056, 0x0056, 0x0056, 0x0056, 0x0056, 0x0056, 0x0056, 0x0056, 0x0056, 0x0455, 0x042A};
            memcpy(&se_mem[MAIN_BG_SBB][8 + 32 * y], tile_map2, sizeof(tile_map2));

            if (timer >= 30)
            {
                state = 3;
                timer = 0;
            }
            break;
        }
        case 3: // Sequentially display the "score min" text over the next 4 frames
        {
            // "Score Min" text
            const unsigned short tile_map3[4] = {0x003E, 0x003F, 0x0040, 0x0041};

            int temp_offset = timer;
            memcpy(&se_mem[MAIN_BG_SBB][12 + temp_offset + 32 * 11], &tile_map3[temp_offset - 1], sizeof(tile_map3[0]));

            if (timer >= 4)
            {
                state = 4;
                timer = 0;
            }
            break;
        }
        case 4: // Every 20 frames, display the blind reward and update the text until it reaches 0
        {
            if (timer % FRAMES(20) != 0) break;

            // TODO: Add sound effect here

            sequence_step++;

            blind_reward--;
            tte_printf("#{P:%d,%d; cx:0xC000}$%d", BLIND_REWARD_RECT.left , BLIND_REWARD_RECT.top, blind_reward);
            tte_printf("#{P:%d,%d; cx:0xC000}$%d", ROUND_END_BLIND_REWARD_RECT.left, ROUND_END_BLIND_REQ_RECT.top, blind_get_reward(current_blind) - blind_reward);

            if (blind_reward <= 0)
            {
                tte_erase_rect_wrapper(BLIND_REWARD_RECT);
                obj_hide(playing_blind_token->obj);
                state = 5;
                sequence_step = 0;
                timer = 0;
            }
            break;
        }
        case 5: // This is just an incredibly convoluted way to slide the "small blind" panel out of view. It was made this way because I didn't know the better way at the time AND because I was being a perfectionist and wanted to make the bottom row of tiles on the panel not connected to the "SCORE" panel.
        {
            if (timer < FRAMES(20)) break;

            sequence_step++;
            {
                int y = 5;

                if (sequence_step == 1) // Erase the score text from the small blind panel and move it all up by one tile
                {
                    tte_erase_rect(40, 16, 64, 40);

                    const unsigned short tile_map1[10] = {0x0064, 0x0065, 0x0065, 0x0066, 0x0067, 0x0067, 0x0067, 0x0068, 0x0464, se_mem[MAIN_BG_SBB][32 * y + 9]}; // Swaps the top of the score panel tiles with ones that aren't connected to the small blind panel
                    memcpy(&se_mem[MAIN_BG_SBB][32 * y], &tile_map1, sizeof(tile_map1));

                    y -= 1;
                    const unsigned short tile_map2[10] = {0x006F, 0x0070, 0x0070, 0x0070, 0x0070, 0x0070, 0x0070, 0x0070, 0x046F, se_mem[MAIN_BG_SBB][32 * y + 9]}; // Swaps the bottom of the small blind panel tiles with ones that aren't connected to the score panel
                    memcpy(&se_mem[MAIN_BG_SBB][32 * y], &tile_map2, sizeof(tile_map2));

                    // These just move everything else up by one tile
                    y = 1;
                    const unsigned short tile_map3[10] = {se_mem[MAIN_BG_SBB][32 * y], se_mem[MAIN_BG_SBB][32 * y + 1], se_mem[MAIN_BG_SBB][32 * y + 2], se_mem[MAIN_BG_SBB][32 * y + 3], se_mem[MAIN_BG_SBB][32 * y + 4], se_mem[MAIN_BG_SBB][32 * y + 5], se_mem[MAIN_BG_SBB][32 * y + 6], se_mem[MAIN_BG_SBB][32 * y + 7], se_mem[MAIN_BG_SBB][32 * y + 8], se_mem[MAIN_BG_SBB][32 * (y - 1) + 9]};
                    y -= 1;
                    memcpy(&se_mem[MAIN_BG_SBB][32 * y], &tile_map3, sizeof(tile_map3));

                    y = 2;
                    const unsigned short tile_map4[10] = {se_mem[MAIN_BG_SBB][32 * y], se_mem[MAIN_BG_SBB][32 * y + 1], se_mem[MAIN_BG_SBB][32 * y + 2], se_mem[MAIN_BG_SBB][32 * y + 3], se_mem[MAIN_BG_SBB][32 * y + 4], se_mem[MAIN_BG_SBB][32 * y + 5], se_mem[MAIN_BG_SBB][32 * y + 6], se_mem[MAIN_BG_SBB][32 * y + 7], se_mem[MAIN_BG_SBB][32 * y + 8], se_mem[MAIN_BG_SBB][32 * (y - 1) + 9]};
                    y -= 1;
                    memcpy(&se_mem[MAIN_BG_SBB][32 * y], &tile_map4, sizeof(tile_map4));

                    y = 3;
                    const unsigned short tile_map5[10] = {se_mem[MAIN_BG_SBB][32 * y], se_mem[MAIN_BG_SBB][32 * y + 1], se_mem[MAIN_BG_SBB][32 * y + 2], se_mem[MAIN_BG_SBB][32 * y + 3], se_mem[MAIN_BG_SBB][32 * y + 4], se_mem[MAIN_BG_SBB][32 * y + 5], se_mem[MAIN_BG_SBB][32 * y + 6], se_mem[MAIN_BG_SBB][32 * y + 7], se_mem[MAIN_BG_SBB][32 * y + 8], se_mem[MAIN_BG_SBB][32 * (y - 1) + 9]};
                    y -= 1;
                    memcpy(&se_mem[MAIN_BG_SBB][32 * y], &tile_map5, sizeof(tile_map5));
                }
                else if (sequence_step == 2)
                {
                       
                    y = 1;
                    const unsigned short tile_map3[10] = {se_mem[MAIN_BG_SBB][32 * y], se_mem[MAIN_BG_SBB][32 * y + 1], se_mem[MAIN_BG_SBB][32 * y + 2], se_mem[MAIN_BG_SBB][32 * y + 3], se_mem[MAIN_BG_SBB][32 * y + 4], se_mem[MAIN_BG_SBB][32 * y + 5], se_mem[MAIN_BG_SBB][32 * y + 6], se_mem[MAIN_BG_SBB][32 * y + 7], se_mem[MAIN_BG_SBB][32 * y + 8], se_mem[MAIN_BG_SBB][32 * (y - 1) + 9]};
                    y -= 1;
                    memcpy(&se_mem[MAIN_BG_SBB][32 * y], &tile_map3, sizeof(tile_map3));
                    
                    y = 2;
                    const unsigned short tile_map4[10] = {se_mem[MAIN_BG_SBB][32 * y], se_mem[MAIN_BG_SBB][32 * y + 1], se_mem[MAIN_BG_SBB][32 * y + 2], se_mem[MAIN_BG_SBB][32 * y + 3], se_mem[MAIN_BG_SBB][32 * y + 4], se_mem[MAIN_BG_SBB][32 * y + 5], se_mem[MAIN_BG_SBB][32 * y + 6], se_mem[MAIN_BG_SBB][32 * y + 7], se_mem[MAIN_BG_SBB][32 * y + 8], se_mem[MAIN_BG_SBB][32 * (y - 1) + 9]};
                    y -= 1;
                    memcpy(&se_mem[MAIN_BG_SBB][32 * y], &tile_map4, sizeof(tile_map4));

                    y = 3;
                    const unsigned short tile_map5[10] = {se_mem[MAIN_BG_SBB][32 * y], se_mem[MAIN_BG_SBB][32 * y + 1], se_mem[MAIN_BG_SBB][32 * y + 2], se_mem[MAIN_BG_SBB][32 * y + 3], se_mem[MAIN_BG_SBB][32 * y + 4], se_mem[MAIN_BG_SBB][32 * y + 5], se_mem[MAIN_BG_SBB][32 * y + 6], se_mem[MAIN_BG_SBB][32 * y + 7], se_mem[MAIN_BG_SBB][32 * y + 8], se_mem[MAIN_BG_SBB][32 * (y - 1) + 9]};
                    y -= 1;
                    memcpy(&se_mem[MAIN_BG_SBB][32 * y], &tile_map5, sizeof(tile_map5));

                    y = 4;
                    const unsigned short tile_map6[10] = {se_mem[MAIN_BG_SBB][32 * y], se_mem[MAIN_BG_SBB][32 * y + 1], se_mem[MAIN_BG_SBB][32 * y + 2], se_mem[MAIN_BG_SBB][32 * y + 3], se_mem[MAIN_BG_SBB][32 * y + 4], se_mem[MAIN_BG_SBB][32 * y + 5], se_mem[MAIN_BG_SBB][32 * y + 6], se_mem[MAIN_BG_SBB][32 * y + 7], se_mem[MAIN_BG_SBB][32 * y + 8], se_mem[MAIN_BG_SBB][32 * (y - 1) + 9]};
                    y -= 1;
                    memcpy(&se_mem[MAIN_BG_SBB][32 * y], &tile_map6, sizeof(tile_map6));

                    y = 4;
                    const unsigned short tile_map2[10] = {0x046B, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x006B, se_mem[MAIN_BG_SBB][32 * y + 9]}; // This sets the uncleared bottom row of the small blind panel to empty tiles that match the stats panel
                    memcpy(&se_mem[MAIN_BG_SBB][32 * y], &tile_map2, sizeof(tile_map2));
                }
                else // The rest of this just moves the panel tiles up in vram until they're gone
                {
                    y = 1;
                    const unsigned short tile_map3[10] = {se_mem[MAIN_BG_SBB][32 * y], se_mem[MAIN_BG_SBB][32 * y + 1], se_mem[MAIN_BG_SBB][32 * y + 2], se_mem[MAIN_BG_SBB][32 * y + 3], se_mem[MAIN_BG_SBB][32 * y + 4], se_mem[MAIN_BG_SBB][32 * y + 5], se_mem[MAIN_BG_SBB][32 * y + 6], se_mem[MAIN_BG_SBB][32 * y + 7], se_mem[MAIN_BG_SBB][32 * y + 8], se_mem[MAIN_BG_SBB][32 * (y - 1) + 9]};
                    y -= 1;
                    memcpy(&se_mem[MAIN_BG_SBB][32 * y], &tile_map3, sizeof(tile_map3));
                    
                    y = 2;
                    const unsigned short tile_map4[10] = {se_mem[MAIN_BG_SBB][32 * y], se_mem[MAIN_BG_SBB][32 * y + 1], se_mem[MAIN_BG_SBB][32 * y + 2], se_mem[MAIN_BG_SBB][32 * y + 3], se_mem[MAIN_BG_SBB][32 * y + 4], se_mem[MAIN_BG_SBB][32 * y + 5], se_mem[MAIN_BG_SBB][32 * y + 6], se_mem[MAIN_BG_SBB][32 * y + 7], se_mem[MAIN_BG_SBB][32 * y + 8], se_mem[MAIN_BG_SBB][32 * (y - 1) + 9]};
                    y -= 1;
                    memcpy(&se_mem[MAIN_BG_SBB][32 * y], &tile_map4, sizeof(tile_map4));

                    y = 3;
                    const unsigned short tile_map5[10] = {se_mem[MAIN_BG_SBB][32 * y], se_mem[MAIN_BG_SBB][32 * y + 1], se_mem[MAIN_BG_SBB][32 * y + 2], se_mem[MAIN_BG_SBB][32 * y + 3], se_mem[MAIN_BG_SBB][32 * y + 4], se_mem[MAIN_BG_SBB][32 * y + 5], se_mem[MAIN_BG_SBB][32 * y + 6], se_mem[MAIN_BG_SBB][32 * y + 7], se_mem[MAIN_BG_SBB][32 * y + 8], se_mem[MAIN_BG_SBB][32 * (y - 1) + 9]};
                    y -= 1;
                    memcpy(&se_mem[MAIN_BG_SBB][32 * y], &tile_map5, sizeof(tile_map5));

                    y = 4;
                    const unsigned short tile_map6[10] = {se_mem[MAIN_BG_SBB][32 * y], se_mem[MAIN_BG_SBB][32 * y + 1], se_mem[MAIN_BG_SBB][32 * y + 2], se_mem[MAIN_BG_SBB][32 * y + 3], se_mem[MAIN_BG_SBB][32 * y + 4], se_mem[MAIN_BG_SBB][32 * y + 5], se_mem[MAIN_BG_SBB][32 * y + 6], se_mem[MAIN_BG_SBB][32 * y + 7], se_mem[MAIN_BG_SBB][32 * y + 8], se_mem[MAIN_BG_SBB][32 * (y - 1) + 9]};
                    y -= 1;
                    memcpy(&se_mem[MAIN_BG_SBB][32 * y], &tile_map6, sizeof(tile_map6));
                }
            }

            if (sequence_step == 6)
            {
                memset16(&pal_bg_mem[18], 0x1483, 1);
                state = 6;
                timer = 0;
                sequence_step = 0;
            }
            break;
        }
        case 6: // This state handles displaying the rewards earned from the completed round
        {
            if (timer < FRAMES(20)) break;

            int hand_y = 0;
            int interest_y = 0;

            if (hands > 0)
            {
                hand_y = 1;
            }

            // TODO: implement interest
            // if (interest > 0)
            // {
            //     interest_y = 1 + hand_y;
            // }

            if (sequence_step == 0) // Expand the black part of the panel down by one tile
            {
                int y = 14;

                const unsigned short tile_map1[17] = {se_mem[MAIN_BG_SBB][8 + 32 * (y - 1)], 0x002A, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x042A};
                memcpy(&se_mem[MAIN_BG_SBB][8 + 32 * (y - 1)], tile_map1, sizeof(tile_map1));

                const unsigned short tile_map2[17] = {se_mem[MAIN_BG_SBB][8 + 32 * y], 0x002A, 0x0055, 0x0056, 0x0056, 0x0056, 0x0056, 0x0056, 0x0056, 0x0056, 0x0056, 0x0056, 0x0056, 0x0056, 0x0056, 0x0455, 0x042A};
                memcpy(&se_mem[MAIN_BG_SBB][8 + 32 * y], tile_map2, sizeof(tile_map2));
            }
            else if (sequence_step < 15) // Use TTE to print '.' until the end of the panel width
            {
                // Print the separator dots
                int x = (9 + sequence_step) * 8;
                int y = (13) * 8;

                tte_printf("#{P:%d,%d; cx:0xF000}.", x, y); 
            }
            else if (sequence_step >= 30 && hand_reward > 0) // Wait an additional 15 frames since the last sequenced action
            {
                if (sequence_step == 30) // Expand the black part of the panel down by one tile again
                {
                    int y = 14 + hand_y; // This was intended for modularity so that I can add interest rewards later

                    const unsigned short tile_map1[17] = {se_mem[MAIN_BG_SBB][8 + 32 * (y - 1)], 0x002A, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x042A};
                    memcpy(&se_mem[MAIN_BG_SBB][8 + 32 * (y - 1)], tile_map1, sizeof(tile_map1));

                    const unsigned short tile_map2[17] = {se_mem[MAIN_BG_SBB][8 + 32 * y], 0x002A, 0x0055, 0x0056, 0x0056, 0x0056, 0x0056, 0x0056, 0x0056, 0x0056, 0x0056, 0x0056, 0x0056, 0x0056, 0x0056, 0x0455, 0x042A};
                    memcpy(&se_mem[MAIN_BG_SBB][8 + 32 * y], tile_map2, sizeof(tile_map2));

                    tte_printf("#{P:%d,%d; cx:0xD000}%d #{cx:0xF000}Hands", ROUND_END_NUM_HANDS_RECT.left, ROUND_END_NUM_HANDS_RECT.top, hand_reward); // Print the hand reward
                }
                else if (sequence_step > 45 && timer % FRAMES(20) == 0) // After 15 frames, every 20 frames, increment the hand reward text until the hand reward variable is depleted
                {
                    int y = (13 + hand_y) * 8;
                    hand_reward--;
                    tte_printf("#{P:%d, %d; cx:0xC000}$%d", HAND_REWARD_RECT.left, y, hands - hand_reward); // Print the hand reward
                }
            }

            sequence_step++;

            if (hand_reward <= 0 && interest_reward <= 0) // Once all rewards are accounted for go to the next state
            {
                sequence_step = 0;
                timer = 0; // Reset the timer
                state = 7; // Go to the next state
            }

            break;
        }
        case 7:
        {
            if (timer == FRAMES(40)) // Put the "cash out" button onto the round end panel
            {
                int y = 8;

                // 1st row
                const unsigned short tile_map1[14] = {0x002B, 0x002C, 0x002C, 0x002C, 0x002C, 0x002C, 0x002C, 0x002C, 0x002C, 0x002C, 0x002C, 0x002C, 0x002C, 0x042B};
                memcpy(&se_mem[MAIN_BG_SBB][10 + 32 * y], tile_map1, sizeof(tile_map1));

                // 2nd row
                y += 1;
                const unsigned short tile_map2[14] = {0x0030, 0x0031, 0x0031, 0x0031, 0x0031, 0x0031, 0x0031, 0x0031, 0x0031, 0x0031, 0x0031, 0x0031, 0x0031, 0x0430};
                memcpy(&se_mem[MAIN_BG_SBB][10 + 32 * y], tile_map2, sizeof(tile_map2));

                // 3rd row
                y += 1;
                unsigned short tile_map3[14] = {0x0037, 0x0038, 0x0038, 0x0038, 0x0038, 0x0038, 0x0038, 0x0038, 0x0038, 0x0038, 0x0038, 0x0038, 0x0038, 0x0437};
                memcpy(&se_mem[MAIN_BG_SBB][10 + 32 * y], tile_map3, sizeof(tile_map3));

                tte_printf("#{P:%d, %d; cx:0xF000}Cash Out: $%d", CASHOUT_RECT.left, CASHOUT_RECT.top, hands + blind_get_reward(current_blind)); // Print the cash out amount
            }
            else if (timer > FRAMES(40) && key_hit(KEY_A)) // Wait until the player presses A to cash out
            {   
                state = 8; // Go to the next state
                money += hands + blind_get_reward(current_blind); // Reward the player
                set_money(money);
                memset16(&pal_bg_mem[6], 0x0174, 1);

                obj_hide(round_end_blind_token->obj); // Hide the blind token object
                tte_erase_rect_wrapper(BLIND_TOKEN_TEXT_RECT); // Erase the blind token text
            }

            break;
        }
        case 8: // Shift the round end panel back out of view and go to the next state
        {
            sequence_step++;
            main_bg_se_move_rect_1_tile_vert(POP_MENU_ANIM_RECT, SE_DOWN);

            if (sequence_step >= 20)
            {
                sequence_step = 0;
                timer = 0; 
                state = 9;
            }
            break;
        }   
        default:
            sequence_step = 0;
            state = 0;
            blind_reward = 0;
            hand_reward = 0;
            interest_reward = 0;
            game_round_end_cleanup();
            game_set_state(GAME_SHOP);
            break;
    }
}

void game_shop()
{
    change_background(BG_ID_SHOP);
    
    // TODO: Later move these static variables somewhere else so they can be reused for each game state
    static int state = 0;

    // these are for controlling the shop menu
    static bool top_row = true;
    static ushort selection_x = 0;

    // temp variables for future implementation
    const ushort max_items_top = 0; 
    const ushort max_items_bottom = 0;

    if (timer % 20 == 0) // Shift palette around the border of the shop icon
    {
        unsigned short shifted_palette[4];
        memcpy16(&shifted_palette[0], &pal_bg_mem[14], 1);
        memcpy16(&shifted_palette[1], &pal_bg_mem[17], 1);
        memcpy16(&shifted_palette[2], &pal_bg_mem[22], 1);
        memcpy16(&shifted_palette[3], &pal_bg_mem[8], 1);

        // Circularly shift the palette
        int last = shifted_palette[3];

        for (int i = 3; i > 0; --i) {
            shifted_palette[i] = shifted_palette[i - 1];
        }

        shifted_palette[0] = last;

        memcpy16(&pal_bg_mem[14], &shifted_palette[0], 1); // Copy the shifted palette to the next 4 slots
        memcpy16(&pal_bg_mem[17], &shifted_palette[1], 1);
        memcpy16(&pal_bg_mem[22], &shifted_palette[2], 1);
        memcpy16(&pal_bg_mem[8], &shifted_palette[3], 1);
    }

    switch (state) // I'm only using magic numbers here for the sake of simplicity since it's just sequential, but you can replace them with named constants or enums if it makes it clearer
    {
        case 0: // Intro sequence (menu and shop icon coming into frame)
        {           
            main_bg_se_copy_rect_1_tile_vert(POP_MENU_ANIM_RECT, SE_UP);

            if (timer >= 7) // Shift the shop icon
            {
                int timer_offset = timer - 6;

                // TODO: Extract to generic function?
                for (int y = 0; y < timer_offset; y++)
                {
                    int y_from = 26 + y - timer_offset;
                    int y_to = 0 + y;
                    memcpy16(&se_mem[MAIN_BG_SBB][32 * y_to], &se_mem[MAIN_BG_SBB][32 * y_from], 9);
                }
            }

            if (timer == 12)
            {
                state = 1;
                timer = 0; // Reset the timer
            }

            break;
        }    
        case 1: // Shop menu input and selection
        {
            // Shop input logic
            if (key_hit(KEY_UP))
            {
                top_row = true;

                if (selection_x > max_items_top)
                {
                    selection_x = max_items_top;
                }
            }
            else if (key_hit(KEY_DOWN))
            {
                top_row = false;

                if (selection_x > max_items_bottom)
                {
                    selection_x = max_items_bottom;
                }
            }
            else if (key_hit(KEY_LEFT))
            {
                if (selection_x > 0)
                {
                    selection_x--;
                }
            }
            else if (key_hit(KEY_RIGHT))
            {
                if (top_row && selection_x < max_items_top)
                {
                    selection_x++;
                }
                else if (!top_row && selection_x < max_items_bottom)
                {
                    selection_x++;
                }
            }

            // Shop selection logic
            if (selection_x == 0 && top_row)
            {
                memcpy16(&pal_bg_mem[7], &pal_bg_mem[3], 1);
                memset16(&pal_bg_mem[5], 0xFFFF, 1);

                if (key_hit(KEY_A))
                {
                    // Go to next blind selection game state
                    state = 2; // Go to the outro sequence state
                    timer = 0; // Reset the timer

                    memcpy16(&pal_bg_mem[5], &pal_bg_mem[6], 1);

                    // memcpy16(&pal_bg_mem[16], &pal_bg_mem[6], 1); 
                    // This changes the color of the button to a dark red.
                    // However, it shares a palette with the shop icon, so it will change the color of the shop icon as well.
                    // And I don't care enough to fix it right now.
                }
            }
            else if (selection_x == 0 && !top_row)
            {
                memset16(&pal_bg_mem[7], 0xFFFF, 1);
                memcpy16(&pal_bg_mem[5], &pal_bg_mem[16], 1);

                if (key_hit(KEY_A))
                {
                    // Do shop re-roll for money
                }
            }

            break;
        }
        case 2: // Outro sequence (menu and shop icon going out of frame)
        {
            // Shift the shop panel
            main_bg_se_move_rect_1_tile_vert(POP_MENU_ANIM_RECT, SE_DOWN);

            main_bg_se_copy_rect_1_tile_vert(TOP_LEFT_PANEL_ANIM_RECT, SE_UP);
            
            if (timer == 1)
            {
                int y = 6;
                memset16(&se_mem[MAIN_BG_SBB][32 * (y - 1)], 0x0006, 1);
                memset16(&se_mem[MAIN_BG_SBB][1 + 32 * (y - 1)], 0x0007, 2);
                memset16(&se_mem[MAIN_BG_SBB][3 + 32 * (y - 1)], 0x0008, 1);
                memset16(&se_mem[MAIN_BG_SBB][4 + 32 * (y - 1)], 0x0009, 4);
                memset16(&se_mem[MAIN_BG_SBB][7 + 32 * (y - 1)], 0x000A, 1);
                memset16(&se_mem[MAIN_BG_SBB][8 + 32 * (y - 1)], 0x0406, 1); 
            }
            else if (timer == 2)
            {
                int y = 5;
                memset16(&se_mem[MAIN_BG_SBB][32 * (y - 1)], 0x0001, 1);
                memset16(&se_mem[MAIN_BG_SBB][1 + 32 * (y - 1)], 0x0002, 7);
                memset16(&se_mem[MAIN_BG_SBB][8 + 32 * (y - 1)], 0x0401, 1); 
            }

            if (timer >= MENU_POP_OUT_ANIM_FRAMES)
            {
                state = 3; // Go to the next state
                timer = 0; // Reset the timer
            }

            break;
        }
        default:
            state = 0; // Reset the state

            selection_x = 0; // Reset the selection
            top_row = true; // Reset the top row selection

            increment_blind(BLIND_DEFEATED);
            game_set_state(GAME_BLIND_SELECT); // If we reach here, we should go to the blind select state

            break;
    }
}

void game_blind_select()
{
    change_background(BG_ID_BLIND_SELECT);

    static int state = 0;

    static bool top_row = true; // There's only one row in this game state, but this is here for consistency with the shop state if we make these variables global or something

    switch (state) // I'm only using magic numbers here for the sake of simplicity since it's just sequential, but you can replace them with named constants or enums if it makes it clearer
    {
        case 0: // Intro sequence (menu coming into frame)
        {           
            main_bg_se_copy_rect_1_tile_vert(POP_MENU_ANIM_RECT, SE_UP);

            for (int i = 0; i < MAX_BLINDS; i++)
            {
                sprite_position(blind_select_tokens[i], blind_select_tokens[i]->pos.x, blind_select_tokens[i]->pos.y - 8);
            }

            if (timer == 12)
            {
                state = 1;
                timer = 0; // Reset the timer
            }

            break;
        }
        case 1: // Blind select input and selection
        {
            // Blind select input logic
            if (key_hit(KEY_UP))
            {
                top_row = true;
            }
            else if (key_hit(KEY_DOWN))
            {
                top_row = false;
            }
            else if (key_hit(KEY_A))
            {
                if (top_row)
                {
                    state = 2;
                    timer = 0;
                }
                else if (current_blind != BOSS_BLIND)
                {
                    increment_blind(BLIND_SKIPPED);
                    
                    background = -1; // Force refresh of the background
                    change_background(BG_ID_BLIND_SELECT);

                    // TODO: Create a generic vertical move by any number of tiles to avoid for loops?
                    for (int i = 0; i < 12; i++)
                    {
                        main_bg_se_copy_rect_1_tile_vert(POP_MENU_ANIM_RECT, SE_UP);
                    }

                    for (int i = 0; i < MAX_BLINDS; i++)
                    {
                        sprite_position(blind_select_tokens[i], blind_select_tokens[i]->pos.x, blind_select_tokens[i]->pos.y - (TILE_SIZE * 12));
                    }

                    timer = 0;
                }
            }

            if (top_row)
            {
                memset16(&pal_bg_mem[18], 0xFFFF, 1);
                memcpy16(&pal_bg_mem[10], &pal_bg_mem[5], 1);
            }
            else
            {
                memcpy16(&pal_bg_mem[18], &pal_bg_mem[15], 1);
                memset16(&pal_bg_mem[10], 0xFFFF, 1);
            }

            break;
        }
        case 2: // Blind selected, perform menu popout animation
        {
            if (timer < 15)
            {
                Rect blinds_rect = POP_MENU_ANIM_RECT;
                blinds_rect.top -= 1; // Because of the raised blind
                main_bg_se_move_rect_1_tile_vert(blinds_rect, SE_DOWN);

                for (int i = 0; i < MAX_BLINDS; i++)
                {
                    sprite_position(blind_select_tokens[i], blind_select_tokens[i]->pos.x, blind_select_tokens[i]->pos.y + TILE_SIZE);
                }
            }
            // TODO: Currently selecting other blinds crashes, remove this condition once fixed
            else if (current_blind == SMALL_BLIND && timer >= MENU_POP_OUT_ANIM_FRAMES)
            {
                for (int i = 0; i < MAX_BLINDS; i++)
                {
                    obj_hide(blind_select_tokens[i]->obj);
                }

                game_set_state(GAME_PLAYING);
                state = 0; // Reset the state
            }
            break;
        }
        default:
            break;
    }
}

void game_update()
{
    timer++;

    switch (game_state)
    {
        case GAME_PLAYING:
            game_playing();
            break;
        case GAME_ROUND_END:
            game_round_end();
            break;
        case GAME_SHOP:
            game_shop();
            break;
        case GAME_BLIND_SELECT:
            game_blind_select();
            break;
        case GAME_LOSE:
            // Handle lose logic here
            break;
    }
}