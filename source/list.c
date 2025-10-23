#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include "list.h"
#include "pool.h"

List list_new(void)
{
    List head = { .head = -1, .tail = -1};
    return head;
}

void list_destroy(List* p_list)
{
    if(list_is_empty(*p_list)) return;

    ListItr itr = list_itr_new(p_list);
    ListNode* ln;

    while((ln = list_itr_next(&itr)))
    {
       POOL_FREE(ListNode, ln);
    }
}

bool list_is_empty(List list)
{
    return list.head < 0;
}

int list_push_front(List *p_list, int elem_idx)
{
    ListNode *p_node = POOL_GET(ListNode);
    int n = POOL_IDX(ListNode, p_node); // Get the index of of the node in the pool

    p_node->elem_idx = elem_idx;
    p_node->prev = -1;
    p_node->next = p_list->head;

    if (list_is_empty(*p_list))
    {
        p_list->tail = n;
    }
    else
    {
        POOL_AT(ListNode, p_list->head)->prev = n;
    }

    p_list->head = n;

    return n;
}

int list_push_back(List *p_list, int elem_idx)
{
    ListNode *p_node = POOL_GET(ListNode);
    int n = POOL_IDX(ListNode, p_node); // Get the index of of the node in the pool
    p_node->elem_idx = elem_idx;
    p_node->prev = p_list->tail;
    p_node->next = -1;

    if (list_is_empty(*p_list))
    {
        p_list->head = n;
    }
    else
    {
        POOL_AT(ListNode, p_list->tail)->next = n;
    }

    p_list->tail = n;

    return n;
}

void list_remove_node(List *p_list, ListNode *p_node)
{
    ListNode* p_prev_node = NULL;
    ListNode* p_next_node = NULL;

    if(p_node->prev >= 0)
    {
        p_prev_node = POOL_AT(ListNode, p_node->prev);
    }

    if(p_node->next >= 0)
    {
        p_next_node = POOL_AT(ListNode, p_node->next);
    }

    if(p_prev_node && !p_next_node) // end of list
    {
        p_prev_node->next = -1;
        p_list->tail = POOL_IDX(ListNode, p_prev_node);
    }
    else if(p_prev_node && p_next_node) // somewhere in between
    {
        p_prev_node->next = POOL_IDX(ListNode, p_next_node);
        p_next_node->prev = POOL_IDX(ListNode, p_prev_node);
    }
    else if(p_next_node && !p_prev_node) // beginning of list
    {
        p_next_node->prev = -1;
        p_list->head = POOL_IDX(ListNode, p_next_node);
    }
    else if(!p_prev_node && !p_next_node) // only element in list
    {
        p_list->head = -1;
        p_list->tail = -1;
    }

    POOL_FREE(ListNode, p_node);
}

ListItr list_itr_new(const List* p_list)
{
    ListItr itr =
    {
        .p_list = p_list,
        .p_current_node = !list_is_empty(*p_list) ? POOL_AT(ListNode, p_list->head) : NULL,
    };

    return itr;
}

ListNode* list_itr_next(ListItr* p_itr)
{
    ListNode* ln = p_itr->p_current_node;
    if(!p_itr->p_current_node) { return NULL; };
    if(ln->next >= 0)
    {
        p_itr->p_current_node = POOL_AT(ListNode, ln->next);
        return ln;
    }

    p_itr->p_current_node = NULL;
    return ln;
}

void list_remove_at_idx(List* p_list, int n)
{
    int len = 0;
    ListItr itr = list_itr_new(p_list);
    ListNode* ln;

    while((ln = list_itr_next(&itr)))
    {
        if(n == len++)
        {
            list_remove_node(p_list, ln);
            return;
        }
    }
}

int list_get_len(List list)
{
    int len = 0;
    ListItr itr = list_itr_new(&list);
    ListNode* ln;

    while((ln = list_itr_next(&itr)))
    {
        len++;
    }

    return len;
}

int list_get_at_idx(List list, int n)
{
    int len = 0;
    ListItr itr = list_itr_new(&list);
    ListNode* ln;

    while((ln = list_itr_next(&itr)))
    {
        if (n == len++) return ln->elem_idx;
    }

    return -1;
}

bool list_exists(List list, int idx)
{
    if (list_is_empty(list)) return false;
    
    ListItr itr = list_itr_new(&list);
    ListNode* ln;

    while((ln = list_itr_next(&itr)))
    {
        if(ln->elem_idx == idx) return true;
    }

    return false;
}


int list_get_at_object_idx(List list, int elem_idx)
{
    int len = 0;
    ListItr itr = list_itr_new(&list);
    ListNode* ln;

    while((ln = list_itr_next(&itr)))
    {
        if(ln->elem_idx == elem_idx) return len;
        len++;
    }

    return -1;
}

void list_remove_at_object_idx(List* p_list, int elem_idx)
{
    ListItr itr = list_itr_new(p_list);
    ListNode* ln;

    while((ln = list_itr_next(&itr)))
    {
        if(ln->elem_idx == elem_idx)
        {
            list_remove_node(p_list, ln);
            return;
        }
    }
}
