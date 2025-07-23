#include "card.h"

#include <maxmod.h>
#include <stdlib.h>

#include "deck_gfx.h"

// Audio
#include "soundbank.h"

// Card sprites lookup table. First index is the suit, second index is the rank. The value is the tile index.
const static u16 card_sprite_lut[NUM_SUITS][NUM_RANKS] = {
    {0, 16, 32, 48, 64, 80, 96, 112, 128, 144, 160, 176, 192},
    {208, 224, 240, 256, 272, 288, 304, 320, 336, 352, 368, 384, 400},
    {416, 432, 448, 464, 480, 496, 512, 528, 544, 560, 576, 592, 608},
    {624, 640, 656, 672, 688, 704, 720, 736, 752, 768, 784, 800, 816}
};

void card_init()
{
    GRIT_CPY(&tile_mem[4], deck_gfxTiles);
    GRIT_CPY(pal_obj_mem, deck_gfxPal);
}

// Card methods
Card *card_new(u8 suit, u8 rank)
{
    Card *card = malloc(sizeof(Card));

    card->suit = suit;
    card->rank = rank;

    return card;
}

void card_destroy(Card **card)
{
    if (*card == NULL) return;
    free(*card);
    *card = NULL;
}

u8 card_get_value(Card *card)
{
    if (card->rank == JACK || card->rank == QUEEN || card->rank == KING)
    {
        return 10; // Face cards are worth 10
    }
    else if (card->rank == ACE)
    {
        return 11; // Ace is worth 11
    }
    else
    {
        return card->rank + RANK_OFFSET; // 2-10 are worth their rank + RANK_OFFSET
    }

    return 0; // Should never reach here, but just in case
}

// CardObject methods
CardObject *card_object_new(Card *card)
{
    CardObject *card_object = malloc(sizeof(CardObject));

    card_object->card = card;
    card_object->sprite = NULL;
    card_object->tx = 0; // Target position
    card_object->ty = 0;
    card_object->x = 0;
    card_object->y = 0;
    card_object->vx = 0;
    card_object->vy = 0;
    card_object->tscale = float2fx(1.0f); // Target scale
    card_object->scale = float2fx(1.0f);
    card_object->vscale = float2fx(0.0f);
    card_object->trotation = 0; // Target rotation
    card_object->rotation = 0;
    card_object->vrotation = 0;
    card_object->selected = false;

    return card_object;
}

void card_object_destroy(CardObject **card_object)
{
    if (*card_object == NULL) return;
    sprite_destroy(&(*card_object)->sprite);
    //card_destroy(&(*card_object)->card); // In practice, this is unnecessary because the card will be inserted into the discard pile and then back into the deck. If you need to destroy the card, you can do it manually before calling this function.
    free(*card_object);
    *card_object = NULL;
}

void card_object_update(CardObject *card_object)
{
    card_object->vx += (card_object->tx - card_object->x) / 8;
    card_object->vy += (card_object->ty - card_object->y) / 8;

    card_object->vscale += (card_object->tscale - card_object->scale) / 8; // Scale up the card when it's played

    card_object->vrotation += (card_object->trotation - card_object->rotation) / 8; // Rotate the card when it's played

    // set velocity to 0 if it's close enough to the target
    const float epsilon = float2fx(0.01f);
    if (card_object->vx < epsilon && card_object->vx > -epsilon && card_object->vy < epsilon && card_object->vy > -epsilon)
    {
        card_object->vx = 0;
        card_object->vy = 0;

        card_object->x = card_object->tx;
        card_object->y = card_object->ty;
    }
    else
    {
        card_object->vx = (card_object->vx * 7) / 10;
        card_object->vy = (card_object->vy * 7) / 10;

        card_object->x += card_object->vx;
        card_object->y += card_object->vy;
    }

     // Set scale to 0 if it's close enough to the target
    if (card_object->vscale < epsilon && card_object->vscale > -epsilon)
    {
        card_object->vscale = 0;
        card_object->scale = card_object->tscale; // Set the scale to the target scale
    }
    else
    {
        card_object->vscale = (card_object->vscale * 7) / 10;
        card_object->scale += card_object->vscale;
    }

    // Set rotation to 0 if it's close enough to the target
    if (card_object->vrotation < epsilon && card_object->vrotation > -epsilon)
    {
        card_object->vrotation = 0;
        card_object->rotation = card_object->trotation; // Set the rotation to the target rotation
    }
    else
    {
        card_object->vrotation = (card_object->vrotation * 7) / 10;
        card_object->rotation += card_object->vrotation;
    }

    obj_aff_rotscale(card_object->sprite->aff, card_object->scale, card_object->scale, -card_object->vx + card_object->rotation); // Apply rotation and scale to the sprite
    sprite_position(card_object->sprite, fx2int(card_object->x), fx2int(card_object->y));
}

void card_object_set_sprite(CardObject *card_object, int layer)
{
    sprite_destroy(&card_object->sprite); // Destroy the old sprite if it exists
    card_object->sprite = sprite_new(ATTR0_SQUARE | ATTR0_4BPP | ATTR0_AFF, ATTR1_SIZE_32, card_sprite_lut[card_object->card->suit][card_object->card->rank], 0, layer);
}

void card_object_shake(CardObject *card_object, mm_word sound_id)
{
    card_object->vscale = float2fx(0.3f); // Scale down the card when it's scored
    card_object->vrotation = float2fx(8.0f); // Rotate the card when it's scored

    if (sound_id == -1) return; // If no sound ID is provided, do nothing

    mm_sound_effect sfx_select = {{sound_id}, 1024, 0, 255, 128,};
    mmEffectEx(&sfx_select);
}
