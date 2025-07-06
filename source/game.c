#include "game.h"

#include <maxmod.h>
#include <tonc.h>
#include <string.h>
#include <stdlib.h>

#include "util.h"
#include "sprite.h"
#include "card.h"
#include "blind.h"

#include "background_gfx.h"
#include "background_play_gfx.h"
#include "background_round_end_gfx.h"
#include "background_shop_gfx.h"

#include "soundbank.h"

static int timer = 1; // This might already exist in libtonc but idk so i'm just making my own
static int game_speed = 1;
static int background = 0;

static enum GameState game_state = GAME_PLAYING; // The current game state, this is used to determine what the game is doing at any given time
static enum HandState hand_state = HAND_DRAW;
static enum PlayState play_state = PLAY_PLAYING;

static enum HandType hand_type = NONE;

static Sprite *playing_blind_token = NULL; // The sprite that displays the blind when in "GAME_PLAYING/GAME_ROUND_END" state
static Sprite *round_end_blind_token = NULL; // The sprite that displays the blind when in "GAME_ROUND_END" state

static enum BlindType current_blind = SMALL_BLIND;

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
    // Idk if this is how Balatro does it but this is how I'm doing it
    if (hand_selections == 0 || hand_state == HAND_DISCARD)
    {
        hand_type = NONE;
        return hand_type;
    }

    hand_type = HIGH_CARD;

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
            hand_type = FLUSH;
            break;
        }
    }

    // Check for straight
    for (int i = 0; i < 9; i++)
    {
        if (ranks[i] && ranks[i + 1] && ranks[i + 2] && ranks[i + 3] && ranks[i + 4])
        {
            if (hand_type == FLUSH)
            {
                hand_type = STRAIGHT_FLUSH;
            }
            else
            {
                hand_type = STRAIGHT;
            }
            break;
        }
    }

    // Check for ace low straight
    if (ranks[ACE] && ranks[TWO] && ranks[THREE] && ranks[FOUR] && ranks[FIVE])
    {
        hand_type = STRAIGHT;
    }

    // Check for royal flush
    if (hand_type == STRAIGHT_FLUSH && ranks[TEN] && ranks[JACK] && ranks[QUEEN] && ranks[KING] && ranks[ACE])
    {
        hand_type = ROYAL_FLUSH;
        return hand_type;
    }

    // Check for straight flush
    if (hand_type == STRAIGHT_FLUSH)
    {
        hand_type = STRAIGHT_FLUSH;
        return hand_type;
    }

    // Check for flush five and five of a kind
    if (hand_type == FLUSH && ranks[FIVE] >= 5)
    {
        hand_type = FLUSH_FIVE;
        return hand_type;
    }
    else if (ranks[FIVE] >= 5)
    {
        return FIVE_OF_A_KIND;
    }
    
    // Check for for of a kind
    for (int i = 0; i < NUM_RANKS; i++)
    {
        if (ranks[i] >= 4)
        {
            hand_type = FOUR_OF_A_KIND;
            return hand_type;
        }
        else if (ranks[i] == 3)
        {   
            if (hand_type == PAIR)
            {
                hand_type = FULL_HOUSE;
                return hand_type;
            }
            else
            {
                hand_type = THREE_OF_A_KIND;
            }
        }
        else if (ranks[i] == 2)
        {
            if (hand_type == THREE_OF_A_KIND)
            {
                hand_type = FULL_HOUSE;
                return hand_type;
            }
            else if (hand_type == PAIR)
            {
                hand_type = TWO_PAIR;
                return hand_type;
            }
            else
            {
                hand_type = PAIR;
            }
        }
    }

    return hand_type;
}

