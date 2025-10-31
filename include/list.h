#ifndef LIST_H
#define LIST_H

#include <stdbool.h>

typedef struct List
{
    void** _array;
    int size;
    int allocated_size;
} List;

List *list_new(int init_size);
void list_destroy(List **list);
bool list_append(List *list, void *value);
bool list_remove_by_idx(List *list, int index);
void* list_get(List *list, int index);
int list_get_size(List *list);
bool list_remove_by_value(List *list, void *value);
bool list_exists(List *list, void *value);

bool int_list_append(List *list, intptr_t value);
intptr_t int_list_get(List *list, int index);
bool int_list_remove_by_value(List *list, intptr_t value);
bool int_list_exists(List *list, intptr_t value);

#endif
