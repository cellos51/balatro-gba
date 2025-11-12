#include <tonc.h>

#include "joker.h"
#include "joker_gfx.h"
#include "graphic_utils.h"
#include "card.h"
#include "soundbank.h"
#include "util.h"

#include <maxmod.h>
#include <stdlib.h>
#include <string.h>

#include "pool.h"

#define JOKER_SCORE_TEXT_Y 48
#define HELD_CARD_SCORE_TEXT_Y 108
#define MAX_CARD_SCORE_STR_LEN 2
#define NUM_JOKERS_PER_SPRITESHEET 2

static const unsigned int *joker_gfxTiles[] =
{
#define DEF_JOKER_GFX(idx) joker_gfx##idx##Tiles,
#include "../include/def_joker_gfx_table.h"
#undef DEF_JOKER_GFX
};
static const unsigned short *joker_gfxPal[] = 
{
#define DEF_JOKER_GFX(idx) joker_gfx##idx##Pal,
#include "../include/def_joker_gfx_table.h"
#undef DEF_JOKER_GFX
};

const static u8 edition_price_lut[MAX_EDITIONS] =
{
    0, // BASE_EDITION
    2, // FOIL_EDITION
    3, // HOLO_EDITION
    5, // POLY_EDITION
    5, // NEGATIVE_EDITION
};

/* So for the card objects, I needed them to be properly sorted
   which is why they let you specify the layer index when creating a new card object.
   Since the cards would overlap a lot in your hand, If they weren't sorted properly, it would look like a mess.
   The joker objects are functionally identical to card objects, so they use the same logic.
   But I'm going to use a simpler approach for the joker objects
   since I'm lazy and sorting them wouldn't look good enough to warrant the effort.
*/
static bool used_layers[MAX_JOKER_OBJECTS] = {false}; // Track used layers for joker sprites
// TODO: Refactor sorting into SpriteObject?

// Maps the spritesheet index to the palette bank index allocated to it.
// Spritesheets that were not allocated are
static int joker_spritesheet_pb_map[(MAX_DEFINABLE_JOKERS + 1) / NUM_JOKERS_PER_SPRITESHEET];
static int joker_pb_num_sprite_users[JOKER_LAST_PB - JOKER_BASE_PB + 1] = { 0 };

static int get_num_spritesheets()
{
    return (get_joker_registry_size() + NUM_JOKERS_PER_SPRITESHEET - 1) / NUM_JOKERS_PER_SPRITESHEET;
}

static int joker_get_spritesheet_idx(u8 joker_id)

{
    return joker_id / NUM_JOKERS_PER_SPRITESHEET;
}

// TODO: This should be generalized so any sprite can have dynamic swapping
static void joker_pb_add_sprite_user(int pb)
{
    joker_pb_num_sprite_users[pb - JOKER_BASE_PB]++;
}

static void joker_pb_remove_sprite_user(int pb)
{
    int num_sprite_users = joker_pb_num_sprite_users[pb - JOKER_BASE_PB];
    joker_pb_num_sprite_users[pb - JOKER_BASE_PB] = max(0, num_sprite_users - 1);
}

static int joker_pb_get_num_sprite_users(int joker_pb)
{
    return joker_pb_num_sprite_users[joker_pb - JOKER_BASE_PB];
}

static int get_unused_joker_pb()
{
    for (int i = 0; i < NUM_ELEM_IN_ARR(joker_pb_num_sprite_users); i++)
    {
        if (joker_pb_num_sprite_users[i] == 0)
        {
            return (i + JOKER_BASE_PB);
        }
    }

    return UNDEFINED;
}

static int allocate_pb_if_needed(u8 joker_id)
{
    int joker_spritesheet_idx = joker_get_spritesheet_idx(joker_id);
    int joker_pb = joker_spritesheet_pb_map[joker_spritesheet_idx];
    if (joker_pb != UNDEFINED)
    {
        // Already allocated
        return joker_pb;
    }
    
    // Allocate a new palette
    joker_pb = get_unused_joker_pb();

    if (joker_pb == UNDEFINED)
    {
        // Ran out of palettes, default to base and pray
        joker_pb = JOKER_BASE_PB;
    }
    else
    {
        joker_spritesheet_pb_map[joker_spritesheet_idx] = joker_pb;
        memcpy16(&pal_obj_mem[PAL_ROW_LEN * joker_pb], joker_gfxPal[joker_spritesheet_idx], NUM_ELEM_IN_ARR(joker_gfx0Pal));
    }
    
    return joker_pb;
}

void joker_init()
{
    // This should init once only so no need to free
    int num_spritesheets = get_num_spritesheets();

    for (int i = 0; i < num_spritesheets; i++)
    {
        joker_spritesheet_pb_map[i] = UNDEFINED;
    }
}

