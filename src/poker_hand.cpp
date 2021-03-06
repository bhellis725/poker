//
//  poker_hand.cpp
//  poker_calculator
//
//  Created by Ben Ellis on 7/15/16.
//  Copyright © 2016 Ben Ellis. All rights reserved.
//

#include <algorithm>
#include <memory>

#include "poker_hand.hpp"
#include "deck.hpp"
#include "misc.hpp"

const std::vector<std::string> HANDS_ = {"HIGH", "PAIR", "TWO PAIR", "THREE OF A KIND",
                                         "STRAIGHT", "FLUSH", "FULL HOUSE", "FOUR OF A KIND",
                                         "STRAIGHT FLUSH", "ROYAL FLUSH"};

PokerHand::PokerHand() : Deck(false) {
    score_ = -1;
    special_value_1_ = -1;
    special_value_2_ = -1;

};

PokerHand::PokerHand(const std::vector<Card>& cards) : Deck(false) {
    deck_ = cards;
    score_ = -1;
    special_value_1_ = -1;
    special_value_2_ = -1;
}

PokerHand::PokerHand(const std::vector<Card>&& cards) : Deck(false) {
    deck_ = cards;
    score_ = -1;
    special_value_1_ = -1;
    special_value_2_ = -1;
}

void PokerHand::print_all_hands() {
    std::vector<std::vector<Card>> all_hands = combinations(deck_, SCORE_SIZE_);
    std::cout << all_hands.size() << std::endl;
    for(int ihand = 0; ihand < all_hands.size(); ihand++) {
        std::cout << "hand " << ihand << std::endl;
        for(int icard = 0; icard < all_hands[ihand].size(); icard++) {
            std::cout << display_card(all_hands[ihand][icard]) << " ";
        }
        std::cout << std::endl;
    }
}

bool PokerHand::has_flush(const std::vector<int>& suits_count) {
    for(const int& x: suits_count) {
        if(x == 5) return true;
    }
    return false;
}

bool PokerHand::has_straight(const std::vector<int>& ranks_count) {
    for(const int& i: ranks_count) if(i > 1) return false;
    int diff = 5;
    if(has_ace() && deck_[0].get_rank() == 0) diff = 4;
    int start = std::find(std::begin(ranks_count), std::end(ranks_count), 1) - std::begin(ranks_count);
    if(start > ranks_count.size() - diff) return false;
    for(int i = 0; i < diff; ++i) {
        if(ranks_count[start] != 1) return false;
        start += 1;
    }
    return true;
}

void PokerHand::swap_ace_low() {
    if(has_ace() && deck_[0].get_rank() == 0) {
        //Card ace = cards.back();
        deck_.insert(std::begin(deck_), deck_.back());
        deck_.pop_back();
    }
}

bool PokerHand::has_ace() {
    for(const Card& c: deck_) {
        if(c.get_rank() == RANKS_.size() - 1) return true;
    }
    return false;
}

int PokerHand::has_pairs(const std::vector<int>& ranks_count) {
    int npairs = 0;
    std::vector<int> vals;
    for(int i = 0; i < ranks_count.size(); ++i) {
        if(ranks_count[i] == 2) {
            ++npairs;
            vals.push_back(i);
        }
    }
    if(vals.size() == 1) special_value_1_ = vals[0];
    else if(vals.size() == 2) {
        special_value_1_ = *std::max_element(std::begin(vals), std::end(vals));
        special_value_2_ = *std::min_element(std::begin(vals), std::end(vals));
    }
    return npairs;
}

bool PokerHand::has_three_kind(const std::vector<int>& ranks_count) {
    int pos = std::find(std::begin(ranks_count), std::end(ranks_count), 3) - std::begin(ranks_count);
    if(pos < ranks_count.size()) {
        special_value_1_ = pos;
        return true;
    }
    return false;
}

