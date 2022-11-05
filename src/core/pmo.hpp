#ifndef PMO_H_
#define PMO_H_
// TODO: rename this to .hpp
#include "region.h"
// #include "piece_type.h" // caution! circularity approaching

#include "../utils/utils.h"
#include "../core/coords_grid.hpp"
#include "../retrograde_analysis/state.hpp"

template<::std::size_t FlattenedSz, typename NonPlacementDataType, typename CoordsType>
class PMO {
    // takes a board state and the piece's current position and returns a list of new possible board states
    virtual ::std::vector<BoardState<FlattenedSz, NonPlacementDataType>> 
    getForwards(const BoardState<FlattenedSz, NonPlacementDataType>& b, CoordsType piecePos) = 0;

    // takes a board state and the piece's current position and returns a list of possible board states that could have resulted in it
    virtual ::std::vector<BoardState<FlattenedSz, NonPlacementDataType>> 
    getReverses(const BoardState<FlattenedSz, NonPlacementDataType>& b, CoordsType piecePos) = 0;
};

/* -------- Below are some PMO templates that may be useful extending ------- */

// Any move that takes a piece and moves it somewhere else. Exposes displacement of moves to make complex move implementation easier.
template<::std::size_t FlattenedSz, typename NonPlacementDataType, typename CoordsType>
class DisplacementPMO : public PMO<FlattenedSz, NonPlacementDataType, CoordsType> {

    // takes a board state and the piece's current position and returns a list of new possible board states 
    // AND a parallel vector of the displacements of the moving piece 
    virtual ::std::pair<::std::vector<BoardState<FlattenedSz, NonPlacementDataType>>, ::std::vector<CoordsType>>
    getForwardsWithDisplacement(const BoardState<FlattenedSz, NonPlacementDataType>& b, CoordsType piecePos) = 0;

    // takes a board state and the piece's current position and returns a list of possible board states that could have resulted in it
    // AND a parallel vector of the displacements of the moving piece 
    virtual ::std::pair<::std::vector<BoardState<FlattenedSz, NonPlacementDataType>>, ::std::vector<CoordsType>>
    getReversesWithDisplacement(const BoardState<FlattenedSz, NonPlacementDataType>& b, CoordsType piecePos) = 0;

    ::std::vector<BoardState<FlattenedSz, NonPlacementDataType>> 
    getForwards(const BoardState<FlattenedSz, NonPlacementDataType>& b, CoordsType piecePos) {
        return getForwardsWithDisplacement(b, piecePos).first;
    };

    ::std::vector<BoardState<FlattenedSz, NonPlacementDataType>> 
    getReverses(const BoardState<FlattenedSz, NonPlacementDataType>& b, CoordsType piecePos) {
        return getReversesWithDisplacement(b, piecePos).first;
    };
};
// TODO: generalize chess classes to interfaces that can be put here

#endif