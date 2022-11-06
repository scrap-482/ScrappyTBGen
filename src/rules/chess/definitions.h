#ifndef STANDARD_CHESS_DEFINITIONS_H_
#define STANDARD_CHESS_DEFINITIONS_H_

#include "../../retrograde_analysis/state.hpp"
// #include "../../retrograde_analysis/state_transition.hpp"
#include "../../core/coords_grid.hpp"
#include "../../core/piece_type.h"
#include "../../core/pmo.hpp"
#include "../../retrograde_analysis/state_transition.hpp"

#include <cctype> // For tolower() and toupper()
#include <map>

// Non-placement data
struct ChessNPD {
    // Column number that piece just double-jumped in. -1 if none.
    int enpassantRights = -1;
    // We do not consider castling rights because it has negligable effect on end game.
};

// Since this is a rectangular board
using Coords = CoordsGrid<int, int>;
const size_t BOARD_WIDTH = 8;
const size_t BOARD_HEIGHT = 8;

using ChessBoardState = BoardState<64, ChessNPD>;
using ChessPMO = PMO<64, ChessNPD, Coords>;
using ChessDisplacementPMO = DisplacementPMO<64, ChessNPD, Coords>;
using ChessPieceType = PieceType<64, ChessNPD, Coords>;

#define flatten(file, rank) ((file + rank*BOARD_WIDTH))
#define flattenCoords(coords) ((coords.file + coords.rank*BOARD_WIDTH))
#define unflatten(index) (Coords(index % BOARD_WIDTH, index / BOARD_WIDTH))

template<::std::size_t FlattenedSz, typename NonPlacementDataType>
struct cm_function : public CheckmateEvaluator<FlattenedSz, NonPlacementDataType>
{
  bool operator()(const BoardState<FlattenedSz, NonPlacementDataType>& b)
  { return true; }
};

// mnemonic to remember this order: pawn first, king last, order remaining pieces from outside in of starting position.
enum PIECE_TYPE_ENUM {PAWN=0, ROOK, KNIGHT, BISHOP, QUEEN, KING};
const int NUM_PIECE_TYPES = KING + 1;
const int NUM_FLIPPABLE_PIECES = 0; // for shogi-like variants
// For shogi-like games, non-flippable pieces should be at end of PIECE_TYPE_ENUM enum so we can use < to check if it can be 
// flipped, then just add NUM_FLIPPABLE_PIECES to get new PIECE_TYPE_ENUM enum. Also note that this does not exclude chess-
// like promotion.

const std::map<piece_label_t, const PIECE_TYPE_ENUM> LABEL_T_TO_TYPE_ENUM_MAP = {
  {'p', PAWN},
  {'r', ROOK},
  {'n', KNIGHT},
  {'b', BISHOP},
  {'q', QUEEN},
  {'k', KING}
  };
const std::array<piece_label_t, NUM_PIECE_TYPES> TYPE_ENUM_TO_LABEL_T = {'p', 'r', 'n', 'b', 'q', 'k'};

// Define a namespace so that this doesn't get accidentally used. Probably not the best practice.
namespace encapsulate_this_lol {
  const ::std::array<piece_label_t, 64> CHESS_ARRAY = {
    'R', 'N', 'B', 'Q', 'K', 'B', 'N', 'R',
    'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 
    '\0','\0','\0','\0','\0','\0','\0','\0',
    '\0','\0','\0','\0','\0','\0','\0','\0',
    '\0','\0','\0','\0','\0','\0','\0','\0',
    '\0','\0','\0','\0','\0','\0','\0','\0',
    'p', 'p', 'p', 'p', 'p', 'p', 'p', 'p', 
    'r', 'n', 'b', 'q', 'k', 'b', 'n', 'r'
  };
};
const ChessBoardState INIT_BOARD_STATE = {::std::bitset<1>(), encapsulate_this_lol::CHESS_ARRAY, ChessNPD()};

// Which promotions to consider. Some users may only want to consider queen promotions since another is rarely optimal.
const std::vector<PIECE_TYPE_ENUM> ALLOWED_PROMOTIONS = {QUEEN};
// const std::vector<PIECE_TYPE_ENUM> ALLOWED_PROMOTIONS = {ROOK, KNIGHT, BISHOP, QUEEN};

/* -------- Specify max number of pieces for reverse move generation -------- */
// assumes use of toColoredTypeIndex
// TODO: What are good values for these? Obvi there can only be 1 king and 8 pawns, but what about the rest? E.g. how many queens should we limit our search to?
const std::array<const int, 2*NUM_PIECE_TYPES> MAX_PIECES_BY_TYPE = {8, 2, 2, 2, 1, 1, 8, 2, 2, 2, 1, 1};

// inclusive limit of number of total pieces on board, e.g. 5-man tablebase
// TODO: we can probably take this in from the command line.
const size_t MAN_LIMIT = 5;

/* ------------------------- piece_label_t functions ------------------------ */
inline piece_label_t toBlack(piece_label_t letter) {
  return tolower(letter);
}
inline piece_label_t toWhite(piece_label_t letter) {
  return toupper(letter);
}
inline bool isEmpty(piece_label_t letter) {
  return letter == '\0';
}
inline bool isWhite(piece_label_t letter) {
  return isupper(letter);
}
inline PIECE_TYPE_ENUM getTypeEnumFromPieceLabel(piece_label_t letter) {
  return LABEL_T_TO_TYPE_ENUM_MAP.at(toBlack(letter));
}
/* -------------------------------------------------------------------------- */
// The number of pieces if we distguish black and white
const size_t NUM_PIECE_TYPES_COLORED = 2*NUM_PIECE_TYPES;
inline size_t toColoredTypeIndex(bool colorIsWhite, PIECE_TYPE_ENUM uncoloredType) {
  return uncoloredType + (colorIsWhite? 0 : NUM_PIECE_TYPES);
}
inline size_t toColoredTypeIndex(piece_label_t p) {
  return toColoredTypeIndex(isWhite(p), getTypeEnumFromPieceLabel(p));
}

// Note: PIECE_TYPE_DATA[] is in chess_pmo.h because need of PMO definitions
#endif