bool PokerHand::has_four_kind(const std::vector<int>& ranks_count) {
    int pos = std::find(std::begin(ranks_count), std::end(ranks_count), 4) - std::begin(ranks_count);
    if(pos < ranks_count.size()) {
        special_value_1_ = pos;
        return true;
    }
    return false;
}

bool PokerHand::has_full_house(const std::vector<int>& ranks_count) {
    int pos_3 = std::find(std::begin(ranks_count), std::end(ranks_count), 3) - std::begin(ranks_count);
    if(pos_3 >= ranks_count.size()) return false;
    int pos_2 = std::find(std::begin(ranks_count), std::end(ranks_count), 2) - std::begin(ranks_count);
    if(pos_2 >= ranks_count.size()) return false;
    special_value_1_ = pos_3;
    special_value_2_ = pos_2;
    return true;
}

void PokerHand::score_hand() {
    if(score_ < 0) {
        sort();
        std::vector<int> ranks_count (RANKS_.size(), 0);
        std::vector<int> suits_count (SUITS_.size(), 0);
        enumerate(ranks_count, suits_count);
        bool flush = has_flush(suits_count);
        bool straight = has_straight(ranks_count);
        
        //int pairs = has_pairs(ranks_count);
        //bool three_kind = has_three_kind(ranks_count);
        //bool four_kind = has_four_kind(ranks_count);
        if(flush && straight && deck_[0].get_rank() > 0) {
            if(set_hand_score("ROYAL FLUSH")) return;
            else {
                std::cout << "ERROR SETTING ROYAL FLUSH" << std::endl;
                std::exit(0);
            }
        } else if(flush && straight) {
            swap_ace_low();
            if(set_hand_score("STRAIGHT FLUSH")) return;
            else {
                std::cout << "ERROR SETTING STRAIGHT FLUSH" << std::endl;
                std::exit(0);
            }
        } else if(has_four_kind(ranks_count)) {
            if(set_hand_score("FOUR OF A KIND")) return;
            else {
                std::cout << "ERROR SETTING FOUR OF A KIND" << std::endl;
                std::exit(0);
            }
        } else if(has_full_house(ranks_count)) {
            if(set_hand_score("FULL HOUSE")) return;
            else {
                std::cout << "ERROR SETTING FULL HOUSE" << std::endl;
                std::exit(0);
            }
        } else if(flush) {
            if(set_hand_score("FLUSH")) return;
            else {
                std::cout << "ERROR SETTING FLUSH" << std::endl;
                std::exit(0);
            }
        } else if(straight) {
            swap_ace_low();
            if(set_hand_score("STRAIGHT")) return;
            else {
                std::cout << "ERROR SETTING STRAIGHT" << std::endl;
                std::exit(0);
            }
        } else if(has_three_kind(ranks_count)) {
            if(set_hand_score("THREE OF A KIND")) return;
            else {
                std::cout << "ERROR SETTING THREE OF A KIND" << std::endl;
                std::exit(0);
            }
        } else if(has_pairs(ranks_count) == 2) {
            if(set_hand_score("TWO PAIR")) return;
            else {
                std::cout << "ERROR SETTING TWO PAIR" << std::endl;
                std::exit(0);
            }
        } else if(has_pairs(ranks_count) == 1) {
            if(set_hand_score("PAIR")) return;
            else {
                std::cout << "ERROR SETTING PAIR" << std::endl;
                std::exit(0);
            }
        } else {
            if(set_hand_score("HIGH")) return;
            else {
                std::cout << "ERROR SETTING HIGH" << std::endl;
                std::exit(0);
            }
        }
    }
    std::cout << "hand is already scored!" << std::endl;
}

std::string PokerHand::show_score() {
    if(score_ >= 0) return HANDS_[score_];
    std::string not_scored = "hand not scored yet";
    return not_scored;
}

bool PokerHand::set_hand_score(const std::string& hand) {
    long idx = std::find(std::begin(HANDS_), std::end(HANDS_), hand) - std::begin(HANDS_);
    if (idx <= HANDS_.size() && idx >= 0) {
        score_ = idx;
        return true;
    }
    return false;
}

