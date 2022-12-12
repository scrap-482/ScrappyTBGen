/*
* Copyright 2022 SCRAP
*
* This file is part of Scrappy Tablebase Generator.
* 
* Scrappy Tablebase Generator is free software: you can redistribute it and/or modify it under the terms 
* of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, 
* or (at your option) any later version.
* 
* Scrappy Tablebase Generator is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of 
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along with Scrappy Tablebase Generator. If not, see <https://www.gnu.org/licenses/>.
*/


#ifndef STANDARD_CHESS_DEFINITIONS_H_
#define STANDARD_CHESS_DEFINITIONS_H_

#include "../../retrograde_analysis/state.hpp"
#include "../../retrograde_analysis/state_transition.hpp"
#include "../../core/rectangular_board.hpp"
#include "../../core/piece_enum.hpp"
#include "../../core/piece_type.hpp"
#include "../../core/piece_count_utils.hpp"
#include "../../core/pmo_instantiable.hpp"

#include <map>

// Non-placement data
struct ChessNPD {
    // Column number that piece just double-jumped in. -1 if none.
    int enpassantRights = -1;
    // We do not consider castling rights because it has negligable effect on end game.
};
template<typename NonPlacementDataType>
std::string NPDToString(const NonPlacementDataType& npd) {
    std::string ret = "En passant rights: ";
    ret += (npd.enpassantRights == -1)? "no" : ::std::to_string(npd.enpassantRights);
    ret += "\n";
    return ret;
}

/* ---------------------- Specify (uncolored) piece enum ---------------------- */

// mnemonic to remember this order: pawn first, king last, order remaining pieces from outside in of starting position.
enum PIECE_TYPE_ENUM : piece_label_t {PAWN=0, ROOK, KNIGHT, BISHOP, QUEEN, KING, VACANT=(piece_label_t) -1};

const int NUM_PIECE_TYPES = KING+1;
// const int NUM_FLIPPABLE_PIECES = 0; // for shogi-like variants // TODO: remove if unused
// For shogi-like games, non-flippable pieces should be at end of PIECE_TYPE_ENUM enum so we can use < to check if it can be 
// flipped, then just add NUM_FLIPPABLE_PIECES to get new PIECE_TYPE_ENUM enum. Also note that this does not exclude chess-
// like promotion.

/* ------------------ Specify colored piece indexing scheme ----------------- */

// The number of pieces if we distguish black and white
const size_t NUM_PIECE_TYPES_COLORED = 2*NUM_PIECE_TYPES;

inline size_t toColoredTypeIndex(bool colorIsWhite, PIECE_TYPE_ENUM uncoloredType) {
  return uncoloredType + (colorIsWhite? 0 : NUM_PIECE_TYPES);
}
inline size_t toColoredTypeIndex(piece_label_t p) {
  return toColoredTypeIndex(isWhite(p), getTypeEnumFromPieceLabel(p));
}

/* ------------- Specify label_t to PIECE_TYPE_ENUM conversions ------------- */

const std::map<piece_label_t, const PIECE_TYPE_ENUM> LABEL_T_TO_TYPE_ENUM_MAP = {
  {'p', PAWN},
  {'r', ROOK},
  {'n', KNIGHT},
  {'b', BISHOP},
  {'q', QUEEN},
  {'k', KING},
  {'\0', VACANT}
  };
const std::array<piece_label_t, NUM_PIECE_TYPES> TYPE_ENUM_TO_LABEL_T = {'p', 'r', 'n', 'b', 'q', 'k'};

inline PIECE_TYPE_ENUM getTypeEnumFromPieceLabel(piece_label_t letter) {
  return LABEL_T_TO_TYPE_ENUM_MAP.at(toBlack(letter));
}
inline piece_label_t getPieceLabelFromTypeEnum(PIECE_TYPE_ENUM type) {
  return TYPE_ENUM_TO_LABEL_T[type];
}

/* -------- Specify max number of pieces for reverse move generation -------- */

// assumes use of toColoredTypeIndex
// TODO: What are good values for these? Obvi there can only be 1 king and 8 pawns, but what about the rest? E.g. how many queens should we limit our search to?
const std::array<const int, NUM_PIECE_TYPES_COLORED> MAX_PIECES_BY_TYPE = {8, 2, 2, 2, 1, 1, 8, 2, 2, 2, 1, 1};
inline int maxPiecesByColoredType(size_t coloredType) {
  return MAX_PIECES_BY_TYPE.at(coloredType);
}

/* -------------------------------------------------------------------------- */
using ChessBoardState = BoardState<64, ChessNPD>;

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
const ChessBoardState INIT_BOARD_STATE = {true, encapsulate_this_lol::CHESS_ARRAY, ChessNPD()};

/* -------------------------------------------------------------------------- */

// Implement this function, which is required for allowedUncaptures defined in core.
template<::std::size_t FS, typename NPDT, typename CT, size_t NumPieceTypes>
std::bitset<NumPieceTypes> allowedUncapturesByPosAndCount(const BoardState<FS, NPDT>& b,  CT piecePos) {
    std::bitset<NumPieceTypes> allowed = allowedUncapturesByCount<FS, NPDT, CT, NumPieceTypes>(b);
    // Check that pawns cannot be in first or last rank
    if (piecePos.rank == 0 || piecePos.rank == BOARD_HEIGHT-1) {
        allowed[PAWN] = 0;
    }
    // TODO: which file are pawns allowed on? Is there a limit to how many pawns can stack into a file I should care about?
    // TODO: bishops only allowed on opposite colors, unless we consider pawns which can promote to bishops.
    // TODO: This feels like it overlaps with ValidityChecker somehow

    return allowed;
}

#endif
