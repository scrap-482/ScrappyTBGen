#ifndef PIECE_TYPE_H
#define PIECE_TYPE_H

#include "pmo.hpp"
#include "piece_enum.hpp"
#include "../utils/utils.h"
#include "../retrograde_analysis/state.hpp"

// An instance of this corresponds to a type of piece, e.g. there is a single Piece instance for pawns.
template<::std::size_t FlattenedSz, typename NonPlacementDataType, typename Coords>
struct PieceType {

    piece_label_t letter;

    // the array of PMOs
    const PMO<FlattenedSz, NonPlacementDataType, Coords>* const* pmoList;
    // length of pmoList
    size_t pmoListSize;

    bool royalty;
    // note: in Chu Shogi, Drunk Elephant promotes to Prince (which is royal)
    // But since these are given different PieceTypes (E vs E+), the royalty is
    // only assigned to the promoted piece's PieceType.

};

// Note: this has to be parallel to PIECE_TYPE_ENUM
template<::std::size_t FlattenedSz, typename NonPlacementDataType, typename Coords>
const PieceType<FlattenedSz, NonPlacementDataType, Coords>&
getPieceTypeData(piece_type_enum_t pieceEnum);

#endif