/*
bool PokerHand::has_royal_flush(const std::vector<Card>& cards) {
    return has_straight_flush(cards) && cards[0].get_rank() > 0 && has_ace(cards);
}
*/

/*
bool PokerHand::has_straight_flush(const std::vector<Card>& cards) {
    return has_straight(cards) && has_flush(cards);
}
*/

/*
int PokerHand::has_x_of_a_kind(const int& x,const std::vector<Card>& cards) {
    std::vector<std::vector<Card>> groups_of_x = combinations(cards, x);
    for(const std::vector<Card>& x_hand: groups_of_x) {
        int rank = x_hand[0].get_rank();
        bool has_it = true;
        for(const Card& c: x_hand) {
            if(c.get_rank() != rank) {
                has_it = false;
                break;
            }
        }
        if(has_it) {
            return rank;
        }
    }
    return -1;
}
*/

/*
std::pair<int, int> PokerHand::has_x_y_of_a_kind(const int& x, const int& y, const std::vector<Card>& cards) {
    std::pair<int, int> result (-1, -1);
    std::vector<std::vector<Card>> groups_of_x = combinations(cards, x);
    for(const std::vector<Card>& x_hand: groups_of_x) {
        std::vector<Card> y_hand;
        for(const Card& card: cards) {
            if(std::find(std::begin(x_hand), std::end(x_hand), card) == std::end(x_hand)) y_hand.push_back(card);
        }
        int x_hand_result = has_x_of_a_kind(x, x_hand);
        int y_hand_result = has_x_of_a_kind(y, y_hand);
        if(x_hand_result >= 0 && y_hand_result >= 0) {
            result.first = x_hand_result;
            result.second = y_hand_result;
            return result;
        }
    }
    return result;
}
*/

