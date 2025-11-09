#include <stdbool.h>
#include <stdint.h>
#include "list.h"
#include "pool.h"

List list_declare(void)
{
    List list = { .head = NULL, .tail = NULL, .len = 0 };
    return list;
}

void list_clear(List* list)
{
    if(list_is_empty(list)) return;

    ListItr itr = list_itr_declare(list);
    ListNode* ln;

    while((ln = list_itr_next(&itr)))
    {
       POOL_FREE(ListNode, ln);
    }

    list->head = NULL;
    list->tail = NULL;
    list->len = 0;
}

bool list_is_empty(const List* list)
{
    return list->len == 0;
}

void list_push_front(List *list, void* data)
{
    ListNode *node = POOL_GET(ListNode);

    node->data = data;
    node->prev = NULL;
    node->next = list->head;

    if (list_is_empty(list))
    {
        list->tail = node;
    }
    else
    {
        list->head->prev = node;
    }

    list->head = node;

    list->len++;
}

void list_push_back(List *list, void* data)
{
    ListNode* node = POOL_GET(ListNode);
    node->data = data;
    node->prev = list->tail;
    node->next = NULL;

    if (list_is_empty(list))
    {
        list->head = node;
    }
    else
    {
        list->tail->next = node;
    }

    list->tail = node;

    list->len++;
}

void list_remove_node(List *list, ListNode *node)
{
    if(node->prev && !node->next) // end of list
    {
        node->prev->next = NULL;
        list->tail = node->prev;
    }
    else if(node->prev && node->next) // somewhere in between
    {
        node->prev->next = node->next;
        node->next->prev = node->prev;
    }
    else if(node->next && !node->prev) // beginning of list
    {
        node->next->prev = NULL;
        list->head = node->next;
    }
    else if(!node->prev && !node->next) // only element in list
    {
        list->head = NULL;
        list->tail = NULL;
    }

    POOL_FREE(ListNode, node);

    list->len--;
}

ListItr list_itr_declare(const List* list)
{
    ListItr itr =
    {
        .list = list,
        .current_node = !list_is_empty(list) ? list->head : NULL,
    };

    return itr;
}

ListNode* list_itr_next(ListItr* itr)
{
    if(!itr->current_node) return NULL;

    ListNode* ln = itr->current_node;

    if(ln->next)
    {
        itr->current_node = ln->next;
        return ln;
    }

    itr->current_node = NULL;
    return ln;
}

int list_get_len(const List* list)
{
    return list->len;
}

void* list_get_at_idx(const List* list, int n)
{
    if(n >= list_get_len(list) || n < 0) return NULL;

    int curr_idx = 0;
    ListItr itr = list_itr_declare(list);
    ListNode* ln;

    while((ln = list_itr_next(&itr)))
    {
        if (n == curr_idx++) return ln->data;
    }

    return NULL;
}

bool list_remove_at_idx(List* list, int n)
{
    if(n >= list_get_len(list) || n < 0) return false;

    int len = 0;
    ListItr itr = list_itr_declare(list);
    ListNode* ln;

    while((ln = list_itr_next(&itr)))
    {
        if(n == len++)
        {
            list_remove_node(list, ln);
            return true;
        }
    }
    return false;
}
