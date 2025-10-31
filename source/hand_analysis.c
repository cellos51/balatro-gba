#include "hand_analysis.h"
#include "card.h"
#include "game.h"

static void get_distribution(CardObject **cards, int top, u8 *ranks_out, u8 *suits_out) {
    for (int i = 0; i < NUM_RANKS; i++) ranks_out[i] = 0;
    for (int i = 0; i < NUM_SUITS; i++) suits_out[i] = 0;

    for (int i = 0; i <= top; i++) {
        if (cards[i] && card_object_is_selected(cards[i])) {
            ranks_out[cards[i]->card->rank]++;
            suits_out[cards[i]->card->suit]++;
        }
    }
}

void get_hand_distribution(u8 *ranks_out, u8 *suits_out) {
    get_distribution(get_hand_array(), get_hand_top(), ranks_out, suits_out);
}

void get_played_distribution(u8 *ranks_out, u8 *suits_out) {
    get_distribution(get_played_array(), get_played_top(), ranks_out, suits_out);
}

// Returns the highest N of a kind. So a full-house would return 3.
u8 hand_contains_n_of_a_kind(u8 *ranks) {
    u8 highest_n = 0;
    for (int i = 0; i < NUM_RANKS; i++) {
        if (ranks[i] > highest_n)
            highest_n = ranks[i];
    }
    return highest_n;
}

bool hand_contains_two_pair(u8 *ranks) {
    bool contains_other_pair = false;
    for (int i = 0; i < NUM_RANKS; i++) {
        if (ranks[i] >= 2) {
            if (contains_other_pair)
                return true;
            contains_other_pair = true;
        }
    }
    return false;
}

bool hand_contains_full_house(u8* ranks) {
    int count_three = 0;
    int count_pair = 0;
    for (int i = 0; i < NUM_RANKS; i++) {
        if (ranks[i] >= 3) {
            count_three++;
        }
        else if (ranks[i] >= 2) {
            count_pair++;
        }
    }
    // Full house if there is:
    // - at least one three-of-a-kind and at least one other pair,
    // - OR at least two three-of-a-kinds (second "three" acts as pair).
    // This accounts for hands with 6 or more cards even though
    // they are currently not possible and probably never will be.
    return (count_three >= 2 || (count_three && count_pair));
}

bool hand_contains_straight(u8 *ranks) {
    for (int i = 0; i < NUM_RANKS - 4; i++)
    {
        if (ranks[i] && ranks[i + 1] && ranks[i + 2] && ranks[i + 3] && ranks[i + 4])
            return true;
    }
    // Check for ace low straight
    if (ranks[ACE] && ranks[TWO] && ranks[THREE] && ranks[FOUR] && ranks[FIVE])
        return true;

    return false;
}

bool hand_contains_flush(u8 *suits) {
    for (int i = 0; i < NUM_SUITS; i++)
    {
        if (suits[i] >= MAX_SELECTION_SIZE) // this allows MAX_SELECTION_SIZE - 1 for four fingers joker
        {
            return true;
        }
    }
    return false;
}