Joker *joker_new(u8 id)
{
    if (id >= get_joker_registry_size()) return NULL;

    Joker *joker = POOL_GET(Joker);
    const JokerInfo *jinfo = get_joker_registry_entry(id);

    joker->id = id;
    joker->modifier = BASE_EDITION; // TODO: Make this a parameter
    joker->value = jinfo->base_value + edition_price_lut[joker->modifier];
    joker->rarity = jinfo->rarity;
    joker->data = 0;

    // initialize persistent Joker data if needed
    jinfo->joker_effect(joker, NULL, JOKER_EVENT_ON_JOKER_CREATED);

    return joker;
}

void joker_destroy(Joker **joker)
{
    POOL_FREE(Joker, *joker);
    *joker = NULL;
}

JokerEffect joker_get_score_effect(Joker *joker, Card *scored_card, enum JokerEvent joker_event)
{
    const JokerInfo *jinfo = get_joker_registry_entry(joker->id);
    if (!jinfo) return (JokerEffect){0};

    return jinfo->joker_effect(joker, scored_card, joker_event);
}

int joker_get_sell_value(const Joker* joker)
{
    if (joker == NULL)
    {
        return UNDEFINED;
    }

    return joker->value/2;
}

// JokerObject methods
JokerObject *joker_object_new(Joker *joker)
{
    JokerObject *joker_object = POOL_GET(JokerObject);

    int layer = 0;
    for (int i = 0; i < MAX_JOKER_OBJECTS; i++)
    {
        if (!used_layers[i])
        {
            layer = i;
            used_layers[i] = true; // Mark this layer as used
            break;
        }
    }

    joker_object->joker = joker;
    joker_object->sprite_object = sprite_object_new();

    int tile_index = JOKER_TID + (layer * JOKER_SPRITE_OFFSET);
    
    int joker_spritesheet_idx = joker_get_spritesheet_idx(joker->id);
    int joker_idx = joker->id % NUM_JOKERS_PER_SPRITESHEET;
    int joker_pb = allocate_pb_if_needed(joker->id);
    joker_pb_add_sprite_user(joker_pb);

    memcpy32(&tile_mem[4][tile_index], &joker_gfxTiles[joker_spritesheet_idx][joker_idx * TILE_SIZE * JOKER_SPRITE_OFFSET], TILE_SIZE * JOKER_SPRITE_OFFSET);

    sprite_object_set_sprite
    (
        joker_object->sprite_object, 
        sprite_new
        (
            ATTR0_SQUARE | ATTR0_4BPP | ATTR0_AFF, 
            ATTR1_SIZE_32, 
            tile_index, 
            joker_pb,
            JOKER_STARTING_LAYER + layer
        )
    );

    

    return joker_object;
}

void joker_object_destroy(JokerObject **joker_object)
{
    if (joker_object == NULL || *joker_object == NULL) return;

    int layer = sprite_get_layer(joker_object_get_sprite(*joker_object)) - JOKER_STARTING_LAYER;
    used_layers[layer] = false;
    joker_pb_remove_sprite_user(sprite_get_pb(joker_object_get_sprite(*joker_object)));
    if (joker_pb_get_num_sprite_users((sprite_get_pb(joker_object_get_sprite(*joker_object)))) == 0)
    {
        joker_spritesheet_pb_map[joker_get_spritesheet_idx((*joker_object)->joker->id)] = UNDEFINED;
    }

    sprite_object_destroy(&(*joker_object)->sprite_object); // Destroy the sprite
    joker_destroy(&(*joker_object)->joker); // Destroy the joker
    POOL_FREE(JokerObject, *joker_object);
    *joker_object = NULL;
}

void joker_object_update(JokerObject *joker_object)
{
    CardObject *card_object = (CardObject *)joker_object;
    card_object_update(card_object);
}

void joker_object_shake(JokerObject *joker_object, mm_word sound_id)
{
    sprite_object_shake(joker_object->sprite_object, sound_id);
}

void set_and_shift_text(char* str, int* cursor_pos_x, int* cursor_pos_y, int color_pb)
{
    tte_set_pos(*cursor_pos_x, *cursor_pos_y);
    tte_set_special(color_pb * TTE_SPECIAL_PB_MULT_OFFSET);
    tte_write(str);

    // + 1 For space
    const int joker_score_display_offset_px = (MAX_CARD_SCORE_STR_LEN + 1) * TTE_CHAR_SIZE;
    *cursor_pos_x += joker_score_display_offset_px;
}

