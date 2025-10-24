/** @file list.h
 *
 *  @brief A doubly-linked list designed to be used with this projects corresponding memory pools.
 *
 *  List Implementation
 *  ===================
 *
 *  - This @ref List operates as a linked list @ref ListNodes. It operates as a regular doubly-linked list
 *  but uses integers as offsets into memory pools. 
 */ 
#ifndef LIST_H
#define LIST_H

#include <stdbool.h>
#include <stdint.h>

/*
typedef struct
{
    int prev;       // link-node idx, or -1 if at tail of list
    int next;       // link-node idx, or -1 if at head of list
    int elem_idx;   // index into T_pool
} ListNode;
*/

union ListData
{
    int val;
    void* ptr;
};

typedef struct ListNode ListNode;
struct ListNode
{
    ListNode* prev;
    ListNode* next;
    union ListData data;
};

/*
typedef struct
{
    int head; // link-node idx or -1, or -1 if nothing in list
    int tail;
} List;
*/
typedef struct
{
    ListNode* head;
    ListNode* tail;
} List;

typedef struct
{
    const List *p_list;
    ListNode *p_current_node;
} ListItr;

// add push back
// add iterator to fix while loop

/**
 * Create a new list.
 *
 * @return A List struct with head and tail reset.
 */
List list_new(void);
// Destroy the list and it's nodes
void list_destroy(List* p_list);
// Return true if empty, false if otherwise
bool list_is_empty(List list);
// Prepend a list node entry to the front of the list
void list_push_front(List *p_list, union ListData data);
// append a list node entry to the back of the list
void list_push_back(List *p_list, union ListData data);
// Remove a list node in a list. If it can't find the node, then it won't do nothing
void list_remove_node(List *p_list, ListNode *p_node);


// Get the node's `elem_idx` at the N'th node entry
union ListData list_get_at_idx(List list, int n);
// Remove the node at the N'th node entry
void list_remove_at_idx(List *p_list, int n);
// Iterate through the list, if a node entry has the same elem_idx as passed, it returns what N'th thing the node is at
int list_get_at_object_idx(List list, union ListData data);
// Remove at the matching elem_idx like above
void list_remove_at_object_idx(List *p_list, union ListData data);
// Get the length of a list (number of entries or whatever)
int list_get_len(List list);

// List Iterator stuff
ListItr list_itr_new(const List* p_list);
ListNode* list_itr_next(ListItr* p_itr);

inline union ListData list_data_ptr(void* ptr)
{
    union ListData data =
    {
        .ptr = ptr
    };

    return data;
}

inline union ListData list_data_int(int val)
{
    union ListData data =
    {
        .val = val
    };

    return data;
}

#endif
