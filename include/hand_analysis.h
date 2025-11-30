#ifndef HAND_ANALYSIS_H
#define HAND_ANALYSIS_H

#include "card.h"

#include <tonc.h>

void get_hand_distribution(u8* ranks_out, u8* suits_out);
void get_played_distribution(u8* ranks_out, u8* suits_out);

u8 hand_contains_n_of_a_kind(u8* ranks);
bool hand_contains_two_pair(u8* ranks);
bool hand_contains_full_house(u8* ranks);
bool hand_contains_straight(u8* ranks);
bool hand_contains_flush(u8* suits);

int find_flush_in_played_cards(CardObject** played, int top, int min_len, bool* out_selection);
int find_straight_in_played_cards(
    CardObject** played,
    int top,
    bool shortcut_active,
    int min_len,
    bool* out_selection
);
void select_paired_cards_in_hand(CardObject** played, int top, bool* selection);

#endif
