#include "sprite_container.h"

#include "util.h"

static void container_adjust_pos_and_spacing(
    SpriteContainer* container,
    int max_length,
    int overrun,
    int nb_sprites,
    int sprite_size,
    int* spacing,
    POINT* start_pos
);
static void container_adjust_starting_pos_ortho(
    SpriteContainer* container,
    int overrun_ortho,
    POINT* start_pos
);
static void container_update_set_sprite_pos(
    SpriteContainer* container,
    POINT pos_size,
    POINT pos_size_ortho,
    int spacing,
    POINT* start_pos
);

void container_update(SpriteContainer* container)
{
    GBAL_RETURN_IF_NULL(container, RET_NONE);

    // position and size of the sprite along the main `direction`
    POINT pos_size;
    // same, but with direction orthogonal to the main one
    POINT pos_size_ortho;
    // Sprites' starting position along the main and orthogonal directions respectively
    POINT start_pos;
    // size of the container along the main and orthogonal directions respectively
    POINT max_length;

    if (container->orientation == LAYOUT_ORI_HORIZONTAL)
    {
        pos_size.x = container->sprite_local_visible_rect.left;
        pos_size.y = rect_width(&(container->sprite_local_visible_rect)) - 1;

        pos_size_ortho.x = container->sprite_local_visible_rect.top;
        pos_size_ortho.y = rect_height(&(container->sprite_local_visible_rect)) - 1;

        start_pos.x = container->pos.left;
        start_pos.y = container->pos.top;

        max_length.x = rect_width(&(container->pos));
        max_length.y = rect_height(&(container->pos));
    }
    else
    {
        pos_size.x = container->sprite_local_visible_rect.top;
        pos_size.y = rect_height(&(container->sprite_local_visible_rect)) - 1;

        pos_size_ortho.x = container->sprite_local_visible_rect.left;
        pos_size_ortho.y = rect_width(&(container->sprite_local_visible_rect)) - 1;

        start_pos.x = container->pos.top;
        start_pos.y = container->pos.left;

        max_length.x = rect_height(&(container->pos));
        max_length.y = rect_width(&(container->pos));
    }

    int nb_sprites = list_get_len(container->contents_list);
    int spacing = container->maximum_spacing;
    int naive_length = nb_sprites * pos_size.y + (nb_sprites - 1) * spacing;

    int overrun = naive_length - max_length.x;
    int overrun_ortho = pos_size_ortho.y - max_length.y;

    container_adjust_pos_and_spacing(
        container,
        max_length.x,
        overrun,
        nb_sprites,
        pos_size.y,
        &spacing,
        &start_pos
    );

    container_adjust_starting_pos_ortho(container, overrun_ortho, &start_pos);

    container_update_set_sprite_pos(container, pos_size, pos_size_ortho, spacing, &start_pos);

    // Invoke custom callbacks if any
    if (container->custom_global_update)
        container->custom_global_update();
    if (container->custom_update)
    {
        ListItr itr = list_itr_create(container->contents_list);
        SpriteObject* sprite_object;
        while ((sprite_object = list_itr_next(&itr)))
            container->custom_update(sprite_object);
    }
}

/**
 * @brief Take all container info and determine at what position and with which spacing all
 *         container contents should be displayed
 *
 * @param[in]  container   SpriteContainer object to update
 * @param[in]  max_length  maximum size of the Container, in pixels, along the orientation its
 *                         Sprites are arranged in
 * @param[in]  overrun     How many pixels the naively arranged Sprites overflow from the
 *                         @p max_length
 * @param[in]  nb_sprites  Number of elements in the @p container
 * @param[in]  sprite_size Size of the Sprites, in pixels, along the orientation they are arranged
 *                         in
 * @param[out] spacing     pointer to the base spacing value, will be updated to reflect how the
 *                         Sprites will be arranged
 * @param[out] start_pos   pointer to the Point in space the Sprites start at initially, will be
 *                         updated to reflect how the Sprites will be arranged
 */
static inline void container_adjust_pos_and_spacing(
    SpriteContainer* container,
    int max_length,
    int overrun,
    int nb_sprites,
    int sprite_size,
    int* spacing,
    POINT* start_pos
)
{
    // If sprites take too much space, correct the spacing
    if (overrun > 0 && nb_sprites > 1)
    {
        // Ceil the reduction so the corrected layout does not still overflow.
        int reduce = (overrun + (nb_sprites - 2)) / (nb_sprites - 1);
        *spacing -= reduce;

        // If the sprites need to be centered, and depending on the numer of them, the reduction to
        // the spacing may cause an imbalance that can be solved by shifting the sprites slightly
        // to the right
        if (container->justification == LAYOUT_JUST_CENTER)
        {
            int new_length = nb_sprites * sprite_size + (nb_sprites - 1) * (*spacing);
            start_pos->x += (max_length - new_length) / 2;
        }
    }
    // When they fit inside the container, correct the starting point if need be
    else if (overrun < 0)
    {
        switch (container->justification)
        {
            case LAYOUT_JUST_CENTER:
                start_pos->x -= overrun / 2;
                break;

            case LAYOUT_JUST_END:
                start_pos->x -= overrun;
                break;

            default:
                // Don't need to correct anything for the default alignment
                break;
        }
    }
    // And don't do anything if the fit is perfect (overrun == 0)
}

