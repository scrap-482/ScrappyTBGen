#ifndef GAME_H
#define GAME_H

#include "constants.hpp"
#include "board.h"
#include "ruleset.h"

#include <cstdint>
#include <stack>
#include <vector>
#include <string>

class Game {
    public:
        Board* m_board = nullptr;
        // list of moves applied to starting SFEN.
        std::stack<std::unique_ptr<Move>> m_moveHistory; 

        // Number of full moves (increments after black plays).
        // WARNING: starts at 1.
        int m_moveCount = 1;

        // 50 move rule counter. Resets at zero anytime a pawn moves or a 
        // capture happens, stalemate happens if this reaches 50.
        short m_50Count = 0; 
        // whose turn it is
        PieceColor m_turn = WHITE;
        // What rules we are playing with
        Ruleset m_rules;

        // TODO: probably should be able to take in PGNs too.
        Game(std::string _sfen, std::string _rulesFile="rules/default.rules"); // TODO: weird hardcode
        // FIXME: test if rules file is actually being read in.
        void reset(const std::string _sfen);

        // Applies the move, and stores the unique_ptr in its history stack
        bool applyMove(std::unique_ptr<Move> _move);

        // Undoes the move, then pops and returns the unique_ptr from its history stack.
        // unique_ptr is to nullptr if it fails.
        std::unique_ptr<Move> undoMove();

        uint64_t getHash() const {
            //TODO: implement
            return 0u;
        };

        std::string toSfen() const;

        std::string print();
        std::string printMoveHistory(size_t _numTurns);
};

#endif