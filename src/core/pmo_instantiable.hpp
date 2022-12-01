#ifndef PMO_INSTANTIABLE_H
#define PMO_INSTANTIABLE_H

#include "pmo_moddable.hpp"
#include "piece_count_utils.hpp"
#include "rectangular_board.hpp"

// This file contains leaves in the PMO heirarchy that your variant can employ.
// You can use these, or define your own leaves off of the abstract types like these do.

// Shorthand: 
// FS = FlattenedSize
// NPDT = NonPlacementDataType
// CT = CoordsType
// PTC = "Piece Type Count" AKA NumPieceTypes // TODO: bad naming, refactor?


template<::std::size_t FS, typename NPDT, typename CT, ::std::size_t PTC>
class SlidePMO : public ModdablePMO<FS, NPDT, CT, PTC> {
    // the increment in each direction, stored as flattened coordinates.
    std::vector<CT> moveOffsets;

public:
    virtual ::std::pair<::std::vector<BoardState<FS, NPDT>>, ::std::vector<CT>> 
    getModdableMovesWithDisplacement(const BoardState<FS, NPDT>& b,  CT piecePos) const override {
        std::vector<BoardState<FS, NPDT>> resultStates;
        // parallel to resultStates, describes the displacement of the moving piece's coords
        std::vector<CT> resultDisplacements;

        for (auto moveOffset : moveOffsets) {
            // Keep moving in this direction until we have to stop
            CT pieceEndPos = piecePos;
            for (int displacementMultiplier = 1; true; ++displacementMultiplier) {
                pieceEndPos += moveOffset;

                if (!inBounds(pieceEndPos)) break;

                bool isCapture = false;
                auto movedToContents = b.m_board.at(pieceEndPos.flatten());
                if (!isEmpty(movedToContents)) {
                    // if the player-to-move's color is the same as the piece we are moving to, not allowed.
                    if (isWhite(movedToContents) == b.m_player) break;
                    // Otherwise, this is a capture
                    isCapture = true;
                } 
                BoardState<FS, NPDT> newState(b); // copy state

                newState.m_board.at(pieceEndPos.flatten()) = newState.m_board.at(piecePos.flatten());
                newState.m_board.at(piecePos.flatten()) = '\0';
                // Any time we make a move, invert turn and reset en-passant. //TODO: smells bad doing this here, separate this functionality somehow.
                newState.m_player = !newState.m_player; 
                // TODO: reset en passant info

                resultStates.push_back(newState);
                resultDisplacements.push_back(displacementMultiplier * moveOffset);
                if (isCapture) break;
            }
        }
        return std::make_pair(std::move(resultStates), std::move(resultDisplacements));
    }

    virtual ::std::pair<::std::vector<BoardState<FS, NPDT>>, ::std::vector<CT>> 
    getModdableUnmovesWithDisplacement(const BoardState<FS, NPDT>& b,  CT piecePos) const override {
        std::vector<BoardState<FS, NPDT>> resultStates;
        // parallel to resultStates, describes the displacement of the moving piece's coords
        std::vector<CT> resultDisplacements;

        for (auto moveOffset : moveOffsets) {
            // Keep moving in this direction until we have to stop
            CT pieceEndPos = piecePos;
            for (int displacementMultiplier = 1; true; ++displacementMultiplier) {
                pieceEndPos += moveOffset;

                if (!inBounds(pieceEndPos)) break;

                auto movedToContents = b.m_board.at(pieceEndPos.flatten());
                if (!isEmpty(movedToContents)) break; 
                BoardState<FS, NPDT> newState(b); // copy state

                newState.m_board.at(pieceEndPos.flatten()) = newState.m_board.at(piecePos.flatten());
                newState.m_board.at(piecePos.flatten()) = '\0';
                // Any time we make a move, invert turn
                newState.m_player = !newState.m_player; 

                resultStates.push_back(newState);
                resultDisplacements.push_back(displacementMultiplier * moveOffset);
            }
        }
        auto unmoves = std::make_pair(std::move(resultStates), std::move(resultDisplacements));
        addUncaptures<FS, NPDT, CT, PTC>(b, piecePos, unmoves); //FIXME: hack
        // then just add uncaptures
        return unmoves;
    }

    SlidePMO(::std::vector<CT> _moveOffsets) 
        : moveOffsets(_moveOffsets) { }

};


#endif