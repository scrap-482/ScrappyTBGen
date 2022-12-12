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


#ifndef INTERFACE_LOGIC_H_
#define INTERFACE_LOGIC_H_

#include "pmo_moddable.hpp"
#include "../retrograde_analysis/state.hpp"

// Shorthand: 
// FS = FlattenedSize
// NPDT = NonPlacementDataType
// CT = CoordsType
// PTC = "Piece Type Count" AKA NumPieceTypes // TODO: bad naming, refactor?

// Looping over all PMOs is common functionality, but what to do with them varies; as such, this is implemented as a 
// template function.
// actOnPMO will act on every move (except unpromotions), while actOnUnpromotionPMO acts only on unpromotions.
// Set reverse=true to flip the color of who we are generating moves for (which is needed for all backwards moves).
// ForEachPMOFunc is assumed to have the type:
//     (std::vector<BoardState<FS, NPDT>> newMoves, const BoardState<FS, NPDT>& b) -> bool
// ForEachPMOUnpromotionFunc is assumed to have the type:
//     (std::vector<BoardState<FS, NPDT>> newMoves, const BoardState<FS, NPDT>& b, piece_label_t unpromotedPlt, piece_label_t thisPiece) -> bool
// Can generate all PMOs for the player whose turn it is (forPlayerToMove=true) or for the other.
template <::std::size_t FS, typename NPDT, typename CT, ::std::size_t  PTC, typename ForEachPMOFunc>
void loopAllPMOs(const BoardState<FS, NPDT>& b, ForEachPMOFunc actOnPMO, bool reverse=false) {
    const auto noopLambda = [&](const BoardState<FS, NPDT>& b, const PMO<FS, NPDT, CT>* pmo, size_t flatStartPos, piece_label_t unpromoted, piece_label_t promoted) { return false; };
    loopAllPMOs<FS, NPDT, CT, PTC, ForEachPMOFunc, decltype(noopLambda) >(b, actOnPMO, reverse, noopLambda);
}
template <::std::size_t FS, typename NPDT, typename CT, ::std::size_t PTC, typename ForEachPMOFunc, typename ForEachPMOUnpromotionFunc>
void loopAllPMOs(const BoardState<FS, NPDT>& b, ForEachPMOFunc actOnPMO, bool reverse, ForEachPMOUnpromotionFunc actOnUnpromotionPMO) {
    // TODO: consider using a list of the positions of every type of piece, rather than brute-force checking all tiles
    for (size_t flatStartPos = 0; flatStartPos < FS; ++flatStartPos) {
        piece_label_t thisPiece = b.m_board.at(flatStartPos);
        if (isEmpty(thisPiece)) continue;
        // Ignore this piece if it is not for the player-to-move. Invert this result if forPlayerToMove=false.
        if (isWhite(thisPiece) ^ b.m_player ^ reverse) continue;

        PIECE_TYPE_ENUM type = getTypeEnumFromPieceLabel(thisPiece);

        for (size_t i = 0; i < getPieceTypeData<FS, NPDT, CT>(type).pmoListSize; ++i) {
            auto pmo = getPieceTypeData<FS, NPDT, CT>(type).pmoList[i];
            // Break if function returns false
            if (!actOnPMO(b, pmo, flatStartPos)) return;
        }

        // Now check for unpromotion PMOs
        if (reverse) {
            for (auto unpromotedPlt : promotionScheme.getUnpromotions(thisPiece)) {
                PIECE_TYPE_ENUM unpromotedType = getTypeEnumFromPieceLabel(unpromotedPlt);

                for (size_t i = 0; i < getPieceTypeData<FS, NPDT, CT>(unpromotedType).pmoListSize; ++i) {
                    // ASSUMPTION: every PMO held by a promotable piece is a PromotablePMO.
                    auto pmo = (PromotablePMO<FS, NPDT, CT, PTC>*) getPieceTypeData<FS, NPDT, CT>(unpromotedType).pmoList[i];
                    // Break if function returns false
                    if (!actOnUnpromotionPMO(b, pmo, flatStartPos, unpromotedPlt, thisPiece)) return;
                }
            }
        }
    }
    return;
}

/* -------------------------------------------------------------------------- */

// Determine if player isWhiteAttacking is attacking opponent's (last) royal
// ASSUMPTION: only 1 Royal piece per color for now
template<::std::size_t FS, typename NPDT, typename CT, ::std::size_t PTC>
bool inCheck(const BoardState<FS, NPDT>& b, bool isWhiteAttacking) {
    // std::cout << "in inCheck start, isWhiteAttacking: " << isWhiteAttacking << std::endl;

    // TODO: check if multiple royal pieces. If like Chu Shogi all royal need to be captured, then return false if num royal > 0

    // TODO: seems kinda silly we need to copy the whole board just to flip player-to-move...
    // copy state so it is not const
    BoardState<FS, NPDT> bRev(b);
    // we will generate moves for side attacking
    bRev.m_player = isWhiteAttacking;

    //std::cout << "in inCheck start, after copy bRev.m_player: " << bRev.m_player << std::endl;

    // Check is just saying that if the player were to move again, they could capture opponent royal.
    // So check for royal captures.
    bool isCheck = false;
    auto actOnPMO = [&](const BoardState<FS, NPDT>& bRev, const PMO<FS, NPDT, CT>* pmo, size_t flatStartPos) {
        auto startPos = CT(flatStartPos);
        auto newMovesWithDisplacement = ((ModdablePMO<FS, NPDT, CT, PTC>*) pmo)->getForwardsWithDisplacement(bRev, startPos); // Warning: cast assumes all moves are displacements.
        
        for (size_t i = 0; i < newMovesWithDisplacement.first.size(); ++i) {
            // auto move = newMovesWithDisplacement.first.at(i);
            auto displacement = newMovesWithDisplacement.second.at(i);

            // Check if this move is the capture of a royal
            // ASSUMPTION: a piece can only capture a royal by ending its turn on royal's position
            auto endPos = startPos + displacement;

            if (isRoyal(bRev.m_board.at(endPos.flatten()))) {
            // TODO: if counting was fast, we could just use that on royal pieces
                isCheck = true;
                // stop search
                return false;
            } 
        }
        return true;
    };
    loopAllPMOs<FS, NPDT, CT, PTC, decltype(actOnPMO)>(bRev, actOnPMO);
    return isCheck;
}