bool PokerHand::operator>(const PokerHand& other) {
    if(score_ > other.score_) return true;
    if(score_ == other.score_) {
        if(show_score() == "STRAIGHT FLUSH") {
            for(long i = deck_.size() - 1; i >= 0; i--) {
                if(deck_[i] > other.deck_[i]) return true;
            }
            return false;
        }
        if(show_score() == "FOUR OF A KIND") {
            if(special_value_1_ > other.special_value_1_) return true;
            if(special_value_1_ < other.special_value_1_) return false;
            int this_val = -1; int other_val = -1;
            for(long i = deck_.size() - 1; i >= 0; --i) {
                if(deck_[i].get_rank() != special_value_1_) this_val = deck_[i].get_rank();
                if(other.deck_[i].get_rank() != other.special_value_1_) other_val = other.deck_[i].get_rank();
            }
            return this_val > other_val;
        }
        if(show_score() == "FULL HOUSE") {
            if(special_value_1_ > other.special_value_1_) return true;
            //if(special_value_1_ < other.special_value_1_) return false;
            if(special_value_1_ == other.special_value_1_ && special_value_2_ > other.special_value_2_) return true;
            //if(special_value_1_ == other.special_value_1_ && special_value_2_ < other.special_value_2_) return false;
            return false;
            //int val = -1;
            //int val_other = -1;
            //for(int i = deck_.size(); i >= 0; i--) {
            //    if(deck_[i].get_rank() != special_value_1_ ||
            //       deck_[i].get_rank() != special_value_2_) val = deck_[i].get_rank();
            //    if(other.deck_[i].get_rank() != other.special_value_1_ ||
            //       other.deck_[i].get_rank() != other.special_value_2_) val_other = other.deck_[i].get_rank();
            //}
            //return val > val_other;
        }
        if(show_score() == "FLUSH") {
            for(long i = deck_.size() - 1; i >= 0; --i) {
                if(deck_[i] > other.deck_[i]) return true;
            }
            return false;
        }
        if(show_score() == "STRAIGHT") {
            for(long i = deck_.size() - 1; i >= 0; --i) {
                if(deck_[i] > other.deck_[i]) return true;
            }
            return false;
        }
        if(show_score() == "THREE OF A KIND") {
            if(special_value_1_ > other.special_value_1_) return true;
            if(special_value_1_ < other.special_value_1_) return false;
            std::vector<int> this_vals;
            std::vector<int> other_vals;
            for(long i = deck_.size() - 1; i >=0; --i) {
                if(deck_[i].get_rank() != special_value_1_) this_vals.push_back(deck_[i].get_rank());
                if(other.deck_[i].get_rank() != other.special_value_1_) other_vals.push_back(other.deck_[i].get_rank());
            }
            std::sort(std::begin(this_vals), std::end(this_vals));
            std::sort(std::begin(other_vals), std::end(other_vals));
            if(this_vals.back() > other_vals.back()) return true;
            if(this_vals.back() == other_vals.back() && this_vals.front() > other_vals.front()) return true;
            return false;
        }
        if(show_score() == "TWO PAIR") {
            if(special_value_1_ > other.special_value_1_) return true;
            if(special_value_1_ < other.special_value_1_) return false;
            if(special_value_2_ > other.special_value_2_) return true;
            if(special_value_2_ < other.special_value_2_) return false;
            int this_val = -1; int other_val = -1;
            for(long i = deck_.size() - 1; i >=0; --i) {
                if(deck_[i].get_rank() != special_value_1_ &&
                   deck_[i].get_rank() != special_value_2_) this_val = deck_[i].get_rank();
                if(other.deck_[i].get_rank() != other.special_value_1_ &&
                   other.deck_[i].get_rank() != other.special_value_2_) other_val = other.deck_[i].get_rank();
            }
            if(this_val > other_val) return true;
            return false;
        }
        if(show_score() == "PAIR") {
            if(special_value_1_ > other.special_value_1_) return true;
            if(special_value_1_ < other.special_value_1_) return false;
            std::vector<int> this_vals;
            std::vector<int> other_vals;
            for(long i = deck_.size() - 1; i >=0; --i) {
                if(deck_[i].get_rank() != special_value_1_) this_vals.push_back(deck_[i].get_rank());
                if(other.deck_[i].get_rank() != other.special_value_1_) other_vals.push_back(other.deck_[i].get_rank());
            }
            std::sort(std::begin(this_vals), std::end(this_vals));
            std::sort(std::begin(other_vals), std::end(other_vals));
            for(long i = this_vals.size() - 1; i >= 0; --i) {
                if(this_vals[i] > other_vals[i]) return true;
            }
            return false;
        }
        if(show_score() == "HIGH") {
            for(long i = deck_.size() - 1; i >= 0; --i) {
                // std::cout << deck_[i].get_rank() << " " << other.deck_[i].get_rank() << std::endl;
                if(deck_[i] > other.deck_[i]) return true;
            }
            return false;
        }
    }
    return false;
}

bool PokerHand::operator<(const PokerHand &other) {
    return !(*this > other);
}

bool PokerHand::operator==(const PokerHand &other) {
    //return !(*this > other) && !(*this < other);
    if(score_ != other.score_) return false;
    for(int i = 0; i < deck_.size(); ++i) {
        if(deck_[i] != other.deck_[i]) return false;
    }
    return true;
}
                  
bool PokerHand::operator!=(const PokerHand& other) {
    return !(*this == other);
}

bool PokerHand::operator<=(const PokerHand &other) {
    return *this < other || *this == other;
}

bool PokerHand::operator>=(const PokerHand &other) {
    return *this > other || *this == other;
}

void PokerHand::enumerate(std::vector<int>& ranks, std::vector<int>& suits) {
    //std::vector<int> ranks (RANKS_.size(), 0);
    //std::vector<int> suits (SUITS_.size(), 0);
    for(const Card& c: deck_) {
        ranks[c.get_rank()] += 1;
        suits[c.get_suit()] += 1;
    }
}



