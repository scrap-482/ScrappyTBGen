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


#ifndef PMO_H_
#define PMO_H_

#include "../utils/utils.h"
#include "../retrograde_analysis/state.hpp"
#include "coords_grid.hpp"
#include "promotion.h"
#include "ignore_macros.hpp"

// Shorthand: 
// FS = FlattenedSize
// NPDT = NonPlacementDataType
// CT = CoordsType
// PTC = "Piece Type Count" AKA NumPieceTypes // TODO: bad naming, refactor?

// Maximum number of pieces that can be on the board specified by user.
const size_t MAN_LIMIT = N_MAN;

template<::std::size_t FS, typename NPDT, typename CT>
class PMO {
public:
    // takes a board state and the piece's current position and returns a list of new possible board states
    virtual ::std::vector<BoardState<FS, NPDT>> 
    getForwards(const BoardState<FS, NPDT>& b, CT piecePos) const = 0;

    // takes a board state and the piece's current position and returns a list of possible board states that could have resulted in it
    virtual ::std::vector<BoardState<FS, NPDT>> 
    getReverses(const BoardState<FS, NPDT>& b, CT piecePos) const = 0;
};

#endif