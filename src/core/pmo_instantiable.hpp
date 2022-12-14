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
// PTC = "Piece Type Count" AKA NumPieceTypes


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
                // Any time we make a move //TODO: smells bad doing this here, separate this functionality somehow.
                newState.m_player = !newState.m_player; 

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
        addUncaptures<FS, NPDT, CT, PTC>(b, piecePos, unmoves);
        // then just add uncaptures
        return unmoves;
    }

    SlidePMO(::std::vector<CT> _moveOffsets) 
        : moveOffsets(_moveOffsets) { }

};

// JumpPMO but moveOffsets are negated for black.
template<::std::size_t FS, typename NPDT, typename CT, ::std::size_t PTC>
class DirectedJumpPMO : public ModdablePMO<FS, NPDT, CT, PTC> {
public:
    // The possible displacements for a white piece, stored as flattened coordinates.
    // ASSUMPTION: can just be negated for black pieces. Sufficient if rectangular board and moves is horizontally symmetric.
    std::vector<CT> moveOffsets;

    // Parallel to moveOffsets, each vector for a moveOffset specifies which tiles must be empty so this piece can 'pass through' it (e.g. Xiangqi horse, double-jumping pawn).
    std::vector<std::vector<CT>> obstructOffsets;

    virtual ::std::pair<::std::vector<BoardState<FS, NPDT>>, ::std::vector<CT>> 
    getModdableMovesWithDisplacement(const BoardState<FS, NPDT>& b,  CT piecePos) const override {
        std::vector<BoardState<FS, NPDT>> resultStates;
        // parallel to resultStates, describes the displacement of the moving piece's coords
        std::vector<CT> resultDisplacements;

        for (size_t offsetIndex = 0; offsetIndex < moveOffsets.size(); ++offsetIndex) {
            auto moveOffset = moveOffsets.at(offsetIndex);
            // this is just the piece we are moving
            auto movedFromContents = b.m_board.at(piecePos.flatten());
            // negate if black
            if (!isWhite(movedFromContents)) moveOffset *= -1;

            CT pieceEndPos = piecePos + moveOffset;
            if (!inBounds(pieceEndPos)) continue;

            bool obstructed = false;
            for (auto obstructOffset : obstructOffsets.at(offsetIndex)) {
                // negate if black
                if (!isWhite(movedFromContents)) obstructOffset *= -1;

                auto obstrContents = b.m_board.at((piecePos + obstructOffset).flatten());
                if (obstrContents != '\0') {
                    obstructed = true;
                    break;
                }
            }
            if (obstructed) continue;

            auto movedToContents = b.m_board.at(pieceEndPos.flatten());

            if (!isEmpty(movedToContents)) {
                // if the player-to-move's color is the same as the piece we are moving to, not allowed.
                if (isWhite(movedToContents) == b.m_player) continue;
                // Otherwise, this is a capture
            }
            BoardState<FS, NPDT> newState(b); // copy state.

            newState.m_board.at(pieceEndPos.flatten()) = newState.m_board.at(piecePos.flatten());
            newState.m_board.at(piecePos.flatten()) = '\0';
            // Any time we make a move, invert turn //TODO: smells bad doing this here, separate this functionality somehow.
            newState.m_player = !newState.m_player; 

            resultStates.push_back(newState);
            resultDisplacements.push_back(moveOffset);
        }
        
        return std::make_pair(std::move(resultStates), std::move(resultDisplacements));
    }

