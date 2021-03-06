//
//  poker_hand.cpp
//  poker_calculator
//
//  Created by Ben Ellis on 7/15/16.
//  Copyright © 2016 Ben Ellis. All rights reserved.
//

#include <algorithm>

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

PokerHand::PokerHand(const std::vector<Card>& cards) {
    deck_ = cards;
    sort();
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

bool PokerHand::has_flush(const std::vector<Card>& cards) {
    int suit = cards[0].get_suit();
    for(const Card& c: cards) {
        if(c.get_suit() != suit) return false;
    }
    return true;
}

bool PokerHand::has_straight(const std::vector<Card>& cards) {
    std::vector<Card>::const_iterator end_iter = std::end(cards);
    if(has_ace(cards) && cards[0].get_rank() == 0) end_iter -= 1;
    for(std::vector<Card>::const_iterator i = std::begin(cards); i != end_iter; i++) {
        std::vector<Card>::const_iterator j = i + 1;
        if(j == end_iter) break;
        if((*j).get_rank() - (*i).get_rank() != 1) {
            return false;
        }
    }
    return true;
}

void PokerHand::swap_ace_low(std::vector<Card>& cards) {
    if(has_ace(cards) && cards[0].get_rank() == 0) {
        //Card ace = cards.back();
        cards.insert(std::begin(cards), cards.back());
        cards.pop_back();
    }
}

bool PokerHand::has_ace(const std::vector<Card>& cards) {
    for(const Card& c: cards) {
        if(c.get_rank() == RANKS_.size() - 1) return true;
    }
    return false;
}

void PokerHand::score_hand() {
    if(score_ < 0) {
        sort();
        if(has_royal_flush(deck_)) {
            if(set_hand_score("ROYAL FLUSH")) return;
            std::cout << "error setting royal flush" << std::endl;
        }
        if(has_straight_flush(deck_)) {
            swap_ace_low(deck_);
            if(set_hand_score("STRAIGHT FLUSH")) return;
            std::cout << "error setting straight flush" << std::endl;
        }
        int four_kind_rank = has_x_of_a_kind(4, deck_);
        if(four_kind_rank >= 0) {
            special_value_1_ = four_kind_rank;
            if(set_hand_score("FOUR OF A KIND")) return;
            std::cout << "error setting four of a kind" << std::endl;
        }
        std::pair<int, int> full_house_ranks = has_x_y_of_a_kind(3, 2, deck_);
        if(full_house_ranks.first >= 0 && full_house_ranks.second >= 0) {
            special_value_1_ = full_house_ranks.first;
            special_value_2_ = full_house_ranks.second;
            if(set_hand_score("FULL HOUSE")) return;
            std::cout << "error setting full house" << std::endl;
        }
        if(has_flush(deck_)) {
            if(set_hand_score("FLUSH")) return;
            std::cout << "error setting flush" << std::endl;
        }
        if(has_straight(deck_)) {
            swap_ace_low(deck_);
            if(set_hand_score("STRAIGHT")) return;
            std::cout << "error setting straight" << std::endl;
        }
        int three_of_a_kind_rank = has_x_of_a_kind(3, deck_);
        if(three_of_a_kind_rank >= 0) {
            special_value_1_ = three_of_a_kind_rank;
            if(set_hand_score("THREE OF A KIND")) return;
            std::cout << "error setting three of a kind" << std::endl;
        }
        std::pair<int, int> two_pair_ranks = has_x_y_of_a_kind(2, 2, deck_);
        if(two_pair_ranks.first >= 0 && two_pair_ranks.second >= 0) {
            special_value_1_ = std::max(two_pair_ranks.first, two_pair_ranks.second);
            special_value_2_ = std::min(two_pair_ranks.first, two_pair_ranks.second);
            if(set_hand_score("TWO PAIR")) return;
            std::cout << "error setting two pair" << std::endl;
        }
        int pair_rank = has_x_of_a_kind(2, deck_);
        if(pair_rank >= 0) {
            special_value_1_ = pair_rank;
            if(set_hand_score("PAIR")) return;
            std::cout << "error setting pair" << std::endl;
        }
        if(set_hand_score("HIGH")) return;
        std::cout << "no hands set.  pretty fucked up." << std::endl;
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

bool PokerHand::has_royal_flush(const std::vector<Card>& cards) {
    return has_straight_flush(cards) && cards[0].get_rank() > 0 && has_ace(cards);
}

bool PokerHand::has_straight_flush(const std::vector<Card>& cards) {
    return has_straight(cards) && has_flush(cards);
}

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

/*
 * this is pretty lazy...
 */
bool PokerHand::operator==(const PokerHand &other) {
    return !(*this > other) && !(*this < other);
}

bool PokerHand::operator<=(const PokerHand &other) {
    return *this < other || *this == other;
}

bool PokerHand::operator>=(const PokerHand &other) {
    return *this > other || *this == other;
}

void PokerHand::enumerate() {
    std::vector<int> ranks (RANKS_.size(), 0);
    std::vector<int> suits (SUITS_.size(), 0);
    for(const Card& c: deck_) {
        ranks[c.get_rank()] += 1;
        suits[c.get_suit()] += 1;
    }
}