bool joker_object_score(JokerObject *joker_object, CardObject* card_object, enum JokerEvent joker_event, int *chips, int *mult, int *money, bool *retrigger)
{
    if (joker_object == NULL)
    {
        return false;
    }

    JokerEffect joker_effect = joker_get_score_effect(joker_object->joker, card_object->card, joker_event);

    if (memcmp(&joker_effect, &(JokerEffect){0}, sizeof(JokerEffect)) == 0)
    {
        return false;
    }

    *chips    += joker_effect.chips;
    *mult     += joker_effect.mult;
    *mult     *= joker_effect.xmult > 0 ? joker_effect.xmult : 1; // if xmult is zero, DO NOT multiply by it
    *money    += joker_effect.money;
    *retrigger = joker_effect.retrigger;
    // joker_effect.message will have been set if the Joker had anything custom to say

    int cursorPosX = TILE_SIZE; // Offset of one tile to better center the text on the card
    int cursorPosY = 0;
    if (joker_event == JOKER_EVENT_ON_CARD_HELD)
    {
        // display the text on top of the card instead of below the Joker for Held Cards effects
        // scored_card cannot be NULL here because of the joker event
        cursorPosX += fx2int(card_object->sprite_object->x);
        cursorPosY = HELD_CARD_SCORE_TEXT_Y;
    }
    else
    {
        cursorPosX += fx2int(joker_object->sprite_object->x);
        cursorPosY = JOKER_SCORE_TEXT_Y;
    }

    if (joker_effect.chips > 0)
    {
        char score_buffer[INT_MAX_DIGITS + 2]; // For '+' and null terminator
        snprintf(score_buffer, sizeof(score_buffer), "+%d", joker_effect.chips);
        set_and_shift_text(score_buffer, &cursorPosX, &cursorPosY, TTE_BLUE_PB);
    }
    if (joker_effect.mult > 0)
    {
        char score_buffer[INT_MAX_DIGITS + 2];
        snprintf(score_buffer, sizeof(score_buffer), "+%d", joker_effect.mult);
        set_and_shift_text(score_buffer, &cursorPosX, &cursorPosY, TTE_RED_PB);
    }
    if (joker_effect.xmult > 0)
    {
        char score_buffer[INT_MAX_DIGITS + 2];
        snprintf(score_buffer, sizeof(score_buffer), "X%d", joker_effect.xmult);
        set_and_shift_text(score_buffer, &cursorPosX, &cursorPosY, TTE_RED_PB);
    }
    if (joker_effect.money > 0)
    {
        char score_buffer[INT_MAX_DIGITS + 2];
        snprintf(score_buffer, sizeof(score_buffer), "%d$", joker_effect.money);
        set_and_shift_text(score_buffer, &cursorPosX, &cursorPosY, TTE_YELLOW_PB);
    }
    // custom message for Jokers (including retriggers where Jokers will say "Again!")
    if (joker_effect.message != NULL && joker_effect.message[0] != '\0') // Message is not empty
    {
        set_and_shift_text(joker_effect.message, &cursorPosX, &cursorPosY, TTE_WHITE_PB);
    }
    if (joker_effect.expire)
    {
        // TODO make Jokers expire
    }

    joker_object_shake(joker_object, SFX_CARD_SELECT); // TODO: Add a sound effect for scoring the joker

    return true;
}

void joker_object_set_selected(JokerObject* joker_object, bool selected)
{
    if (joker_object == NULL)
        return;
    sprite_object_set_selected(joker_object->sprite_object, selected);
}

bool joker_object_is_selected(JokerObject* joker_object)
{
    if (joker_object == NULL)
        return false;
    return sprite_object_is_selected(joker_object->sprite_object);
}

Sprite* joker_object_get_sprite(JokerObject* joker_object)
{
    if (joker_object == NULL)
        return NULL;
    return sprite_object_get_sprite(joker_object->sprite_object);
}

int joker_get_random_rarity()
{
    int joker_rarity = 0;
    int rarity_roll = random() % 100; 
    if (rarity_roll < COMMON_JOKER_CHANCE)  
    {
        joker_rarity = COMMON_JOKER;
    }
    else if (rarity_roll < COMMON_JOKER_CHANCE + UNCOMMON_JOKER_CHANCE) 
    {
        joker_rarity = UNCOMMON_JOKER;
    }
    else if (rarity_roll < COMMON_JOKER_CHANCE + UNCOMMON_JOKER_CHANCE + RARE_JOKER_CHANCE)
    {
        joker_rarity = RARE_JOKER;
    }
    else if (rarity_roll < COMMON_JOKER_CHANCE + UNCOMMON_JOKER_CHANCE + RARE_JOKER_CHANCE + LEGENDARY_JOKER_CHANCE)
    {
        joker_rarity = LEGENDARY_JOKER;
    }

    return joker_rarity;
}
