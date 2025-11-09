#include "list.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>

// As simple as it gets, just needs to be initialized correctly
// - list_create
// - list_is_empty
// - list_get_len
// - list_clear
void create_and_clear_list(void)
{
    List my_cool_list = list_create();

    // verify no data 
    assert(my_cool_list.head == NULL);
    assert(my_cool_list.tail == NULL);
    assert(list_get_len(&my_cool_list) == 0);
    assert(list_is_empty(&my_cool_list));

    list_clear(&my_cool_list);

    assert(list_is_empty(&my_cool_list));
}

// Push back one entry, make sure it looks as expected
// tests:
// - list_create
// - list_push_back
// - list_is_empty
// - list_get_len
// - list_get_at_idx
// - list_clear
void push_back_one_entry(void)
{
    List my_cool_list = list_create();

    // verify no data 
    assert(my_cool_list.head == NULL);
    assert(my_cool_list.tail == NULL);
    assert(list_get_len(&my_cool_list) == 0);
    assert(list_is_empty(&my_cool_list));

    int test_data = 1337;

    list_push_back(&my_cool_list, &test_data);

    // should have data and be the same head/tail
    assert(my_cool_list.head != NULL);
    assert(my_cool_list.tail != NULL);
    assert(my_cool_list.head == my_cool_list.tail);
    assert(list_get_len(&my_cool_list) == 1);

    // pointer should be the same
    assert(my_cool_list.head->data == &test_data);
    // and consequently the value should be the same
    assert(*(int*)(my_cool_list.head->data) == test_data);
    assert(list_get_at_idx(&my_cool_list, 0) == &test_data);
    assert(list_get_at_idx(&my_cool_list, 1) == NULL);
    assert(!list_is_empty(&my_cool_list));
    // clear the list
    list_clear(&my_cool_list);

    // verify no data 
    assert(my_cool_list.head == NULL);
    assert(my_cool_list.tail == NULL);
    assert(list_is_empty(&my_cool_list));
}

// Push front one entry, make sure it looks as expected
// - list_create
// - list_push_front
// - list_is_empty
// - list_get_len
// - list_get_at_idx
// - list_clear
void push_front_one_entry(void)
{
    List my_cool_list = list_create();

    // verify no data 
    assert(my_cool_list.head == NULL);
    assert(my_cool_list.tail == NULL);
    assert(list_get_len(&my_cool_list) == 0);
    assert(list_is_empty(&my_cool_list));

    int test_data = 1337;

    list_push_front(&my_cool_list, &test_data);

    // should have data and be the same head/tail
    assert(my_cool_list.head != NULL);
    assert(my_cool_list.tail != NULL);
    assert(my_cool_list.head == my_cool_list.tail);
    assert(list_get_len(&my_cool_list) == 1);
    assert(!list_is_empty(&my_cool_list));

    // pointer should be the same
    assert(my_cool_list.head->data == &test_data);
    // and consequently the value should be the same
    assert(*(int*)(my_cool_list.head->data) == test_data);
    assert(list_get_at_idx(&my_cool_list, 0) == &test_data);
    assert(list_get_at_idx(&my_cool_list, 1) == NULL);
    // clear the list
    list_clear(&my_cool_list);

    // verify no data 
    assert(my_cool_list.head == NULL);
    assert(my_cool_list.tail == NULL);
    assert(list_is_empty(&my_cool_list));
}

