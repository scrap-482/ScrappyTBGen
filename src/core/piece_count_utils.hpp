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


#ifndef PIECE_COUNT_UTILS_H
#define PIECE_COUNT_UTILS_H

#include "piece_enum.hpp"
#include "../retrograde_analysis/state.hpp"

// Shorthand: 
// FS = FlattenedSize
// NPDT = NonPlacementDataType
// CT = CoordsType
// PTC = "Piece Type Count" AKA NumPieceTypes // TODO: bad naming, refactor?

// User needs to define how many pieces of each type can be on board
int maxPiecesByColoredType(size_t coloredType);

template<::std::size_t FS, typename NPDT, typename CT, size_t NumPieceTypes>
std::unique_ptr<std::array<int, 2*NumPieceTypes>> countPiecesOnBoard(const BoardState<FS, NPDT>& b) {
    auto res = std::make_unique<std::array<int, 2*NumPieceTypes>>();
    res->fill(0); // TODO: not sure if this line is needed, or zero initialization already guaranteed
    for (piece_label_t p : b.m_board) {
        if (isEmpty(p)) continue;
        ++(*res)[toColoredTypeIndex(p)];
    }
    return std::move(res);
}

// Consider allowed uncaptures purely by number of pieces on the board
// Returns bitset corresponding to PIECE_TYPE_ENUM, where 1 means uncapture of this type allowed.
template<::std::size_t FS, typename NPDT, typename CT, size_t NumPieceTypes>
std::bitset<NumPieceTypes> allowedUncapturesByCount(const BoardState<FS, NPDT>& b) {
    auto piecesCountByType = countPiecesOnBoard<FS, NPDT, CT, NumPieceTypes>(b);
    std::bitset<NumPieceTypes> res;

    int totalPieces = 0;
    for (auto c : *piecesCountByType) totalPieces += c;
    if (totalPieces >= MAN_LIMIT) return std::bitset<NumPieceTypes>();
    // otherwise, we haven't hit the man limit. Check limit of number of pieces.

    // Consider when it is white's turn to play, black had the previous move and could have captured a white piece;
    // therefore we uncapture the color of the turn to play.
    int loopStart = b.m_player? 0               : NumPieceTypes;
    int loopEnd =   b.m_player? NumPieceTypes : 2 * NumPieceTypes;
    int i = 0; // need to index of res
    for (size_t pieceType = loopStart; pieceType < loopEnd; ++pieceType) {
        if (piecesCountByType->at(pieceType) < maxPiecesByColoredType(pieceType)) { // TODO: change logic so that specifying -1 means an unlimited number of pieces of this type can be added, e.g. queens by promotion
            res[i] = 1;
        } else {
            res[i] = 0;
        }
        ++i;
    }
    return res;
}

// Consider allowed uncaptures by count and position of uncaptured piece.
// Returns bitset corresponding to PIECE_TYPE_ENUM, where 1 means uncapture of this type allowed.
// Forward declare, since user needs to specify how position affects uncaptures. //TODO: consider if there is a way of using existing validity checker so user doesn't have to define this function.
template<::std::size_t FS, typename NPDT, typename CT, size_t NumPieceTypes>
std::bitset<NumPieceTypes> allowedUncapturesByPosAndCount(const BoardState<FS, NPDT>& b,  CT piecePos);

// Give res as non-capturing unmoves, this adds legal uncaptures to it.
// ASSUMPTION: Pieces can only be removed from board via an opponent piece capturing them
template<::std::size_t FS, typename NPDT, typename CT, size_t NumPieceTypes>
void addUncaptures(const BoardState<FS, NPDT>& b, CT piecePos, ::std::pair<::std::vector<BoardState<FS, NPDT>>, ::std::vector<CT>>& res) {

    auto allowedUncapturesPieces = allowedUncapturesByPosAndCount<FS, NPDT, CT, NumPieceTypes>(b, piecePos);
    auto loopLimit = res.first.size(); // specify this here since we are adding to vector.
    // for each unmove that does not uncapture
    for (size_t i = 0; i < loopLimit; ++i) {
        // for each type of piece we can uncapture
        for (piece_type_enum_t uncapTypeUncolored = 0; uncapTypeUncolored < allowedUncapturesPieces.size(); ++uncapTypeUncolored) {
            if (!allowedUncapturesPieces[uncapTypeUncolored]) continue;

            piece_label_t uncapType = getPieceLabelFromTypeEnum((PIECE_TYPE_ENUM) uncapTypeUncolored);
            // Uncapture white when white to move, since prior turn black moved.
            uncapType = (b.m_player? toWhite(uncapType) : toBlack(uncapType));

            // Write this uncapture as a new unmove
            auto newBoard = res.first.at(i); // copy
            newBoard.m_board.at(piecePos.flatten()) = uncapType;
            res.first.push_back(newBoard);
            res.second.push_back(res.second.at(i)); // displacement of actual piece is the same
        }
    }
}

#endif