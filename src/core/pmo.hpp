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