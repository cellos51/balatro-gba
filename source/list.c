#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include "list.h"
#include "pool.h"
#include "util.h"

// Good!
List list_new(void)
{
    List head = { .head = NULL, .tail = NULL};
    return head;
}

// Good!
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

// Good!
bool list_is_empty(List list)
{
    return list.head == NULL;
}

// Good!
void list_push_front(List *p_list, void* data)
{
    ListNode *p_node = POOL_GET(ListNode);

    p_node->data = data;
    p_node->prev = NULL;
    p_node->next = p_list->head;

    if (list_is_empty(*p_list))
    {
        p_list->tail = p_node;
    }
    else
    {
        p_list->head->prev = p_node;
    }

    p_list->head = p_node;
}

// Good!
void list_push_back(List *p_list, void* data)
{
    ListNode *p_node = POOL_GET(ListNode);
    p_node->data = data;
    p_node->prev = p_list->tail;
    p_node->next = NULL;

    if (list_is_empty(*p_list))
    {
        p_list->head = p_node;
    }
    else
    {
        p_list->tail->next = p_node;
    }

    p_list->tail = p_node;
}

// Good!
void list_remove_node(List *p_list, ListNode *p_node)
{
    if(p_node->prev && !p_node->next) // end of list
    {
        p_node->prev->next = NULL;
        p_list->tail = p_node->prev;
    }
    else if(p_node->prev && p_node->next) // somewhere in between
    {
        p_node->prev->next = p_node->next;
        p_node->next->prev = p_node->prev;
    }
    else if(p_node->next && !p_node->prev) // beginning of list
    {
        p_node->next->prev = NULL;
        p_list->head = p_node->next;
    }
    else if(!p_node->prev && !p_node->next) // only element in list
    {
        p_list->head = NULL;
        p_list->tail = NULL;
    }

    POOL_FREE(ListNode, p_node);
    /*
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
    */
}

// Good!
ListItr list_itr_new(const List* p_list)
{
    ListItr itr =
    {
        .p_list = p_list,
        .p_current_node = !list_is_empty(*p_list) ? p_list->head : NULL,
    };

    return itr;
}

// Good!
ListNode* list_itr_next(ListItr* p_itr)
{
    ListNode* ln = p_itr->p_current_node;
    if(!p_itr->p_current_node) { return NULL; };
    if(ln->next)
    {
        //p_itr->p_current_node = POOL_AT(ListNode, ln->next);
        p_itr->p_current_node = ln->next;
        return ln;
    }

    p_itr->p_current_node = NULL;
    return ln;
}

// Good!
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

// Good!
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

// Good!
void* list_get_at_idx(List list, int n)
{
    int len = 0;
    ListItr itr = list_itr_new(&list);
    ListNode* ln;

    while((ln = list_itr_next(&itr)))
    {
        if (n == len++) return ln->data;
    }

    return NULL;
}

// Good!
int list_get_at_object_idx(List list, void* data)
{
    int len = 0;
    ListItr itr = list_itr_new(&list);
    ListNode* ln;

    while((ln = list_itr_next(&itr)))
    {
        if(ln->data == data) return len;
        len++;
    }

    return UNDEFINED;
}

// Good!
void list_remove_at_object_idx(List* p_list, void* data)
{
    ListItr itr = list_itr_new(p_list);
    ListNode* ln;

    while((ln = list_itr_next(&itr)))
    {
        if(ln->data == data)
        {
            list_remove_node(p_list, ln);
            return;
        }
    }
}