/**
 * @brief Determine the position along the axis orthogonal to the main orientation container
 *         contents should be displayed
 *
 * @param[in]  container     SpriteContainer object to update
 * @param[in]  overrun_ortho How many pixels the naively arranged Sprites overflow from the
 *                            @p max_length
 * @param[out] start_pos     pointer to the Point in space the Sprites start at initially, will be
 *                            updated to reflect how the Sprites will be arranged
 */
static inline void container_adjust_starting_pos_ortho(
    SpriteContainer* container,
    int overrun_ortho,
    POINT* start_pos
)
{
    if (container->justification_ortho != LAYOUT_JUST_OFF)
    {
        // Always center along the direction orthogonal to the main one if Sprite takes too much
        // space
        if (overrun_ortho > 0)
            start_pos->y -= overrun_ortho / 2;
        // If there's enough room, align Sprite accoring to `container->justification_ortho`
        else if (overrun_ortho < 0)
        {
            switch (container->justification_ortho)
            {
                case LAYOUT_JUST_CENTER:
                    start_pos->y -= overrun_ortho / 2;
                    break;

                case LAYOUT_JUST_END:
                    start_pos->y -= overrun_ortho;
                    break;

                default:
                    // Still don't need to correct anything for the default alignment
                    break;
            }
        }
    }
}

/**
 * @brief Set the positions for all Sprites inside of a Container
 *
 * @param[in]  container      SpriteContainer object to update
 * @param[in]  pos_size       Actual position/size of the Sprite on the axis along which they are
 *                            organized on
 * @param[in]  pos_size_ortho Actual position/size of the Sprite on the axis perpendicular to the
 *                            one which they are organized on
 * @param[in]  spacing        How many pixels are put in between each Sprite. Can be negative if
 *                            Sprites need to overlap
 * @param[out] start_pos      pointer to the Point in space the Sprites start at initially.
 */
static inline void container_update_set_sprite_pos(
    SpriteContainer* container,
    POINT pos_size,
    POINT pos_size_ortho,
    int spacing,
    POINT* start_pos
)
{
    SpriteObject* sprite_object = NULL;
    ListItr itr = list_itr_create(container->contents_list);

    while ((sprite_object = list_itr_next(&itr)))
    {
        FIXED* coord;
        if (container->orientation == LAYOUT_ORI_HORIZONTAL)
            coord = &sprite_object->tx;
        else
            coord = &sprite_object->ty;
        *coord = int2fx(start_pos->x - pos_size.x);

        if (container->justification_ortho != LAYOUT_JUST_OFF)
        {
            FIXED* coord_ortho;
            if (container->orientation == LAYOUT_ORI_HORIZONTAL)
                coord_ortho = &sprite_object->ty;
            else
                coord_ortho = &sprite_object->tx;
            *coord_ortho = int2fx(start_pos->y - pos_size_ortho.x);
        }

        start_pos->x += pos_size.y + spacing;
    }
}

void container_push_front(SpriteContainer* container, SpriteObject* sprite_object, bool do_update)
{
    GBAL_RETURN_IF_NULL(container, RET_NONE);
    GBAL_RETURN_IF_NULL(sprite_object, RET_NONE);
    list_push_front(container->contents_list, (void*)sprite_object);
    if (do_update)
        container_update(container);
}

void container_push_back(SpriteContainer* container, SpriteObject* sprite_object, bool do_update)
{
    GBAL_RETURN_IF_NULL(container, RET_NONE);
    GBAL_RETURN_IF_NULL(sprite_object, RET_NONE);
    list_push_back(container->contents_list, (void*)sprite_object);
    if (do_update)
        container_update(container);
}

void container_insert(
    SpriteContainer* container,
    SpriteObject* sprite_object,
    unsigned int idx,
    bool do_update
)
{
    GBAL_RETURN_IF_NULL(container, RET_NONE);
    GBAL_RETURN_IF_NULL(sprite_object, RET_NONE);
    list_insert(container->contents_list, (void*)sprite_object, idx);
    if (do_update)
        container_update(container);
}

bool container_swap(
    SpriteContainer* container,
    unsigned int idx_a,
    unsigned int idx_b,
    bool do_update
)
{
    GBAL_RETURN_IF_NULL(container, false);
    bool res = list_swap(container->contents_list, idx_a, idx_b);
    if (do_update)
        container_update(container);
    return res;
}

void container_clear_contents(SpriteContainer* container)
{
    GBAL_RETURN_IF_NULL(container, RET_NONE);
    list_clear(container->contents_list);
}

bool container_remove_at_idx(SpriteContainer* container, unsigned int idx, bool do_update)
{
    GBAL_RETURN_IF_NULL(container, false);
    bool res = list_remove_at_idx(container->contents_list, idx);
    if (do_update)
        container_update(container);
    return res;
}

bool container_remove_data(SpriteContainer* container, SpriteObject* sprite_object, bool do_update)
{
    GBAL_RETURN_IF_NULL(container, false);
    GBAL_RETURN_IF_NULL(sprite_object, false);
    bool res = list_remove_data(container->contents_list, (void*)sprite_object);
    if (do_update)
        container_update(container);
    return res;
}

void container_itr_remove_current_node(SpriteContainer* container, ListItr* itr, bool do_update)
{
    GBAL_RETURN_IF_NULL(container, RET_NONE);
    list_itr_remove_current_node(itr);
    if (do_update)
        container_update(container);
}
