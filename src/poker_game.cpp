//
//  poker_game.cpp
//  poker_calculator
//
//  Created by Ben Ellis on 7/19/16.
//  Copyright © 2016 Ben Ellis. All rights reserved.
//

#include <iostream>
#include <vector>
#include <iomanip>
#include <ctime>
#include <chrono>
#include <thread>
#include <future>

#include "poker_game.hpp"
#include "misc.hpp"
#include "poker_hand.hpp"
#include "deck.hpp"

PokerGame::PokerGame() {
    deck_ = Deck();
    int num_players;
    do {
        std::cout << "Enter number of players " << std::endl;
        std::cin >> num_players;
        // num_players = 2;
        if(num_players < 2 || num_players > 10) {
            std::cout << "must have at between 2 and 7 players... " << std::endl;
        }
    } while(num_players < 2 || num_players > 10);
    for(int i = 0; i < num_players; ++i) {
        players_.push_back(PokerHand());
    }
}

PokerGame::~PokerGame() { };

void PokerGame::init_hand() {
    std::cout << "cards in deck: " << std::endl;
    std::cout << deck_.str() << std::endl;
    std::cout << "first card in hand: " << std::endl;
    Card c = get_card_from_user();
    //Card c = Card(0, 12);
    players_[0].add_back(c);
    deck_.delete_card(c);
    std::cout << "second card in hand: " << std::endl;
    c = get_card_from_user();
    //c = Card(1, 12);
    players_[0].add_back(c);
    deck_.delete_card(c);
    std::cout << std::endl;
}

void PokerGame::init_community() {
    int ncards;
    do {
        std::cout << "enter number of community cards: ";
        std::cin >> ncards;
        //ncards = 0;
        if(ncards > 5) std::cout << "cannot have more than 5 community cards.  try again." << std::endl;
        if(ncards < 0) ncards = 0;
    } while (ncards > 5);
    if(ncards > 0) {
        std::cout << "cards in deck: " << std::endl;
        std::cout << deck_.str() << std::endl;
        for (int i = 0; i < ncards; ++i) {
            std::cout << "community card " << i + 1 << ":" << std::endl;
            Card c = get_card_from_user();
            community_cards_.push_back(c);
        }
    }
    std::cout << std::endl;
}

void PokerGame::monte_carlo_loop(const int& ntrials) {
    std::cout << "Evaluating win probability using Monte Carlo." << std::endl;
    long community_cards_left = 5 - community_cards_.size();
    int nwin = 0;
    auto t0 = std::chrono::high_resolution_clock::now();
    for(int i = 0; i < ntrials; ++i) {
        double pct_done = double(i + 1) / double(ntrials) * 100.0e0;
        std::cout << "  " << i + 1 << " out of " << ntrials
                  << " trials (" << std::fixed << std::setprecision(2) << pct_done << "%)" << "\r";
        nwin += monte_carlo_trial(community_cards_left);
    }
    std::cout << std::endl;
    double pct = double(nwin) / double(ntrials)  * 100.e0;
    auto tf = std::chrono::high_resolution_clock::now();
    auto duration = (double)std::chrono::duration_cast<std::chrono::milliseconds>(tf -t0).count();
    std::cout << std::endl;
    std::cout << players_[0].str() << "wins approximately " << pct << "% of hands. "
              << "Calculation took " << duration / 1000 << " seconds. " << std::endl;
    std::cout << std::endl;
}

