#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include "list.h"
#include "util.h"

List *list_new(int init_size) {
    List *list = (List *)malloc(sizeof(List));
    if (list == NULL) return NULL;
    list->_array = (void **)malloc(sizeof(void*) * init_size);
    if (!list->_array) 
    {
        free(list);
        return NULL;
    }
    list->size = 0;
    list->allocated_size = init_size;
    return list;
}

void list_destroy(List **list) {
    if (list == NULL || *list == NULL)
        return; 
    {
        free((*list)->_array);
        free(*list);
    }

    *list = NULL;
}

bool int_list_append(List *list, intptr_t value) 
{
    return list_append(list, (void*)value);
}

bool list_append(List *list, void *value)
{
    if (list->size >= list->allocated_size) 
    {
        int new_size = list->allocated_size * 2;
        void **new_arr = (void **)realloc(list->_array, sizeof(void*) * new_size);
        if (new_arr == NULL) 
            return false;
        list->_array = new_arr;
        list->allocated_size = new_size;
    }

    list->_array[list->size++] = value;
    return true;
}

bool list_remove_by_idx(List *list, int index) {
    if (index < 0 || index >= list->size) 
        return false;
    for (int i = index; i < list->size - 1; ++i) 
    {
        list->_array[i] = list->_array[i + 1];
    }
    list->size--;
    return true;
}

bool list_remove_by_value(List *list, void* value)
{
    for (int i = 0; i < list->size; i++)
    {
        if (list->_array[i] == value)
        {
            return list_remove_by_idx(list, i);
        }
    }

    return false;
}

bool int_list_remove_by_value(List *list, intptr_t value)
{
    return list_remove_by_value(list, (void*)value);
}

void* list_get(List *list, int index)
{
    if (index < 0 || index >= list->size) 
        return NULL;
    return list->_array[index];
}

intptr_t int_list_get(List *list, int index) 
{
    return (intptr_t)list_get(list, index);
}

int list_get_size(List *list)
{
    if (list == NULL)
    {
        return UNDEFINED;
    }
    return list->size;
}

bool list_exists(List *list, void *value)
{
    if (list == NULL) return false;
    
    for (int i = 0; i < list->size; i++)
    {
        if (list->_array[i] == value)
        {
            return true;
        }
    }

    return false;
}

bool int_list_exists(List *list, intptr_t value)
{
    return list_exists(list, (void*)value);
}