    virtual ::std::pair<::std::vector<BoardState<FS, NPDT>>, ::std::vector<CT>> 
    getModdableUnmovesWithDisplacement(const BoardState<FS, NPDT>& b,  CT piecePos) const override {
        std::vector<BoardState<FS, NPDT>> resultStates;
        // parallel to resultStates, describes the displacement of the moving piece's coords
        std::vector<CT> resultDisplacements;

        for (size_t offsetIndex = 0; offsetIndex < moveOffsets.size(); ++offsetIndex) {
            auto moveOffset = moveOffsets.at(offsetIndex);
            // this is just the piece we are moving
            auto movedFromContents = b.m_board.at(piecePos.flatten());
            // negate since this is UNmove, then negate again if black
            if (isWhite(movedFromContents)) moveOffset *= -1;

            CT pieceEndPos = piecePos + moveOffset;
            if (!inBounds(pieceEndPos)) continue;

            bool obstructed = false;
            for (auto obstructOffset : obstructOffsets.at(offsetIndex)) {
                // negate since this is UNmove, then negate again if black
                if (isWhite(movedFromContents)) obstructOffset *= -1;

                auto obstrContents = b.m_board.at((piecePos + obstructOffset).flatten());
                if (obstrContents != '\0') {
                    obstructed = true;
                    break;
                }
            }
            if (obstructed) continue;

            auto movedToContents = b.m_board.at(pieceEndPos.flatten());
            // ASSUMPTION: moves leave behind empty tile
            if (!isEmpty(movedToContents)) continue; // TODO: consider abstracting fwd and rev move to have less repetition
            BoardState<FS, NPDT> newState(b); // copy state.

            newState.m_board.at(pieceEndPos.flatten()) = newState.m_board.at(piecePos.flatten());
            newState.m_board.at(piecePos.flatten()) = '\0';
            // Any time we make an unmove, invert turn.
            newState.m_player = !newState.m_player; 

            resultStates.push_back(newState);
            resultDisplacements.push_back(moveOffset);
        }
        
        auto unmoves = std::make_pair(std::move(resultStates), std::move(resultDisplacements));
        // non-capture unmoves are same as forward moves, but with opposite person playing
        addUncaptures<FS, NPDT, CT, PTC>(b, piecePos, unmoves);
        // then just add uncaptures
        return unmoves;
    }

    DirectedJumpPMO(::std::vector<CT> _moveOffsets) 
    : moveOffsets(_moveOffsets), obstructOffsets(_moveOffsets.size(), std::vector<CT>()) { }

    DirectedJumpPMO(::std::vector<CT> _moveOffsets, ::std::vector<std::vector<CT>> _obstructOffsets) 
    : moveOffsets(_moveOffsets), obstructOffsets(_obstructOffsets) { }

    using PMOPreModList_ = ::std::vector<const PMOPreMod<FS, NPDT, CT>*>;
    using PMOPostModList_ = ::std::vector<const PMOPostMod<FS, NPDT, CT>*>;

    DirectedJumpPMO(::std::vector<CT> _moveOffsets
        , PMOPreModList_ preFwdMods, PMOPostModList_ postFwdMods, PMOPreModList_ preBwdMods, PMOPostModList_ postBwdMods) 
    : moveOffsets(_moveOffsets), obstructOffsets(_moveOffsets.size(), std::vector<CT>())
        , ModdablePMO<FS, NPDT, CT, PTC>(preFwdMods, postFwdMods, preBwdMods, postBwdMods) { }

    DirectedJumpPMO(::std::vector<CT> _moveOffsets
        , PMOPreModList_ preFwdMods, PMOPostModList_ postFwdMods, PMOPreModList_ preBwdMods, PMOPostModList_ postBwdMods
        , PMOPreModList_ preUnpromotionMods, PMOPostModList_ postUnpromotionMods) 
    : moveOffsets(_moveOffsets), obstructOffsets(_moveOffsets.size(), std::vector<CT>())
        , ModdablePMO<FS, NPDT, CT, PTC>(preFwdMods, postFwdMods, preBwdMods, postBwdMods, preUnpromotionMods, postUnpromotionMods) { }

    DirectedJumpPMO(::std::vector<CT> _moveOffsets, ::std::vector<std::vector<CT>> _obstructOffsets
        , PMOPreModList_ preFwdMods, PMOPostModList_ postFwdMods, PMOPreModList_ preBwdMods, PMOPostModList_ postBwdMods) 
    : moveOffsets(_moveOffsets), obstructOffsets(_obstructOffsets)
        , ModdablePMO<FS, NPDT, CT, PTC>(preFwdMods, postFwdMods, preBwdMods, postBwdMods) { }

    DirectedJumpPMO(::std::vector<CT> _moveOffsets, ::std::vector<std::vector<CT>> _obstructOffsets
        , PMOPreModList_ preFwdMods, PMOPostModList_ postFwdMods, PMOPreModList_ preBwdMods, PMOPostModList_ postBwdMods
        , PMOPreModList_ preUnpromotionMods, PMOPostModList_ postUnpromotionMods)
    : moveOffsets(_moveOffsets), obstructOffsets(_obstructOffsets)
        , ModdablePMO<FS, NPDT, CT, PTC>(preFwdMods, postFwdMods, preBwdMods, postBwdMods, preUnpromotionMods, postUnpromotionMods) { }

};
#endif