int PokerGame::monte_carlo_omp(Deck deck, std::vector<PokerHand> players, std::vector<Card> community_cards,
                                         int community_cards_left, int ntrials) {
    int nwin = 0;
    //auto t0 = omp_get_wtime();
#pragma omp parallel for schedule(static) firstprivate(deck, players, community_cards)
    for(int i = 0; i < ntrials; ++i) {
        //deck.repopulate();
        //for (const Card &c: players[0].get_deck()) {
        //    deck.delete_card(c);
        //}
        //for (const Card &c: community_cards) {
        //    deck.delete_card(c);
        //}
        for (int i_player = 1; i_player < players.size(); i_player++) {
            for (int i_card = 0; i_card < 2; ++i_card) {
                players[i_player].add_back(deck.draw_delete_rand_card());
            }
        }
        for (int i_card = 0; i_card < community_cards_left; ++i_card)
            community_cards.push_back(deck.draw_delete_rand_card());
        std::vector<PokerHand> best_hands;
        for (PokerHand &player: players) {
            std::vector<Card> tmp;
            for (const Card &c: player.get_deck()) {
                tmp.push_back(c);
            }
            for (const Card &c: community_cards) {
                tmp.push_back(c);
            }
            std::vector<std::vector<Card>> five_card_hands = combinations(tmp, 5);
            PokerHand best_hand = find_best_hand(five_card_hands);
            best_hands.push_back(best_hand);
            five_card_hands.clear();
        }
        bool user_win = true;
        for (int i = 1; i < players.size(); ++i) {
            if (best_hands[0] < best_hands[i]) {
                user_win = false;
            }
        }
        if(user_win) {
#pragma omp atomic
            ++nwin;
        }
        //for (int i = 1; i < players.size(); ++i) players[i].clear();
        for (int i = 1; i < players.size(); ++i) {
            for(int j = 0; j < 2; ++j) {
                deck.add_back(players[i].draw_delete_back());
            }
        }
        best_hands.clear();
        for (int i_card = 0; i_card < community_cards_left; ++i_card) {
            deck.add_back(community_cards.back());
            community_cards.pop_back();
        }
    }
    //auto time = omp_get_wtime() - t0;
    //return std::pair<int, double> (nwin, 0.0e0);
    return nwin;
}
/*
void PokerGame::monte_carlo_omp_wrap(const int ntrials) {
    int community_cards_left = 5 - community_cards_.size();
    std::pair<int, double> stats = monte_carlo_omp(deck_, players_, community_cards_, community_cards_left, ntrials);
    std::cout << std::endl;
    double pct = double(stats.first) / double(ntrials)  * 100.e0;
    std::cout << std::endl;
    std::cout << players_[0].str() << "wins approximately " << pct << "% of hands. "
              << "Calculation took " << stats.second << " seconds. " << std::endl;
    std::cout << std::endl;
}
*/

int PokerGame::monte_carlo_trial(const int& community_cards_left) {
    deck_.repopulate();
    for(const Card& c: players_[0].get_deck()) {
        deck_.delete_card(c);
    }
    for(const Card& c: community_cards_) {
        deck_.delete_card(c);
    }
    for(int i_player = 1; i_player < players_.size(); i_player++) {
        for(int i_card = 0; i_card < 2; ++i_card) {
            players_[i_player].add_back(deck_.draw_delete_rand_card());
        }
    }
    for(int i_card = 0; i_card < community_cards_left; ++i_card) community_cards_.push_back(deck_.draw_delete_rand_card());
    std::vector<PokerHand> best_hands;
    for(PokerHand& player: players_) {
        std::vector<Card> tmp;
        for(const Card& c: player.get_deck()) {
            tmp.push_back(c);
        }
        for(const Card& c: community_cards_) {
            tmp.push_back(c);
        }
        std::vector<std::vector<Card>> five_card_hands = combinations(tmp, 5);
        PokerHand best_hand = find_best_hand(five_card_hands);
        best_hands.push_back(best_hand);
        //five_card_hands.clear();
    }
    bool user_win = true;
    //std::cout << "[0] " << best_hands[0].str() << " " << best_hands[0].show_score() << std::endl;
    for(int i = 1; i < players_.size(); ++i) {
        //std::cout << "[1] " << best_hands[i].str() << " " << best_hands[i].show_score() << std::endl;
        if(best_hands[0] < best_hands[i]) {
            user_win = false;
        }
    }
    //std::cout << user_win << std::endl;
    for(int i = 1; i < players_.size(); ++i) players_[i].clear();
    best_hands.clear();
    for(int i_card = 0; i_card < community_cards_left; ++i_card) community_cards_.pop_back();
    if(user_win) return 1;
    return 0;
}