void change_background(int id)
{
    if (background == id)
    {
        return;
    }
    else if (id == BG_ID_CARD_SELECTING)
    {
        memcpy(&se_mem[31], background_gfxMap, background_gfxMapLen);

        tte_erase_rect(128, 152, 152, 160);
    }
    else if (id == BG_ID_CARD_PLAYING)
    {
        memcpy(&se_mem[31], background_play_gfxMap, background_play_gfxMapLen);

        tte_erase_rect(128, 128, 152, 136);
    }
    else if (id == BG_ID_ROUND_END)
    {
        REG_DISPCNT &= ~DCNT_WIN0; // Disable window 0 so it doesn't make the cashout menu transparent

        memcpy(pal_bg_mem, background_round_end_gfxPal, 64);
        memcpy(&tile_mem[1], background_round_end_gfxTiles, background_round_end_gfxTilesLen);
        memcpy(&se_mem[31], background_round_end_gfxMap, background_round_end_gfxMapLen);

        // 1024 0x0400 is when sprites are flipped horizontally, 2048 0x0800 is when they are flipped vertically, 3072 0x0C00 is when they are flipped both horizontally and vertically

        // Incoming hack! this just clears the cash out menu thing so that we can slowly display it with an animation later. The reason this isn't optimal is because the full background is already loaded into the vram at this point.
        // I'm just doing it this way because it's easier than doing some weird shit with Grit in order to get a proper tilemap. I'm not the biggest fan of Grit.
        for (int y = 19; y >= 7; y--)
        {
            const unsigned short tile_map[17] = {se_mem[31][8 + 32 * y], 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000};
            memcpy(&se_mem[31][8 + 32 * y], tile_map, sizeof(tile_map));
        }
        
        //tte_erase_rect(0, 0, 64, 48); // Clear top left corner where the blind stats are displayed
        tte_erase_rect(128, 128, 152, 160); // Clear the hand size/max size display

        //tte_printf("#{P:88,72; cx:0xF000}Cash Out: $5"); // Hardcoded example
    }
    else if (id == BG_ID_SHOP)
    {
        REG_DISPCNT &= ~DCNT_WIN0;

        memcpy(pal_bg_mem, background_shop_gfxPal, 64);
        GRIT_CPY(&tile_mem[1], background_shop_gfxTiles);
        GRIT_CPY(&se_mem[31], background_shop_gfxMap);

        // Set the outline colors for the shop background
        memset16(&pal_bg_mem[27], 0x213D, 1);
        memset16(&pal_bg_mem[7], 0x10B4, 1);

        memset16(&pal_bg_mem[14], 0x32BE, 1); // Reset the shop lights to correct colors
        memset16(&pal_bg_mem[17], 0x4B5F, 1);
        memset16(&pal_bg_mem[22], 0x5F9F, 1);
        memset16(&pal_bg_mem[8], 0xFFFF, 1);

        memcpy16(&pal_bg_mem[6], &pal_bg_mem[3], 1); // Disable the button highlight colors
        memcpy16(&pal_bg_mem[13], &pal_bg_mem[15], 1);
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
    tte_erase_rect(8, 64, 64, 72);
    tte_printf("#{P:%d,64; cx:0xF000}%d", x_offset, value);
}

void set_score(int value)
{
    int x_offset = 32;
    tte_printf("#{P:%d,48; cx:0xF000}%d", x_offset, value);
}

void set_money(int value)
{
    int x_offset = 32 - get_digits_odd(value) * 8;
    tte_erase_rect(8, 120, 64, 120);
    tte_printf("#{P:%d,120; cx:0xC000}$%d", x_offset, value);
}

void set_chips(int value)
{
    int x_offset = 32 - get_digits(value) * 8; // Adjust the x offset based on the number of digits in chips
    tte_erase_rect(8, 80, 32, 88);
    tte_printf("#{P:%d,80; cx:0xF000;}%d", x_offset, value);
}

void set_mult(int value)
{
    tte_erase_rect(40, 80, 64, 88);
    tte_printf("#{P:40,80; cx:0xF000;}%d", value); // Mult
}

void set_hand()
{
    tte_erase_rect(8, 64, 64, 72); // Hand type

    switch (hand_get_type())
    {
    case HIGH_CARD:
        tte_printf("#{P:8,64;}HIGH C");
        chips = 5;
        mult = 1;
        break;
    case PAIR:
        tte_printf("#{P:8,64;}PAIR");
        chips = 10;
        mult = 2;
        break;
    case TWO_PAIR:
        tte_printf("#{P:8,64;}2 PAIR");
        chips = 20;
        mult = 2;
        break;
    case THREE_OF_A_KIND:
        tte_printf("#{P:8,64;}3 OAK");
        chips = 30;
        mult = 3;
        break;
    case STRAIGHT:
        tte_printf("#{P:8,64;}STRT");
        chips = 30;
        mult = 4;
        break;
    case FLUSH:
        tte_printf("#{P:8,64;}FLUSH");
        chips = 35;
        mult = 4;
        break;
    case FULL_HOUSE:
        tte_printf("#{P:8,64;}FULL H");
        chips = 40;
        mult = 4;
        break;
    case FOUR_OF_A_KIND:
        tte_printf("#{P:8,64;}4 OAK");
        chips = 60;
        mult = 7;
        break;
    case STRAIGHT_FLUSH:
        tte_printf("#{P:8,64;}STRT F");
        chips = 100;
        mult = 8;
        break;
    case ROYAL_FLUSH:
        tte_printf("#{P:8,64;}ROYAL F");
        chips = 100;
        mult = 8;
        break;
    case FIVE_OF_A_KIND:
        tte_printf("#{P:8,64;}5 OAK");
        chips = 120;
        mult = 12;
        break;
    case FLUSH_HOUSE:
        tte_printf("#{P:8,64;}FLUSH H");
        chips = 140;
        mult = 14;
        break;
    case FLUSH_FIVE:
        tte_printf("#{P:8,64;}FLUSH 5");
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

// Game functions
void game_init()
{
    playing_blind_token = blind_token_new(current_blind, 8, 18, 33); // Create the blind token sprite at the top left corner
    round_end_blind_token = blind_token_new(current_blind, 82, 86, 34); // Create the blind token sprite for round end
    obj_hide(round_end_blind_token->obj); // Hide the blind token sprite for now

    // Fill the deck with all the cards. Later on this can be replaced with a more dynamic system that allows for different decks and card types.
    for (int suit = 0; suit < NUM_SUITS; suit++)
    {
        for (int rank = 0; rank < NUM_RANKS; rank++)
        {
            Card *card = card_new(suit, rank);
            deck_push(card);
        }
    }

    // Shuffle the deck
    for (int i = 0; i < MAX_DECK_SIZE; i++)
    {
        int j = rand() % MAX_DECK_SIZE;
        Card *temp = deck[i];
        deck[i] = deck[j];
        deck[j] = temp;
    }

    change_background(BG_ID_CARD_SELECTING);

    tte_printf("#{P:128,128; cx:0xF000}%d/%d", hand_get_size(), hand_get_max_size()); // Hand size/max size
    tte_printf("#{P:200,152; cx:0xF000}%d/%d", deck_get_size(), deck_get_max_size()); // Deck size/max size

    tte_printf("#{P:40,24; cx:0xE000}%d", blind_get_requirement(current_blind, ante)); // Blind requirement
    tte_printf("#{P:40,32; cx:0xC000}$%d", blind_get_reward(current_blind)); // Blind reward

    set_score(score); // Set the score display

    set_chips(chips); // Set the chips display
    set_mult(mult); // Set the multiplier display

    tte_printf("#{P:16,104; cx:0xD000}%d", hands); // Hand
    tte_printf("#{P:48,104; cx:0xE000}%d", discards); // Discard

    //tte_printf("#{P:24,120; cx:0xC000}$%d", money); // Money
    set_money(money); // Set the money display

    tte_printf("#{P:48,144; cx:0xC000}%d", 1); // Round
    tte_printf("#{P:8,144; cx:0xC000}%d#{cx:0xF000}/%d", ante, MAX_ANTE); // Ante
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
            tte_printf("#{P:48,104; cx:0xE000}%d", discards);
        }

        if (key_hit(KEY_START) && hands > 0 && hand_play())
        {
            hands--;
            tte_printf("#{P:16,104; cx:0xD000}%d", hands);
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
                tte_erase_rect(8, 64, 64, 72);
            }
        }
        else
        {
            score += temp_score;
            temp_score = 0;
            lerped_temp_score = 0;
            lerped_score = 0;

            tte_erase_rect(8, 64, 64, 72); // Just erase the temp score

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
        timer = 1;
    }
}

static void game_playing_discarded_cards_loop()
{
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
            game_state = GAME_ROUND_END; // Set the game state back to playing
            timer = 1; // Reset the timer
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
                            timer = 1;

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
                    timer = 1;
                    break;
                }

                break;
            case HAND_PLAY:
                hand_x = hand_x + (int2fx(i) - int2fx(hand_top) / 2) * -spacing_lut[hand_top];
                hand_y += (24 << FIX_SHIFT);

                if (hand[i]->selected && timer % FRAMES(10) == 0)
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

                    timer = 1;
                }

                if (i == 0 && timer % FRAMES(10) == 0)
                {
                    hand_state = HAND_PLAYING;
                    cards_drawn = 0;
                    hand_selections = 0;
                    timer = 1;
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
                            timer = 1;
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
                                    tte_erase_rect(72, 48, 240, 56);
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
                                tte_erase_rect(72, 48, 240, 56);
                                play_state = PLAY_ENDING;
                                timer = 1;
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
                            timer = 1;
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
                                timer = 1;
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
            tte_printf("#{P:128,128; cx:0xF000}%d/%d", hand_get_size(), hand_get_max_size()); // Hand size/max size
        }
        else if (background == BG_ID_CARD_PLAYING)
        {
            tte_printf("#{P:128,152; cx:0xF000}%d/%d", hand_get_size(), hand_get_max_size()); // Hand size/max size
        }

        tte_printf("#{P:200,152; cx:0xF000}%d/%d", deck_get_size(), deck_get_max_size()); // Deck size/max size

        last_hand_size = hand_get_size();
        last_deck_size = deck_get_size();
    }
}

