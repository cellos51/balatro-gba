#ifndef SELECTION_GRID_H
#define SELECTION_GRID_H
#include <tonc.h>

typedef POINT Selection;

struct SelectionGridRow;
struct SelectionGrid;
typedef struct SelectionGridRow SelectionGridRow;
typedef struct SelectionGrid SelectionGrid;

// Called whenever there is a change in the selection cursor
// row_idx is the index of the row whose function is invoked - can be used to identify whether it is the previous or new selection row.
typedef void (*RowOnSelectionChangedFunc)(SelectionGrid* selection_grid, int row_idx, const Selection* prev_selection, const Selection* new_selection);
typedef int (*RowGetSizeFunc)();
// Called for any non-directional key hit
// The key will not be passed, the function will have to check key_hit() etc. for the key it wants to check
typedef void (*RowOnKeyHitFunc)(SelectionGrid* selection_grid, Selection* selection);

struct SelectionGridRow
{
    int row_idx;
    RowGetSizeFunc get_size; 
    RowOnSelectionChangedFunc on_selection_changed; 
    RowOnKeyHitFunc on_key_hit;
};

struct SelectionGrid
{
    SelectionGridRow* rows;
    const int num_rows;
    Selection selection;
 };


void selection_grid_process_input(SelectionGrid* selection_grid);

void selection_grid_move_selection_horz(SelectionGrid* selection_grid, int direction_tribool);
void selection_grid_move_selection_vert(SelectionGrid* selection_grid, int direction_tribool);

#endif