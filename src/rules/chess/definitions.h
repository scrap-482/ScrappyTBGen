#ifndef STANDARD_CHESS_DEFINITIONS_H_
#define STANDARD_CHESS_DEFINITIONS_H_

#include "../../retrograde_analysis/state.hpp"
// #include "../../retrograde_analysis/state_transition.hpp"
#include "../../core/piece_type.h"
#include "../../retrograde_analysis/state_transition.hpp"
#include <cctype> // For tolower() and toupper()

// Non-placement data
struct ChessNPD {
    int enpassantRights;
    // We do not consider castling rights because it has negligable effect on end game.
};

// Board State for Standard Chess
using ChessBoardState = BoardState<64, ChessNPD>;

template<::std::size_t FlattenedSz, typename NonPlacementDataType>
struct cm_function : public CheckmateEvaluator<FlattenedSz, NonPlacementDataType>
{
  bool operator()(const BoardState<FlattenedSz, NonPlacementDataType>& b)
  { return true; }
};

enum PIECE_TYPE_ENUM {PAWN=0, ROOK, KNIGHT, BISHOP, QUEEN, KING};
const int NUM_PIECE_TYPES = KING + 1;
const int NUM_FLIPPABLE_PIECES = 0; // for shogi-like variants
// For shogi-like games, non-flippable pieces should be at end of PIECE_TYPE_ENUM enum so we can use < to check if it can be 
// flipped, then just add NUM_FLIPPABLE_PIECES to get new PIECE_TYPE_ENUM enum. Also note that this does not exclude chess-
// like promotion.

// Note: this has to be parallel to PIECE_TYPE_ENUM
const PieceType PIECE_TYPE_DATA[] = {
    {'p', std::vector<PMO>(), false}, 
    {'r', std::vector<PMO>(), false}, 
    {'n', std::vector<PMO>(), false}, 
    {'b', std::vector<PMO>(), false}, 
    {'q', std::vector<PMO>(), false}, 
    {'k', std::vector<PMO>(), true}
};

// Define a namespace so that this doesn't get accidentally used. Probably not the best practice.
namespace encapsulate_this_lol {
  // TODO: check if vertical/horizontal direction of this array is correct
  const ::std::array<piece_label_t, 64> CHESS_ARRAY = {
    'r', 'n', 'b', 'q', 'k', 'b', 'n', 'r',
    'p', 'p', 'p', 'p', 'p', 'p', 'p', 'p', 
    '\0','\0','\0','\0','\0','\0','\0','\0',
    '\0','\0','\0','\0','\0','\0','\0','\0',
    '\0','\0','\0','\0','\0','\0','\0','\0',
    '\0','\0','\0','\0','\0','\0','\0','\0',
    'p', 'p', 'p', 'p', 'p', 'p', 'p', 'p', 
    'R', 'N', 'B', 'Q', 'K', 'B', 'N', 'R'
  };
};
const ChessBoardState INIT_BOARD_STATE = {::std::bitset<1>(), encapsulate_this_lol::CHESS_ARRAY, ChessNPD()};

#endif
