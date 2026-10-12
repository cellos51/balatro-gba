/**
 * @file sprite_container.h
 * @brief Definitions for the SpriteContainer data structure and its handling functions
 */

#ifndef SPRITE_CONTAINER_H
#define SPRITE_CONTAINER_H

#include "graphic_utils.h"
#include "list.h"
#include "sprite.h"

const static Rect CARD_SPRITE_LOCAL_RECT = {4, 0, 28, 32};

enum LayoutOrientation
{
    LAYOUT_ORI_HORIZONTAL,
    LAYOUT_ORI_VERTICAL,
    LAYOUT_ORI_MAX
};

enum LayoutJustification
{
    LAYOUT_JUST_BEGIN,
    LAYOUT_JUST_CENTER,
    LAYOUT_JUST_END,
    LAYOUT_JUST_OFF
};

typedef void (*ContainerCustomGlobalUpdate)(void);
typedef void (*ContainerCustomUpdate)(SpriteObject* sprite_object);

/**
 * @brief A structure organizing a List of SpriteObjects
 */
typedef struct SpriteContainer
{
    /**
     * @brief List of SpriteObjects to organize
     */
    List* contents_list;

    /**
     * @brief Custom callback to invoke only once. Called after the @ref contents are updated, but
     *         before @ref custom_update
     */
    ContainerCustomGlobalUpdate custom_global_update;

    /**
     * @brief Custom callback to invoke once per element in the @ref contents list. Called after the
     *         @ref custom_global_update
     */
    ContainerCustomUpdate custom_update;

    /**
     * @brief A Rect holding the position and size of the Container. Expressed in pixels.
     */
    Rect pos;

    /**
     * @brief Whether the sprites are to be organized horizontally or vertically
     */
    enum LayoutOrientation orientation;

    /**
     * @brief Determines the alignment of the sprites along the main direction. Cannot be
     *         **LAYOUT_JUST_OFF**
     */
    enum LayoutJustification justification;

    /**
     * @brief Determines the alignment of the sprites in the direction orthogonal to the main one.
     *         `LAYOUT_JUST_OFF` disables this justification.
     */
    enum LayoutJustification justification_ortho;

    /**
     * @brief A Rect representing the actual, local position/size of the art within the Sprites
     *         expressed in pixels.
     *
     * Examples:
     *   - For the Skip Tags, the sprite is 16x16 px, but the actual art always fits within
     *     a centered 10x10 px square, which gives `{3, 3, 13, 13}`
     *   - For the Jokers, Cards, Planets etc, they are generally rectangles of 24x32 px centered
     *     in a 32x32 px sprite, which gives `{4, 0, 28, 32}`
     *
     * The Sprites should all be uniform within a single Container, so this is stored here instead
     * of per-sprite.
     */
    Rect sprite_local_visible_rect;

    /**
     * @brief By how much do we need to space the Sprites, in case there are not enough to fill the
     *         whole Container. Expressed in pixels.
     */
    int maximum_spacing;
} SpriteContainer;

// clang-format off
const static Rect DEFAULT_POS_SIZE_RECT = {0, 0, 1, 1};
#define SPRITE_CONTAINER_DEFAULT {                      \
    .contents_list             = NULL,                  \
    .pos                       = DEFAULT_POS_SIZE_RECT, \
    .orientation               = LAYOUT_DIR_HORIZONTAL, \
    .justification             = LAYOUT_JUST_CENTER,    \
    .justification_ortho       = LAYOUT_JUST_CENTER,    \
    .sprite_local_visible_rect = DEFAULT_POS_SIZE_RECT, \
    .maximum_spacing           = 0                      \
}
// clang-format on

/**
 * @brief Update the position of the contents of a given Container
 *
 * @param container pointer to SpriteContainer to update the contents of
 */
void container_update(SpriteContainer* container);

/**
 * Prepend an entry to the `head` of a @ref SpriteContainer contents
 *
 * @param container pointer to a @ref SpriteContainer
 * @param sprite_object pointer to a SpriteObject to put into the @ref SpriteContainer
 * @param do_update whether the contents' spacing needs to update after pushing the new element
 *
 * @sa list_push_front
 */
void container_push_front(SpriteContainer* container, SpriteObject* sprite_object, bool do_update);

/**
 * Append an entry to the `tail` of a @ref SpriteContainer contents
 *
 * @param container pointer to a @ref SpriteContainer
 * @param sprite_object pointer to a SpriteObject to put into the @ref SpriteContainer
 * @param do_update whether the contents' spacing needs to update after pushing the new element
 *
 * @sa list_push_back
 */
void container_push_back(SpriteContainer* container, SpriteObject* sprite_object, bool do_update);

/** Insert a SpriteObject into a @ref SpriteContainer a specific index
 *
 * @param container pointer to a @ref SpriteContainer
 * @param sprite_object pointer to a SpriteObject to put into the @ref SpriteContainer
 * @param idx desired index to insert
 * @param do_update whether the contents' spacing needs to update after pushing the new element
 *
 * @sa list_insert
 */
void container_insert(
    SpriteContainer* container,
    SpriteObject* sprite_object,
    unsigned int idx,
    bool do_update
);

/**
 * Swap the data pointers at the specified indices of a @ref SpriteContainer
 *
 * If either indices are larger than the length of the underlying list, return false.
 *
 * @param container pointer to a @ref SpriteContainer
 * @param idx_a desired index to swap with idx_b
 * @param idx_b desired index to swap with idx_a
 * @param do_update whether the contents' spacing needs to update after pushing the new element
 *
 * @return true if successful, false otherwise
 *
 * @sa list_swap
 */
bool container_swap(
    SpriteContainer* container,
    unsigned int idx_a,
    unsigned int idx_b,
    bool do_update
);

/**
 * Clear the contents of a Container.
 *
 * Go through the underlying list and free each node and set the `head` and `tail` to `NULL`.
 * Note, it doesn't "free" the data at the node.
 *
 * @param container pointer to a @ref SpriteContainer to clear the contents of
 */
void container_clear_contents(SpriteContainer* container);

/**
 * Remove a SpriteContainer's underlying List node at the specified index
 *
 * @param container pointer to a @ref SpriteContainer
 * @param idx index of the desired @ref ListNode in the container
 * @param do_update whether the contents' spacing needs to update after pushing the new element
 *
 * @return `true` if successfully removed, `false` if out-of-bounds
 */
bool container_remove_at_idx(SpriteContainer* container, unsigned int idx, bool do_update);

/**
 * Remove a SpriteContainer's underlying List's node with the matching pointer
 *
 * @param container pointer to a @ref SpriteContainer
 * @param sprite_object pointer to a SpriteObject in node in container
 * @param do_update whether the contents' spacing needs to update after pushing the new element
 *
 * @return `true` if successfully removed, `false` otherwise
 *
 * @note When working with @ref ListItr, use @ref list_itr_remove_current_node()
 */
bool container_remove_data(SpriteContainer* container, SpriteObject* sprite_object, bool do_update);

/**
 * @brief Remove the current @ref ListNode from the iterator from the @ref SpriteContainer's
 *         underlying @ref List.
 *
 * @param container pointer to the @ref SpriteContainer to which the iterator's @ref List belongs to
 * @param itr pointer to the @ref ListItr
 * @param do_update whether the contents' spacing needs to update after pushing the new element
 *
 * @note When working with @ref ListItr, use this and not @ref container_remove_at_idx() as it will
 * "break" the iterator.
 *
 * @sa list_itr_remove_current_node
 */
void container_itr_remove_current_node(SpriteContainer* container, ListItr* itr, bool do_update);

#endif // SPRITE_CONTAINER_H
