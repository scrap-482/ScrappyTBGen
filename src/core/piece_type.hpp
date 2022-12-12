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