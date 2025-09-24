#ifndef LIST_H
#define LIST_H

#include <stdbool.h>
#include <stdint.h>

typedef struct
{
    int prev;       // link-node idx, or -1 if at tail of list
    int next;       // link-node idx, or -1 if at head of list
    int elem_idx;   // index into T_pool
} ListNode;

typedef struct
{
    int head; // link-node idx or -1, or -1 if nothing in list
    int tail;
} ListHead;

typedef struct
{
    const ListHead *p_list;
    ListNode *p_current_node;
} ListItr;

// add push back
// add iterator to fix while loop

ListHead list_new(void);
void list_destroy(ListHead* p_list);
bool list_is_empty(ListHead list);
int list_push_front(ListHead *p_list, int elem_idx);
int list_push_back(ListHead *p_list, int elem_idx);
int list_get_at_idx(ListHead list, int elem_idx);
void list_remove_node(ListHead *p_list, ListNode *p_node);
ListItr list_itr_new(const ListHead* p_list);
ListNode* list_itr_next(ListItr* p_itr);
void list_remove_at_idx(ListHead *p_list, int elem_idx);
int list_get_len(ListHead list);

int list_has_idx(ListHead list, int idx);

#endif
