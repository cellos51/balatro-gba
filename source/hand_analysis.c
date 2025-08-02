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
    if (!is_shortcut_joker_active())
    {
        // This is the regular case of detecting straights
        for (int i = 0; i < NUM_RANKS - 4; i++)
        {
            if (ranks[i] && ranks[i + 1] && ranks[i + 2] && ranks[i + 3] && ranks[i + 4])
                return true;
        }
        // Check for ace low straight
        if (ranks[ACE] && ranks[TWO] && ranks[THREE] && ranks[FOUR] && ranks[FIVE])
            return true;
    } else
    {
        // Shortcut Joker is active, we have to detect straights where any card may "skip" 1 rank
        // We do this by calculating the longest possible straight that can end on each rank
        // and stopping when we find one that is 5 cards long
        u8 longest_short_cut_at[NUM_RANKS] = {0};

        // A low ace can start a sequence. 'ace_low_len' is 1 if an ace is present,
        // acting as a potential predecessor for TWO and THREE.
        int ace_low_len = ranks[ACE] ? 1 : 0;

        // Iterate through all ranks from TWO up to ACE.
        for (int i = 0; i < NUM_RANKS; i++)
        {
            // No cards in this rank, no straight can end here, continue
            if (ranks[i] == 0)
            {
                longest_short_cut_at[i] = 0;
                continue;
            }

            int prev_len1 = 0;
            int prev_len2 = 0;

            // This logic handles the special connections for ace-low straights.
            if (i == TWO)
            {
                // A TWO can be preceded by a low ACE (no skip).
                prev_len1 = ace_low_len;
            }
            else if (i == THREE)
            {
                // A THREE can be preceded by a TWO (no skip) or a low ACE (skip).
                prev_len1 = longest_short_cut_at[TWO];
                prev_len2 = ace_low_len;
            }
            else if (i == ACE)
            {
                // An ACE (as the highest card) can be preceded by a KING or a QUEEN.
                prev_len1 = longest_short_cut_at[KING];
                prev_len2 = longest_short_cut_at[QUEEN];
            }
            else // For all other cards (FOUR through KING).
            {
                // A card can be preceded by the rank directly below or two ranks below.
                prev_len1 = longest_short_cut_at[i - 1];
                prev_len2 = longest_short_cut_at[i - 2];
            }

            // The length of the straight ending at rank 'i' is 1 (for the card itself)
            // plus the length of the longest valid preceding straight.
            if (prev_len1 > prev_len2)
                longest_short_cut_at[i] = 1 + prev_len1;
            else
                longest_short_cut_at[i] = 1 + prev_len2;

            // If we've formed a sequence of 5 or more cards, we have a straight.
            if (longest_short_cut_at[i] >= 5)
            {
                return true;
            }
        }
    }

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
