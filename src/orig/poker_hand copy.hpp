//
//  poker_hand.hpp
//  poker_calculator
//
//  Created by Ben Ellis on 7/15/16.
//  Copyright © 2016 Ben Ellis. All rights reserved.
//

#ifndef poker_hand_hpp
#define poker_hand_hpp

#include <iostream>
#include <vector>

#include "deck.hpp"

class PokerHand : public Deck
{
public:
    PokerHand();
    PokerHand(const bool& fill);
    PokerHand(const std::vector<Card>& cards);
    void print_all_hands();
    void score_hand();
    std::string show_score();
    bool operator>(const PokerHand& other);
    bool operator<(const PokerHand& other);
    bool operator==(const PokerHand& other);
    bool operator>=(const PokerHand& other);
    bool operator<=(const PokerHand& other);
private:
    static const int SCORE_SIZE_ = 5;
    int score_;
    int special_value_1_;
    int special_value_2_;
    static bool has_flush(const std::vector<Card>& cards);
    static bool has_straight(const std::vector<Card>& cards);
    static bool has_royal_flush(const std::vector<Card>& cards);
    static bool has_straight_flush(const std::vector<Card>& cards);
    static bool has_ace(const std::vector<Card>& cards);
    static int has_x_of_a_kind(const int& x, const std::vector<Card>& cards);
    static std::pair<int, int> has_x_y_of_a_kind(const int& x, const int& y, const std::vector<Card>& cards);
    bool set_hand_score(const std::string& hand);
    static void swap_ace_low(std::vector<Card>& cards);
    void enumerate();
};

#endif /* poker_hand_hpp */