PokerHand PokerGame::find_best_hand(const std::vector<std::vector<Card>>& hands_of_5) {
    PokerHand best_hand = PokerHand(std::move(hands_of_5[0]));
    best_hand.score_hand();
    //for(const std::vector<Card>& this_hand: hands_of_5) {
    for(int ihand = 1; ihand < hands_of_5.size(); ++ihand) {
        //PokerHand tmp(hands_of_5[ihand]);
        PokerHand tmp(std::move(hands_of_5[ihand]));
        //PokerHand tmp(this_hand);
        tmp.score_hand();
        if(tmp > best_hand) best_hand = tmp;
    }
    return best_hand;
}

Card PokerGame::get_card_from_user() {
    std::string rank, suit;
    bool in_deck;
    do {
        std::cout << "  rank: ";
        std::cin >> rank;
        std::cout << "  suit: ";
        std::cin >> suit;
        in_deck = deck_.find(suit, rank);
        if(!in_deck) {
            std::cout << "  --> that card isn't in the deck.  try again." << std::endl;
        }
    } while(!in_deck);
    return deck_.generate_card(suit, rank);
}

void PokerGame::monte_carlo_loop_thread(const int& ntrials) {
    std::cout << "Evaluating win probability using Monte Carlo." << std::endl;
    long community_cards_left = 5 - community_cards_.size();
    int nwin = 0;
    int nthreads = std::thread::hardware_concurrency();
    std::cout << "Number of threads: " << nthreads << std::endl;
    std::cout << "Total number of trials: " << ntrials << std::endl;
    std::vector<std::future<int>> wins;
    auto t0 = std::chrono::high_resolution_clock::now();
    for(int i = 0; i < nthreads; ++i) {
        wins.push_back(std::async(std::launch::async, monte_carlo_omp, deck_, players_, community_cards_, community_cards_left, ntrials / nthreads));
    }
    for(int i = 0; i < nthreads; ++i) {
        nwin += wins[i].get();
    }
    std::cout << std::endl;
    //double pct = double(nwin) / double(ntrials * nthreads)  * 100.e0;
    double pct = double(nwin) / double(ntrials)  * 100.e0;
    auto tf = std::chrono::high_resolution_clock::now();
    auto duration = (double)std::chrono::duration_cast<std::chrono::milliseconds>(tf -t0).count() / 1000.0e0;
    std::cout << std::endl;
    std::cout << players_[0].str() << "wins approximately " << pct << "% of hands. "
    << "Calculation took " << duration << " seconds. " << std::endl;
    std::cout << std::endl;
}

int PokerGame::monte_carlo_loop2(const int& ntrials) {
    std::cout << "Evaluating win probability using Monte Carlo." << std::endl;
    long community_cards_left = 5 - community_cards_.size();
    int nwin = 0;
    //auto t0 = std::chrono::high_resolution_clock::now();
    for(int i = 0; i < ntrials; ++i) {
        double pct_done = double(i + 1) / double(ntrials) * 100.0e0;
        std::cout << "  " << i + 1 << " out of " << ntrials
        << " trials (" << std::fixed << std::setprecision(2) << pct_done << "%)" << "\r";
        nwin += monte_carlo_trial(community_cards_left);
    }
    std::cout << std::endl;
    //double pct = double(nwin) / double(ntrials)  * 100.e0;
    //auto tf = std::chrono::high_resolution_clock::now();
    //auto duration = (double)std::chrono::duration_cast<std::chrono::milliseconds>(tf -t0).count();
    //std::cout << std::endl;
    //std::cout << players_[0].str() << "wins approximately " << pct << "% of hands. "
    //<< "Calculation took " << duration / 1000 << " seconds. " << std::endl;
    //std::cout << std::endl;
    return nwin;
}
