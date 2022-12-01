#include "interface.h"
#include "pmo_specs.hpp"

::std::vector<CapablancaBoardState> CapablancaGenerateForwardMoves::operator()(const CapablancaBoardState& b) {
    return StandardGenerateForwardMoves<BOARD_FLAT_SIZE, CapablancaNPD, Coords, NUM_PIECE_TYPES>(b);
}

::std::vector<CapablancaBoardState> CapablancaGenerateReverseMoves::operator()(const CapablancaBoardState& b) {
    return StandardGenerateReverseMoves<BOARD_FLAT_SIZE, CapablancaNPD, Coords, NUM_PIECE_TYPES>(b);
}

bool CapablancaCheckmateEvaluator::operator()(const CapablancaBoardState& b) {
    return StandardCheckmateEvaluator<BOARD_FLAT_SIZE, CapablancaNPD, Coords, NUM_PIECE_TYPES>(b);
}

std::string CapablancaBoardPrinter::operator()(const CapablancaBoardState& b) {
    return printBoard(b);
}

bool CapablancaValidBoardEvaluator::operator()(const CapablancaBoardState& b) {
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
    for (const auto& coords : CapablancaPMOs::kingMove.moveOffsets) {
        Coords secondKingCoords = firstKingCoords + coords;
        if (!inBounds(secondKingCoords)) continue;

        if (b.m_board.at(secondKingCoords.flatten()) == 'k' || b.m_board.at(secondKingCoords.flatten()) == 'K') {
            // we did find an adjacent king, so board is invalid
            return false;
        }
    }

    // Check pawns are not in first rank of their side
    const std::vector<std::pair<piece_label_t, size_t>> piecesProhibitedFromRanks {{'P', 0}, {'p', BOARD_FLAT_SIZE-BOARD_WIDTH}};
    for (auto pieceProhibitedFromRank : piecesProhibitedFromRanks) {
        auto rankValue = pieceProhibitedFromRank.second;
        auto pieceProhibited = pieceProhibitedFromRank.first;

        for (size_t fileIndex = 0; fileIndex < BOARD_WIDTH; ++fileIndex) {
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