template<::std::size_t FS, typename NPDT, typename CT, ::std::size_t PTC>
bool inMate(const BoardState<FS, NPDT>& b) {
    bool isMate = true;
    auto actOnPMO = [&](const BoardState<FS, NPDT>& b, const PMO<FS, NPDT, CT>* pmo, size_t flatStartPos) {
        auto newMoves = pmo->getForwards(b, CT(flatStartPos));
        // Save all moves that do not move self into check
        for (auto newMove : newMoves) {
            // prune out moves where player-to-move checks themselves
            if (inCheck<FS, NPDT, CT, PTC>(newMove, newMove.m_player)) {
                continue;
            }
            // otherwise, we found a valid move
            isMate = false;
            // stop search
            return false;
        }
        return true;
    };
    loopAllPMOs<FS, NPDT, CT, PTC, decltype(actOnPMO)>(b, actOnPMO);
    return isMate;
}

// Forward declare that the user will implement printBoard
template<::std::size_t FS, typename NPDT>
std::string printBoard(const BoardState<FS, NPDT>& b);

template<::std::size_t FS, typename NPDT, typename CT, ::std::size_t PTC>
::std::vector<BoardState<FS, NPDT>> StandardGenerateForwardMoves(const BoardState<FS, NPDT>& b) {
    ::std::vector<BoardState<FS, NPDT>> moves;
    auto actOnPMO = [&](const BoardState<FS, NPDT>& b, const PMO<FS, NPDT, CT>* pmo, size_t flatStartPos) {
            auto newMoves = pmo->getForwards(b, CT(flatStartPos));
            // Save all moves that do not move self into check
            for (auto newMove : newMoves) {
                if (!inCheck<FS, NPDT, CT, PTC>(newMove, newMove.m_player)) {
                    moves.push_back(newMove);
                }
                // else { std:: cout << "cannot do the following move because it moves into check:\n" << printBoard(newMove) << std::endl;}
            }
            // Do not break, go through all moves
            return true;
        };
    loopAllPMOs<FS, NPDT, CT, PTC, decltype(actOnPMO)>(b, actOnPMO); // FIXME: temp hardcode
    return moves;
}

template<::std::size_t FS, typename NPDT, typename CT, ::std::size_t PTC>
::std::vector<BoardState<FS, NPDT>> StandardGenerateReverseMoves(const BoardState<FS, NPDT>& b) {
    std::vector<BoardState<FS, NPDT>> moves;

    auto actOnPMO = [&](const BoardState<FS, NPDT>& b, const PMO<FS, NPDT, CT>* pmo, size_t flatStartPos) {
        auto newMoves = pmo->getReverses(b, CT(flatStartPos));
        // Save all unmoves that do not uncheck opponent, i.e. a state where opponent ended their turn in check.
        for (auto newMove : newMoves) {
            if (!inCheck<FS, NPDT, CT, PTC>(newMove, newMove.m_player)) {
                moves.push_back(newMove);
            }
            // else { std:: cout << "cannot do the following unmove because it moves into check:\n" << printBoard(newMove) << std::endl;}
        }
        // Do not break, go through all moves
        return true;
    };
    // Unpromotion function, basically the same but call getUnpromotions.
    auto actOnUnpromotionPMO = [&](const BoardState<FS, NPDT>& b, const PromotablePMO<FS, NPDT, CT, PTC>* pmo, size_t flatStartPos, piece_label_t unpromoted, piece_label_t promoted) {
        auto newMoves = pmo->getUnpromotions(b, CT(flatStartPos), unpromoted, promoted);
        // Save all unmoves that do not uncheck opponent, i.e. a state where opponent ended their turn in check.
        for (auto newMove : newMoves) {
            if (!inCheck<FS, NPDT, CT, PTC>(newMove, newMove.m_player)) {
                moves.push_back(newMove);
            }
        }
        // Do not break, go through all moves
        return true;
    };
    // reverse=true since we are generating unmoves for previous player-to-move, not current player.
    loopAllPMOs<FS, NPDT, CT, PTC, decltype(actOnPMO), decltype(actOnUnpromotionPMO)>(b, actOnPMO, true, actOnUnpromotionPMO); // FIXME: temp hardcode
    return moves;
}

template<::std::size_t FS, typename NPDT, typename CT, ::std::size_t PTC>
bool StandardCheckmateEvaluator(const BoardState<FS, NPDT>& b) {
    if (inMate<FS, NPDT, CT, PTC>(b)) {
        // check, i.e. if this turn was skipped then next turn the king could be captured.
        if (inCheck<FS, NPDT, CT, PTC>(b, !b.m_player)) {
            // check and mate
            return true; // TODO: should be return LOSS
        } else {
            // mate without check is stalemate
            return false; // TODO: should be return DRAW
        }
    }
    // If not a mate, game is ongoing
    return false; // TODO: should be return ONGOING
}
#endif