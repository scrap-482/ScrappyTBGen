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


#include "interface.h"
#include "pmo_specs.hpp"

::std::vector<ChessBoardState> ChessGenerateForwardMoves::operator()(const ChessBoardState& b) {
    return StandardGenerateForwardMoves<64, ChessNPD, Coords, NUM_PIECE_TYPES>(b);
}

::std::vector<ChessBoardState> ChessGenerateReverseMoves::operator()(const ChessBoardState& b) {
    return StandardGenerateReverseMoves<64, ChessNPD, Coords, NUM_PIECE_TYPES>(b);
}

bool ChessCheckmateEvaluator::operator()(const ChessBoardState& b) {
    return StandardCheckmateEvaluator<64, ChessNPD, Coords, NUM_PIECE_TYPES>(b);
}

std::string ChessBoardPrinter::operator()(const ChessBoardState& b) {
    return printBoard(b);
}

bool ChessValidBoardEvaluator::operator()(const ChessBoardState& b) {
    // Check kings are not adjacent
    // First, find a king. //TODO: this could really be sped up by a piece list...
    size_t kingIndex = 0;
    for (; kingIndex < b.m_board.size(); ++kingIndex)
        if (b.m_board.at(kingIndex) == 'k' || b.m_board.at(kingIndex) == 'K') // TODO: fix hardcode. Prolly need isRoyal() function like isWhite()
            break;
    if (kingIndex == b.m_board.size())
        // return invalid if there are no kings on the board
        return false;

    Coords firstKingCoords(kingIndex);

    // check all adjacents of king
    for (const auto& coords : ChessPMOs::kingMove.moveOffsets) {
        Coords secondKingCoords = firstKingCoords + coords;
        if (!inBounds(secondKingCoords)) continue;

        if (b.m_board.at(secondKingCoords.flatten()) == 'k' || b.m_board.at(secondKingCoords.flatten()) == 'K') {
            // we did find an adjacent king, so board is invalid
            return false;
        }
    }

    // Check pawns are not in first rank of their side
    const std::vector<std::pair<piece_label_t, size_t>> piecesProhibitedFromRanks {{'P', 0}, {'p', 64-8}};
    for (auto pieceProhibitedFromRank : piecesProhibitedFromRanks) {
        auto rankValue = pieceProhibitedFromRank.second;
        auto pieceProhibited = pieceProhibitedFromRank.first;

        for (size_t fileIndex = 0; fileIndex < 8; ++fileIndex) {
            if (b.m_board.at(rankValue + fileIndex) == pieceProhibited) {
                return false;
            }
        }
    }

    // If underpromotion not allowed, check bishops are on opposite colors
    // TODO:

    // otherwise, nothing is wrong with this board that Retrograde Analyzer cares about.
    return true;
}
