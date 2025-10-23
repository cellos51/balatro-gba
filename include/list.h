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
} List;

typedef struct
{
    const List *p_list;
    ListNode *p_current_node;
} ListItr;

// add push back
// add iterator to fix while loop

// Reset the list head and tail to -1, this doesn't destroy the list, so if you call new you better make sure you destroy first
List list_new(void);
// Destroy the list and it's nodes
void list_destroy(List* p_list);
// Return true if empty, false if otherwise
bool list_is_empty(List list);
// Prepend a list node entry to the front of the list
int list_push_front(List *p_list, int elem_idx);
// append a list node entry to the back of the list
int list_push_back(List *p_list, int elem_idx);
// Remove a list node in a list. If it can't find the node, then it won't do nothing
void list_remove_node(List *p_list, ListNode *p_node);
// Get the node's `elem_idx` at the N'th node entry
int list_get_at_idx(List list, int n);
// Remove the node at the N'th node entry
void list_remove_at_idx(List *p_list, int n);
// Iterate through the list, if a node entry has the same elem_idx as passed, it returns what N'th thing the node is at
int list_get_at_object_idx(List list, int elem_idx);
// Remove at the matching elem_idx like above
void list_remove_at_object_idx(List *p_list, int elem_idx);
// Get the length of a list (number of entries or whatever)
int list_get_len(List list);

// List Iterator stuff
ListItr list_itr_new(const List* p_list);
ListNode* list_itr_next(ListItr* p_itr);
#endif
