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

#define JOKER_SCORE_TEXT_Y 48
#define NUM_JOKERS_PER_SPRITESHEET 2

const unsigned int **joker_gfxTiles;
const unsigned short **joker_gfxPal;

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
static int* joker_spritesheet_pb_map;

static int get_num_spritesheets()
{
    return (get_joker_registry_size() + NUM_JOKERS_PER_SPRITESHEET - 1) / NUM_JOKERS_PER_SPRITESHEET;
}

// TODO: This should be generalized so any sprite can have dynamic swapping
static int allocate_pb_if_needed(u8 joker_id)
{
    int joker_spritesheet_idx = joker_id / NUM_JOKERS_PER_SPRITESHEET;
    int joker_pb = joker_spritesheet_pb_map[joker_spritesheet_idx];
    if (joker_pb != UNDEFINED)
    {
        // Already allocated
        return joker_pb;
    }
    
    // Allocate a new palette
    joker_pb = int_arr_max(joker_spritesheet_pb_map, get_num_spritesheets());

    if (joker_pb == UNDEFINED)
    {
        // This is the first palette loaded
        joker_pb = JOKER_BASE_PB;
    }
    else
    {
        // The new palette bank is a 1 increment of the last allocated one
        /* TODO: This doesn't account for removal, we need to address that,
         * track sprite usage and remove palettes for unused sprites
         */ 
        joker_pb = joker_pb + 1;
    }

    if (joker_pb == NUM_PALETTES)
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
    joker_gfxTiles = (const unsigned int**)malloc((sizeof(unsigned int*) * num_spritesheets));
    joker_gfxPal = (const unsigned short**)malloc((sizeof(unsigned int*) * num_spritesheets));
    joker_spritesheet_pb_map = (int*)malloc(sizeof(int) * num_spritesheets);
    
    // TODO: Automate this with the preprocessor somehow?
    joker_gfxTiles[0] = joker_gfx0Tiles;
    joker_gfxTiles[1] = joker_gfx1Tiles;
    joker_gfxTiles[2] = joker_gfx2Tiles;
    joker_gfxTiles[3] = joker_gfx3Tiles;
    joker_gfxTiles[4] = joker_gfx4Tiles;
    joker_gfxTiles[5] = joker_gfx5Tiles;
    joker_gfxTiles[6] = joker_gfx6Tiles;
    joker_gfxTiles[7] = joker_gfx7Tiles;
    joker_gfxTiles[8] = joker_gfx8Tiles;

    joker_gfxPal[0] = joker_gfx0Pal;
    joker_gfxPal[1] = joker_gfx1Pal;
    joker_gfxPal[2] = joker_gfx2Pal;
    joker_gfxPal[3] = joker_gfx3Pal;
    joker_gfxPal[4] = joker_gfx4Pal;
    joker_gfxPal[5] = joker_gfx5Pal;
    joker_gfxPal[6] = joker_gfx6Pal;
    joker_gfxPal[7] = joker_gfx7Pal;
    joker_gfxPal[8] = joker_gfx8Pal;

    for (int i = 0; i < num_spritesheets; i++)
    {
        joker_spritesheet_pb_map[i] = UNDEFINED;
    }
}

Joker *joker_new(u8 id)
{
    if (id >= get_joker_registry_size()) return NULL;

    Joker *joker = (Joker*)malloc(sizeof(Joker));
    const JokerInfo *jinfo = get_joker_registry_entry(id);

    joker->id = id;
    joker->modifier = BASE_EDITION; // TODO: Make this a parameter
    joker->value = jinfo->base_value + edition_price_lut[joker->modifier]; // Base value + edition price
    joker->rarity = jinfo->rarity;
    joker->processed = false;

    return joker;
}

void joker_destroy(Joker **joker)
{
    if (*joker == NULL) return;
    free(*joker);
    *joker = NULL;
}

JokerEffect joker_get_score_effect(Joker *joker, Card *scored_card)
{
    const JokerInfo *jinfo = get_joker_registry_entry(joker->id);
    if (!jinfo || jinfo->effect == NULL) return (JokerEffect){0};

    return jinfo->effect(joker, scored_card);
}

// JokerObject methods
JokerObject *joker_object_new(Joker *joker)
{
    JokerObject *joker_object = malloc(sizeof(JokerObject));

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
    
    int joker_spritesheet_idx = joker->id / NUM_JOKERS_PER_SPRITESHEET;
    int joker_idx = joker->id % NUM_JOKERS_PER_SPRITESHEET;
    int joker_pb = allocate_pb_if_needed(joker->id);

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
    if (*joker_object == NULL) return;

    int layer = sprite_get_layer(joker_object_get_sprite(*joker_object)) - JOKER_STARTING_LAYER;
    used_layers[layer] = false;
    sprite_object_destroy(&(*joker_object)->sprite_object); // Destroy the sprite
    joker_destroy(&(*joker_object)->joker); // Destroy the joker
    free(*joker_object);
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

bool joker_object_score(JokerObject *joker_object, Card* scored_card, int *chips, int *mult, int *xmult, int *money, bool *retrigger)
{
    if (joker_object->joker->processed == true) return false; // If the joker has already been processed, return false

    JokerEffect joker_effect = joker_get_score_effect(joker_object->joker, scored_card);

    if (memcmp(&joker_effect, &(JokerEffect){0}, sizeof(JokerEffect)) != 0)
    {
        *chips += joker_effect.chips;
        *mult += joker_effect.mult;
        *mult *= joker_effect.xmult > 0 ? joker_effect.xmult : 1; // if xmult is zero, DO NOT multiply by it
        *money += joker_effect.money;
        // TODO: Retrigger

        int cursorPosX = fx2int(joker_object->sprite_object->x) + 8; // Offset of 16 pixels to center the text on the card
        if (joker_effect.chips > 0)
        {
            char score_buffer[12];
            tte_set_pos(cursorPosX, JOKER_SCORE_TEXT_Y);
            tte_set_special(0xD000); // Blue
            snprintf(score_buffer, sizeof(score_buffer), "+%d", joker_effect.chips);
            tte_write(score_buffer);
            cursorPosX += 30;
        }
        if (joker_effect.mult > 0)
        {
            char score_buffer[12];
            tte_set_pos(cursorPosX, JOKER_SCORE_TEXT_Y);
            tte_set_special(0xE000); // Red
            snprintf(score_buffer, sizeof(score_buffer), "+%d", joker_effect.mult);
            tte_write(score_buffer);
            cursorPosX += 30;
        }
        if (joker_effect.xmult > 0)
        {
            char score_buffer[12];
            tte_set_pos(cursorPosX, JOKER_SCORE_TEXT_Y);
            tte_set_special(0xE000); // Red
            snprintf(score_buffer, sizeof(score_buffer), "X%d", joker_effect.xmult);
            tte_write(score_buffer);
            cursorPosX += 30;
        }
        if (joker_effect.money > 0)
        {
            char score_buffer[12];
            tte_set_pos(cursorPosX, JOKER_SCORE_TEXT_Y);
            tte_set_special(0xC000); // Yellow
            snprintf(score_buffer, sizeof(score_buffer), "+%d", joker_effect.money);
            tte_write(score_buffer);
            cursorPosX += 30;
        }

        joker_object->joker->processed = true; // Mark the joker as processed
        joker_object_shake(joker_object, SFX_CARD_SELECT); // TODO: Add a sound effect for scoring the joker

        return true;
    }

    return false;
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