// This is meant to be a full test of every function including the use of
// the ListItr iterator
// 
// Push back three entries. To cover the 3 states of nodes outside of single node
// lists. 3 states being, node at the front, node at the middle, and node at the end
//
// Afterwards remove one entry with `list_remove_at_idx` and then remove another
// with `list_remove_node`
//
// Lastly, push to the front the same 3 entries and verify the state of the list
// before detroying.
//
// - list_create
// - list_push_front
// - list_is_empty
// - list_get_len
// - list_get_at_idx
// - list_remove_at_idx
// - list_remove_node
// - list_itr_create
// - list_itr_next
// - list_clear
void push_back_three_remove_push_front_three_entries(void)
{
    List my_cool_list = list_create();

    // verify no data 
    assert(my_cool_list.head == NULL);
    assert(my_cool_list.tail == NULL);

    int test_data[3] = {0, 1, 2};

    list_push_back(&my_cool_list, &test_data[0]);

    // Make sure with one entry it looks right
    assert(my_cool_list.head == my_cool_list.tail);

    list_push_back(&my_cool_list, &test_data[1]);

    // Now with two it should be different
    assert(my_cool_list.head != my_cool_list.tail);

    list_push_back(&my_cool_list, &test_data[2]);

    // 3 entries?
    assert(list_get_len(&my_cool_list) == 3);

    // now use an iterator to examine each node
    ListItr list_itr = list_itr_create(&my_cool_list);
    ListNode* ln;
    int itr = 0;
    ListNode* prev_ln;
    while((ln = list_itr_next(&list_itr)))
    {
        assert(ln != NULL);
        assert(ln->data == &test_data[itr]);
        switch(itr)
        {
            case 0:
                assert(ln->prev == NULL);
                assert(ln->next != NULL);
                assert(ln->next->prev == ln);
                break;
            case 1:
                assert(ln->prev == prev_ln);
                assert(ln->prev != NULL);
                assert(ln->next != NULL);
                assert(ln->next->prev == ln);
                assert(ln->prev->next == ln);
                break;
            case 2:
                assert(ln->prev != NULL);
                assert(ln->next == NULL);
                assert(ln->prev->next == ln);
                assert(ln->prev == prev_ln);
                break;
            default:
                assert(false); // shouldn't get here
                break;
        }
        itr++;
        prev_ln = ln;
    }

    // remove the middle entry
    assert(list_remove_at_idx(&my_cool_list, 1));
    // can't remove entry 2 now (doesn't exist)
    assert(!list_remove_at_idx(&my_cool_list, 2));
    assert(list_get_len(&my_cool_list) == 2);

    list_itr = list_itr_create(&my_cool_list);
    ln = NULL;
    itr = 0;
    prev_ln = NULL;
    while((ln = list_itr_next(&list_itr)))
    {
        assert(ln != NULL);
        switch(itr)
        {
            case 0:
                assert(ln->prev == NULL);
                assert(ln->next != NULL);
                assert(ln->next->prev == ln);
                break;
            case 1:
                assert(ln->prev != NULL);
                assert(ln->next == NULL);
                assert(ln->prev->next == ln);
                assert(ln->prev == prev_ln);
                list_remove_node(&my_cool_list, ln);
                break;
            default:
                assert(false); // shouldn't get here
                break;
        }
        itr++;
        prev_ln = ln;
    }

    assert(list_get_len(&my_cool_list) == 1);
    assert(my_cool_list.head == my_cool_list.tail);

    // now push the same 3 pointers to the front of the list
    list_push_front(&my_cool_list, &test_data[0]);
    list_push_front(&my_cool_list, &test_data[1]);
    list_push_front(&my_cool_list, &test_data[2]);

    // now, the list should be in the order...
    // test_data[2] -> test_data[1] -> test_data[0] -> test_data[0]
    list_itr = list_itr_create(&my_cool_list);
    ln = NULL;
    itr = 0;
    prev_ln = NULL;
    while((ln = list_itr_next(&list_itr)))
    {
        assert(ln != NULL);
        switch(itr)
        {
            case 0:
                assert(ln->data == &test_data[2]);
                break;
            case 1:
                assert(ln->data == &test_data[1]);
                break;
            case 2:
                assert(ln->data == &test_data[0]);
                break;
            case 3:
                assert(ln->data == &test_data[0]);
                break;
            default:
                assert(false); // shouldn't get here
                break;
        }
        itr++;
        prev_ln = ln;
    }

    assert(list_get_len(&my_cool_list) == 4);

    // clear the list
    list_clear(&my_cool_list);

    // verify no data 
    assert(my_cool_list.head == NULL);
    assert(my_cool_list.tail == NULL);
    assert(list_is_empty(&my_cool_list));
}

int main(void)
{
    printf("Testing List Create and Clear.\n");
    create_and_clear_list();

    printf("Testing List Push Back.\n");
    push_back_one_entry();

    printf("Testing List Push Front.\n");
    push_front_one_entry();
    printf("Testing List Complete Exercise.\n");
    push_back_three_remove_push_front_three_entries();

    printf("-------------------------------------------------------------------------------\n");
    printf("List Tests Passed :)\n");
    printf("-------------------------------------------------------------------------------\n");

    return 0;
}
