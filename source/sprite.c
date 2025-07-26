#include "sprite.h"
#include "util.h"

#include <tonc.h>
#include <stdlib.h>
#include <maxmod.h>

#define MAX_SPRITES 128
#define MAX_AFFINES 32

OBJ_ATTR obj_buffer[MAX_SPRITES];
OBJ_AFFINE *obj_aff_buffer = (OBJ_AFFINE*)obj_buffer;

static Sprite *free_sprites[MAX_SPRITES] = {NULL};
static bool free_affines[MAX_AFFINES] = {false};

// Sprite methods
Sprite *sprite_new(u16 a0, u16 a1, u32 tid, u32 pb, int sprite_index)
{
    Sprite *sprite = malloc(sizeof(Sprite));

    sprite->obj = NULL;
    sprite->aff = NULL;

    if(!free_sprites[sprite_index])
    {
        free_sprites[sprite_index] = sprite;
    }
    else
    {
        free(sprite);
        return NULL;
    }

    if (a0 & ATTR0_AFF)
    {
        int aff_index = MAX_AFFINES;

        for(int i = 0; i < MAX_AFFINES; i++)
        {
            if(!free_affines[i])
            {
                free_affines[i] = true;
                aff_index = i;
                break;
            }
        }

        if (aff_index == MAX_AFFINES)
        {
            free(sprite);
            return NULL;
        }

        a1 = a1 | ATTR1_AFF_ID(aff_index);

        sprite->obj = &obj_buffer[sprite_index];
        sprite->aff = &obj_aff_buffer[aff_index];
        obj_set_attr(sprite->obj, a0, a1, ATTR2_PALBANK(pb) | tid);
        obj_aff_identity(&obj_aff_buffer[aff_index]);
        return sprite;
    }
    else
    {
        sprite->obj = &obj_buffer[sprite_index];
        obj_set_attr(sprite->obj, a0, a1, ATTR2_PALBANK(pb) | tid);
        return sprite;
    }
}

void sprite_destroy(Sprite **sprite)
{
    if (*sprite == NULL) return;
    obj_hide((*sprite)->obj);
    free_sprites[(*sprite)->obj - obj_buffer] = NULL;
    if ((*sprite)->aff != NULL)
    {
        free_affines[(*sprite)->aff - obj_aff_buffer] = false;
    }
    free(*sprite);
    *sprite = NULL;
}

int sprite_get_layer(Sprite *sprite)
{
    if (sprite == NULL || sprite->obj == NULL) return -1;
    return sprite->obj - obj_buffer;
}

// Sprite functions
void sprite_init()
{
    oam_init(obj_buffer, MAX_SPRITES); 
}

void sprite_draw()
{
    obj_copy(obj_mem, obj_buffer, MAX_SPRITES);
    obj_aff_copy(obj_aff_mem, obj_aff_buffer, MAX_AFFINES);
    oam_copy(oam_mem, obj_buffer, MAX_SPRITES);
}

// SpriteObject methods
SpriteObject* sprite_object_new()
{
    SpriteObject* sprite_object = (SpriteObject*)malloc(sizeof(SpriteObject));
    sprite_object->sprite = NULL;
    sprite_object_reset_transform(sprite_object);
    sprite_object->selected = false;

    return sprite_object;
}

void sprite_object_destroy(SpriteObject** sprite_object)
{
    if (*sprite_object == NULL) return;
    sprite_destroy(&((*sprite_object)->sprite));
    free(*sprite_object);
    *sprite_object = NULL;
}

void sprite_object_set_sprite(SpriteObject* sprite_object, Sprite* sprite)
{
    if (sprite_object == NULL)
        return;
    sprite_destroy(&sprite_object->sprite); // Destroy the old sprite if it exists
    sprite_object->sprite = sprite;
}

void sprite_object_reset_transform(SpriteObject* sprite_object)
{
    sprite_object->tx = 0; // Target position
    sprite_object->ty = 0;
    sprite_object->x = 0;
    sprite_object->y = 0;
    sprite_object->vx = 0;
    sprite_object->vy = 0;
    sprite_object->tscale = float2fx(1.0f); // Target scale
    sprite_object->scale = float2fx(1.0f);
    sprite_object->vscale = float2fx(0.0f);
    sprite_object->trotation = 0; // Target rotation
    sprite_object->rotation = 0;
    sprite_object->vrotation = 0;
}

void sprite_object_update(SpriteObject* sprite_object)
{
    sprite_object->vx += (sprite_object->tx - sprite_object->x) / 8;
    sprite_object->vy += (sprite_object->ty - sprite_object->y) / 8;

    sprite_object->vscale += (sprite_object->tscale - sprite_object->scale) / 8; // Scale up the card when it's played

    sprite_object->vrotation += (sprite_object->trotation - sprite_object->rotation) / 8; // Rotate the card when it's played

    // set velocity to 0 if it's close enough to the target
    const float epsilon = float2fx(0.01f);
    if (sprite_object->vx < epsilon && sprite_object->vx > -epsilon && sprite_object->vy < epsilon && sprite_object->vy > -epsilon)
    {
        sprite_object->vx = 0;
        sprite_object->vy = 0;

        sprite_object->x = sprite_object->tx;
        sprite_object->y = sprite_object->ty;
    }
    else
    {
        sprite_object->vx = (sprite_object->vx * 7) / 10;
        sprite_object->vy = (sprite_object->vy * 7) / 10;

        sprite_object->x += sprite_object->vx;
        sprite_object->y += sprite_object->vy;
    }

    // Set scale to 0 if it's close enough to the target
    if (sprite_object->vscale < epsilon && sprite_object->vscale > -epsilon)
    {
        sprite_object->vscale = 0;
        sprite_object->scale = sprite_object->tscale; // Set the scale to the target scale
    }
    else
    {
        sprite_object->vscale = (sprite_object->vscale * 7) / 10;
        sprite_object->scale += sprite_object->vscale;
    }

    // Set rotation to 0 if it's close enough to the target
    if (sprite_object->vrotation < epsilon && sprite_object->vrotation > -epsilon)
    {
        sprite_object->vrotation = 0;
        sprite_object->rotation = sprite_object->trotation; // Set the rotation to the target rotation
    }
    else
    {
        sprite_object->vrotation = (sprite_object->vrotation * 7) / 10;
        sprite_object->rotation += sprite_object->vrotation;
    }

    obj_aff_rotscale(sprite_object->sprite->aff, sprite_object->scale, sprite_object->scale, -sprite_object->vx + sprite_object->rotation); // Apply rotation and scale to the sprite
    sprite_position(sprite_object->sprite, fx2int(sprite_object->x), fx2int(sprite_object->y));
}

void sprite_object_shake(SpriteObject* sprite_object, mm_word sound_id)
{
    sprite_object->vscale = float2fx(0.3f); // Scale down the card when it's scored
    sprite_object->vrotation = float2fx(8.0f); // Rotate the card when it's scored

    if (sound_id == UNDEFINED) return; // If no sound ID is provided, do nothing

    mm_sound_effect sfx_select = { {sound_id}, 1024, 0, 255, 128, };
    mmEffectEx(&sfx_select);
}

void sprite_object_set_selected(SpriteObject* sprite_object, bool selected)
{
    if (sprite_object == NULL)
        return;
    sprite_object->selected = selected;
}

bool sprite_object_is_selected(SpriteObject* sprite_object)
{
    if (sprite_object == NULL)
        return false;
    return sprite_object->selected;
}

Sprite* sprite_object_get_sprite(SpriteObject* sprite_object)
{
    if (sprite_object == NULL)
        return NULL;
    return sprite_object->sprite;
}
