#include "selection_grid.h"

static void selection_grid_process_directional_input(SelectionGrid* selection_grid)
{
    int horz_tri_input = bit_tribool(key_hit(KEY_ANY), KI_RIGHT, KI_LEFT);

    if (horz_tri_input != 0)
    {
        selection_grid_move_selection_horz(selection_grid, horz_tri_input);
        /* Avoid handling both vertical and horizontal input at the same time,
         * it creates all sorts of difficult edge cases.
         */
        return;
    }

    int vert_tri_input = bit_tribool(key_hit(KEY_ANY), KI_DOWN, KI_UP);

    if (vert_tri_input != 0)
    {
        selection_grid_move_selection_vert(selection_grid, vert_tri_input);
    }
}

void selection_grid_move_selection_horz(SelectionGrid* selection_grid, int direction_tribool)
{
    if (selection_grid == NULL)
        return;

    Selection new_selection = selection_grid->selection;
    new_selection.x += direction_tribool;
    if (selection_grid->selection.y >= 0 &&
        selection_grid->selection.y < selection_grid->num_rows && new_selection.x >= 0 &&
        new_selection.x < selection_grid->rows[new_selection.y].get_size())
    {
        selection_grid->rows[new_selection.y].on_selection_changed(
            selection_grid,
            new_selection.y,
            &selection_grid->selection,
            &new_selection
        );
        selection_grid->selection = new_selection;
    }
}

void selection_grid_move_selection_vert(SelectionGrid* selection_grid, int direction_tribool)
{
    if (selection_grid == NULL)
        return;

    Selection selection = selection_grid->selection;
    Selection new_selection = selection;
    new_selection.y += direction_tribool;

    if (new_selection.y >= 0 && new_selection.y < selection_grid->num_rows)
    {
        int new_row_size = selection_grid->rows[new_selection.y].get_size();
        if (new_row_size <= 0)
            return;

        if (selection.x >= new_row_size)
        {
            // TODO: Maintain relative horizontal position
            // Clip selection to row size
            new_selection.x = new_row_size - 1;
        }
        if (selection.y >= 0 && selection.y < selection_grid->num_rows)
        {
            selection_grid->rows[selection.y]
                .on_selection_changed(selection_grid, selection.y, &selection, &new_selection);
        }
        selection_grid->rows[new_selection.y]
            .on_selection_changed(selection_grid, new_selection.y, &selection, &new_selection);
        selection_grid->selection = new_selection;
    }
}

void selection_grid_process_input(SelectionGrid* selection_grid)
{
    if (selection_grid == NULL || selection_grid->rows == NULL)
        return;

    selection_grid_process_directional_input(selection_grid);

    u32 non_directional_key = KEY_ANY & ~KEY_DIR;
    if (key_transit(non_directional_key))
    {
        // To make the next line shorter and more readable
        Selection* selection = &selection_grid->selection;
        selection_grid->rows[selection->y].on_key_transit(selection_grid, selection);
    }
}