void game_playing()
{
    if (hand_state == HAND_SELECT && hands == 0) 
    {
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

    // Discarded cards loop (mainly for shuffling)
    game_playing_discarded_cards_loop();

    static int played_selections = 0;
    static bool sound_played = false;
    bool discarded_card = false;

    cards_in_hand_update_loop(&discarded_card, &played_selections, &sound_played);
	played_cards_update_loop(&discarded_card, &played_selections, &sound_played);
    
    game_playing_ui_text_update();
}

void game_round_end() // Writing this kind a made me want to kms. If somewone wants to rewrite this, please do so.
{
    static int state = 0;
    static int sequence_step = 0; // Reusable variable for the animations in states

    static int blind_reward = 0;
    static int hand_reward = 0;
    static int interest_reward = 0; 

    switch (state)
    {
        case 0:
        {
            if (timer == 30)
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
        case 1:
        {
            const int bottom_of_screen = 19;
            int y = bottom_of_screen - timer;

            // 1st row
            const unsigned short tile_map1[17] = {se_mem[31][8 + 32 * y], 0x0026, 0x0027, 0x0027, 0x0027, 0x0027, 0x0027, 0x0027, 0x0027, 0x0027, 0x0027, 0x0027, 0x0027, 0x0027, 0x0027, 0x0027, 0x0426};
            memcpy(&se_mem[31][8 + 32 * y], tile_map1, sizeof(tile_map1));

            // 2nd row
            y += 1;
            if (y > bottom_of_screen) break;
            const unsigned short tile_map2[17] = {se_mem[31][8 + 32 * y], 0x002A, 0x042D, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x002D, 0x042A};
            memcpy(&se_mem[31][8 + 32 * y], tile_map2, sizeof(tile_map2));

            // 3rd row
            y += 1;
            if (y > bottom_of_screen) break;
            unsigned short tile_map3[17] = {se_mem[31][8 + 32 * y], 0x002A, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x042A};
            memcpy(&se_mem[31][8 + 32 * y], tile_map3, sizeof(tile_map3));

            // 4th row
            y += 1;
            if (y > bottom_of_screen) break;
            tile_map3[0] = se_mem[31][8 + 32 * y];
            memcpy(&se_mem[31][8 + 32 * y], tile_map3, sizeof(tile_map3));

            // 5th row
            y += 1;
            if (y > bottom_of_screen) break;
            tile_map3[0] = se_mem[31][8 + 32 * y];
            memcpy(&se_mem[31][8 + 32 * y], tile_map3, sizeof(tile_map3));

            // 6th row
            y += 1;
            if (y > bottom_of_screen) break;
            const unsigned short tile_map4[17] = {se_mem[31][8 + 32 * y], 0x002A, 0x0055, 0x0056, 0x0056, 0x0056, 0x0056, 0x0056, 0x0056, 0x0056, 0x0056, 0x0056, 0x0056, 0x0056, 0x0056, 0x0455, 0x042A};
            memcpy(&se_mem[31][8 + 32 * y], tile_map4, sizeof(tile_map4));

            // 7th row
            y += 1;
            if (y > bottom_of_screen) break;
            const unsigned short tile_map5[17] = {se_mem[31][8 + 32 * y], 0x002A, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x042A};
            memcpy(&se_mem[31][8 + 32 * y], tile_map5, sizeof(tile_map5));

            if (timer >= 12)
            {
                state = 2;
                timer = 0;
            }
            break;
        }
        case 2:
        {
            obj_unhide(round_end_blind_token->obj, 0);
            tte_printf("#{P:112,96; cx:0xE000}%d", blind_get_requirement(current_blind, ante));

            int y = 13;

            const unsigned short tile_map1[17] = {se_mem[31][8 + 32 * (y - 1)], 0x002A, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x042A};
            memcpy(&se_mem[31][8 + 32 * (y - 1)], tile_map1, sizeof(tile_map1));

            const unsigned short tile_map2[17] = {se_mem[31][8 + 32 * y], 0x002A, 0x0055, 0x0056, 0x0056, 0x0056, 0x0056, 0x0056, 0x0056, 0x0056, 0x0056, 0x0056, 0x0056, 0x0056, 0x0056, 0x0455, 0x042A};
            memcpy(&se_mem[31][8 + 32 * y], tile_map2, sizeof(tile_map2));

            if (timer >= 30)
            {
                state = 3;
                timer = 0;
            }
            break;
        }
        case 3:
        {
            // "Score Min" text
            const unsigned short tile_map3[4] = {0x003E, 0x003F, 0x0040, 0x0041};

            int temp_offset = timer;
            memcpy(&se_mem[31][12 + temp_offset + 32 * 11], &tile_map3[temp_offset - 1], sizeof(tile_map3[0]));

            if (timer >= 4)
            {
                state = 4;
                timer = 0;
            }
            break;
        }
        case 4:
        {
            if (timer % FRAMES(20) != 0) break;

            // TODO: Add sound effect here

            sequence_step++;

            blind_reward--;
            tte_printf("#{P:40,32; cx:0xC000}$%d", blind_reward);
            tte_printf("#{P:168, 96; cx:0xC000}$%d", blind_get_reward(current_blind) - blind_reward);

            if (blind_reward <= 0)
            {
                tte_erase_rect(40, 32, 64, 40);
                obj_hide(playing_blind_token->obj);
                state = 5;
                sequence_step = 0;
                timer = 0;
            }
            break;
        }
        case 5:
        {
            if (timer < FRAMES(20)) break;

            sequence_step++;
            {
                int y = 5;

                if (sequence_step == 1)
                {
                    tte_erase_rect(40, 16, 64, 40);
                    obj_hide(playing_blind_token->obj);

                    const unsigned short tile_map1[10] = {0x0064, 0x0065, 0x0065, 0x0066, 0x0067, 0x0067, 0x0067, 0x0068, 0x0464, se_mem[31][32 * y + 9]};
                    memcpy(&se_mem[31][32 * y], &tile_map1, sizeof(tile_map1));

                    y -= 1;
                    const unsigned short tile_map2[10] = {0x006F, 0x0070, 0x0070, 0x0070, 0x0070, 0x0070, 0x0070, 0x0070, 0x046F, se_mem[31][32 * y + 9]};
                    memcpy(&se_mem[31][32 * y], &tile_map2, sizeof(tile_map2));

                    y = 1;
                    const unsigned short tile_map3[10] = {se_mem[31][32 * y], se_mem[31][32 * y + 1], se_mem[31][32 * y + 2], se_mem[31][32 * y + 3], se_mem[31][32 * y + 4], se_mem[31][32 * y + 5], se_mem[31][32 * y + 6], se_mem[31][32 * y + 7], se_mem[31][32 * y + 8], se_mem[31][32 * (y - 1) + 9]};
                    y -= 1;
                    memcpy(&se_mem[31][32 * y], &tile_map3, sizeof(tile_map3));

                    y = 2;
                    const unsigned short tile_map4[10] = {se_mem[31][32 * y], se_mem[31][32 * y + 1], se_mem[31][32 * y + 2], se_mem[31][32 * y + 3], se_mem[31][32 * y + 4], se_mem[31][32 * y + 5], se_mem[31][32 * y + 6], se_mem[31][32 * y + 7], se_mem[31][32 * y + 8], se_mem[31][32 * (y - 1) + 9]};
                    y -= 1;
                    memcpy(&se_mem[31][32 * y], &tile_map4, sizeof(tile_map4));

                    y = 3;
                    const unsigned short tile_map5[10] = {se_mem[31][32 * y], se_mem[31][32 * y + 1], se_mem[31][32 * y + 2], se_mem[31][32 * y + 3], se_mem[31][32 * y + 4], se_mem[31][32 * y + 5], se_mem[31][32 * y + 6], se_mem[31][32 * y + 7], se_mem[31][32 * y + 8], se_mem[31][32 * (y - 1) + 9]};
                    y -= 1;
                    memcpy(&se_mem[31][32 * y], &tile_map5, sizeof(tile_map5));
                }
                else if (sequence_step == 2)
                {
                    const unsigned short tile_map1[10] = {0x0064, 0x0065, 0x0065, 0x0066, 0x0067, 0x0067, 0x0067, 0x0068, 0x0464, se_mem[31][32 * y + 9]};
                    memcpy(&se_mem[31][32 * y], &tile_map1, sizeof(tile_map1));
                    
                    y = 1;
                    const unsigned short tile_map3[10] = {se_mem[31][32 * y], se_mem[31][32 * y + 1], se_mem[31][32 * y + 2], se_mem[31][32 * y + 3], se_mem[31][32 * y + 4], se_mem[31][32 * y + 5], se_mem[31][32 * y + 6], se_mem[31][32 * y + 7], se_mem[31][32 * y + 8], se_mem[31][32 * (y - 1) + 9]};
                    y -= 1;
                    memcpy(&se_mem[31][32 * y], &tile_map3, sizeof(tile_map3));
                    
                    y = 2;
                    const unsigned short tile_map4[10] = {se_mem[31][32 * y], se_mem[31][32 * y + 1], se_mem[31][32 * y + 2], se_mem[31][32 * y + 3], se_mem[31][32 * y + 4], se_mem[31][32 * y + 5], se_mem[31][32 * y + 6], se_mem[31][32 * y + 7], se_mem[31][32 * y + 8], se_mem[31][32 * (y - 1) + 9]};
                    y -= 1;
                    memcpy(&se_mem[31][32 * y], &tile_map4, sizeof(tile_map4));

                    y = 3;
                    const unsigned short tile_map5[10] = {se_mem[31][32 * y], se_mem[31][32 * y + 1], se_mem[31][32 * y + 2], se_mem[31][32 * y + 3], se_mem[31][32 * y + 4], se_mem[31][32 * y + 5], se_mem[31][32 * y + 6], se_mem[31][32 * y + 7], se_mem[31][32 * y + 8], se_mem[31][32 * (y - 1) + 9]};
                    y -= 1;
                    memcpy(&se_mem[31][32 * y], &tile_map5, sizeof(tile_map5));

                    y = 4;
                    const unsigned short tile_map6[10] = {se_mem[31][32 * y], se_mem[31][32 * y + 1], se_mem[31][32 * y + 2], se_mem[31][32 * y + 3], se_mem[31][32 * y + 4], se_mem[31][32 * y + 5], se_mem[31][32 * y + 6], se_mem[31][32 * y + 7], se_mem[31][32 * y + 8], se_mem[31][32 * (y - 1) + 9]};
                    y -= 1;
                    memcpy(&se_mem[31][32 * y], &tile_map6, sizeof(tile_map6));

                    y = 4;
                    const unsigned short tile_map2[10] = {0x046B, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x006B, se_mem[31][32 * y + 9]};
                    memcpy(&se_mem[31][32 * y], &tile_map2, sizeof(tile_map2));
                }
                else
                {
                    y = 1;
                    const unsigned short tile_map3[10] = {se_mem[31][32 * y], se_mem[31][32 * y + 1], se_mem[31][32 * y + 2], se_mem[31][32 * y + 3], se_mem[31][32 * y + 4], se_mem[31][32 * y + 5], se_mem[31][32 * y + 6], se_mem[31][32 * y + 7], se_mem[31][32 * y + 8], se_mem[31][32 * (y - 1) + 9]};
                    y -= 1;
                    memcpy(&se_mem[31][32 * y], &tile_map3, sizeof(tile_map3));
                    
                    y = 2;
                    const unsigned short tile_map4[10] = {se_mem[31][32 * y], se_mem[31][32 * y + 1], se_mem[31][32 * y + 2], se_mem[31][32 * y + 3], se_mem[31][32 * y + 4], se_mem[31][32 * y + 5], se_mem[31][32 * y + 6], se_mem[31][32 * y + 7], se_mem[31][32 * y + 8], se_mem[31][32 * (y - 1) + 9]};
                    y -= 1;
                    memcpy(&se_mem[31][32 * y], &tile_map4, sizeof(tile_map4));

                    y = 3;
                    const unsigned short tile_map5[10] = {se_mem[31][32 * y], se_mem[31][32 * y + 1], se_mem[31][32 * y + 2], se_mem[31][32 * y + 3], se_mem[31][32 * y + 4], se_mem[31][32 * y + 5], se_mem[31][32 * y + 6], se_mem[31][32 * y + 7], se_mem[31][32 * y + 8], se_mem[31][32 * (y - 1) + 9]};
                    y -= 1;
                    memcpy(&se_mem[31][32 * y], &tile_map5, sizeof(tile_map5));

                    y = 4;
                    const unsigned short tile_map6[10] = {se_mem[31][32 * y], se_mem[31][32 * y + 1], se_mem[31][32 * y + 2], se_mem[31][32 * y + 3], se_mem[31][32 * y + 4], se_mem[31][32 * y + 5], se_mem[31][32 * y + 6], se_mem[31][32 * y + 7], se_mem[31][32 * y + 8], se_mem[31][32 * (y - 1) + 9]};
                    y -= 1;
                    memcpy(&se_mem[31][32 * y], &tile_map6, sizeof(tile_map6));
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
        case 6: // TODO: implement interest
        {
            if (timer < FRAMES(20)) break;

            int hand_y = 0;
            int interest_y = 0;

            if (hands > 0)
            {
                hand_y = 1;
            }

            // if (interest > 0)
            // {
            //     interest_y = 1 + hand_y;
            // }

            if (sequence_step == 0)
            {
                int y = 14;

                const unsigned short tile_map1[17] = {se_mem[31][8 + 32 * (y - 1)], 0x002A, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x042A};
                memcpy(&se_mem[31][8 + 32 * (y - 1)], tile_map1, sizeof(tile_map1));

                const unsigned short tile_map2[17] = {se_mem[31][8 + 32 * y], 0x002A, 0x0055, 0x0056, 0x0056, 0x0056, 0x0056, 0x0056, 0x0056, 0x0056, 0x0056, 0x0056, 0x0056, 0x0056, 0x0056, 0x0455, 0x042A};
                memcpy(&se_mem[31][8 + 32 * y], tile_map2, sizeof(tile_map2));
            }
            else if (sequence_step < 15)
            {
                int x = (9 + sequence_step) * 8;
                int y = (13) * 8;

                tte_printf("#{P:%d,%d; cx:0xF000}.", x, y); 
            }
            else if (sequence_step >= 30 && hand_reward > 0)
            {
                if (sequence_step == 30)
                {
                    int y = 14 + hand_y;

                    const unsigned short tile_map1[17] = {se_mem[31][8 + 32 * (y - 1)], 0x002A, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x0014, 0x042A};
                    memcpy(&se_mem[31][8 + 32 * (y - 1)], tile_map1, sizeof(tile_map1));

                    const unsigned short tile_map2[17] = {se_mem[31][8 + 32 * y], 0x002A, 0x0055, 0x0056, 0x0056, 0x0056, 0x0056, 0x0056, 0x0056, 0x0056, 0x0056, 0x0056, 0x0056, 0x0056, 0x0056, 0x0455, 0x042A};
                    memcpy(&se_mem[31][8 + 32 * y], tile_map2, sizeof(tile_map2));

                    tte_printf("#{P:88,116; cx:0xD000}%d #{cx:0xF000}Hands", hand_reward); // Print the hand reward
                }
                else if (sequence_step > 45 && timer % FRAMES(20) == 0)
                {
                    int y = (13 + hand_y) * 8;
                    hand_reward--;
                    tte_printf("#{P:168, %d; cx:0xC000}$%d", y, hands - hand_reward); // Print the hand reward
                }
            }

            sequence_step++;

            if (hand_reward <= 0 && interest_reward <= 0)
            {
                sequence_step = 0;
                timer = 0; // Reset the timer
                state = 7; // Go to the next state
            }

            break;
        }
        case 7:
        {
            if (timer == FRAMES(40))
            {
                int y = 8;

                // 1st row
                const unsigned short tile_map1[14] = {0x002B, 0x002C, 0x002C, 0x002C, 0x002C, 0x002C, 0x002C, 0x002C, 0x002C, 0x002C, 0x002C, 0x002C, 0x002C, 0x042B};
                memcpy(&se_mem[31][10 + 32 * y], tile_map1, sizeof(tile_map1));

                // 2nd row
                y += 1;
                const unsigned short tile_map2[14] = {0x0030, 0x0031, 0x0031, 0x0031, 0x0031, 0x0031, 0x0031, 0x0031, 0x0031, 0x0031, 0x0031, 0x0031, 0x0031, 0x0430};
                memcpy(&se_mem[31][10 + 32 * y], tile_map2, sizeof(tile_map2));

                // 3rd row
                y += 1;
                unsigned short tile_map3[14] = {0x0037, 0x0038, 0x0038, 0x0038, 0x0038, 0x0038, 0x0038, 0x0038, 0x0038, 0x0038, 0x0038, 0x0038, 0x0038, 0x0437};
                memcpy(&se_mem[31][10 + 32 * y], tile_map3, sizeof(tile_map3));

                tte_printf("#{P:88, 72; cx:0xF000}Cash Out: $%d", hands + blind_get_reward(current_blind)); // Print the cash out amount
            }
            else if (timer > FRAMES(40) && key_hit(KEY_A))
            {   
                state = 8; // Go to the next state
                money += hands + blind_get_reward(current_blind); // Reward the player
                set_money(money);
                memset16(&pal_bg_mem[6], 0x0174, 1);

                obj_hide(round_end_blind_token->obj); // Hide the blind token object
                tte_erase_rect(80, 72, 200, 160); // Erase the blind token text
            }

            break;
        }
        case 8:
        {
            sequence_step++;
            int x = 9; 
            
            for (int y = 19; y > 5; y--)
            {
                memcpy16(&se_mem[31][x + 32 * (y + 1)], &se_mem[31][x + 32 * y], 16);
            }

            if (sequence_step >= 20)
            {
                sequence_step = 0;
                timer = 0; 
                state = 9;
            }
        }
            break;
        default:
            timer = 0;
            sequence_step = 0;
            state = 0;
            blind_reward = 0;
            hand_reward = 0;
            interest_reward = 0;
            game_state = GAME_SHOP;
            break;
    }
}


void game_shop()
{
    change_background(BG_ID_SHOP);

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

    if (timer < 13)
    {           
        for (int y = 7; y < 40; y++)
        {
            int x = 9;
            memcpy16(&se_mem[31][x + 32 * (y - 1)], &se_mem[31][x + 32 * y], 16);
        }

        if (timer >= 7)
        {
            int timer_offset = timer - 6;

            for (int y = 0; y < timer_offset; y++)
            {
                int y_from = 26 + y - timer_offset;
                int y_to = 0 + y;
                memcpy16(&se_mem[31][32 * y_to], &se_mem[31][32 * y_from], 9);
            }
        }
        
        // if (timer == 8)
        // {
        //     int y_from = 26;
        //     int y_to = 0;
        //     memcpy16(&se_mem[31][32 * y_to], &se_mem[31][32 * y_from], 9);
        // }
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
            // Handle blind select logic here
            break;
        case GAME_LOSE:
            // Handle lose logic here
            break;
    }
}