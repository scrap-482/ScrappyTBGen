#include "interface.h"
#include "chess_pmo.h"

::std::vector<ChessBoardState> ChessGenerateForwardMoves::operator()(const ChessBoardState& b) {
    ::std::vector<ChessBoardState> moves;
    auto actOnPMO = [&](const ChessBoardState& b, const ChessPMO* pmo, size_t flatStartPos) {
            auto newMoves = pmo->getForwards(b, Coords(flatStartPos));
            // Save all moves that do not move self into check
            for (auto newMove : newMoves) {
                if (!inCheck<64, ChessNPD, Coords, KING+1>(newMove, newMove.m_player)) {
                    moves.push_back(newMove);
                }
                // else { std:: cout << "cannot do the following move because it moves into check:\n" << printBoard(newMove) << std::endl;}
            }
            // Do not break, go through all moves
            return true;
        };
    loopAllPMOs<64, ChessNPD, Coords, KING+1, decltype(actOnPMO)>(b, actOnPMO); // FIXME: temp hardcode
    return moves;
}

::std::vector<ChessBoardState> ChessGenerateReverseMoves::operator()(const ChessBoardState& b) {
    std::vector<ChessBoardState> moves;

    auto actOnPMO = [&](const ChessBoardState& b, const ChessPMO* pmo, size_t flatStartPos) {
        auto newMoves = pmo->getReverses(b, Coords(flatStartPos));
        // Save all unmoves that do not uncheck opponent, i.e. a state where opponent ended their turn in check.
        for (auto newMove : newMoves) {
            if (!inCheck<64, ChessNPD, Coords, KING+1>(newMove, newMove.m_player)) {
                moves.push_back(newMove);
            }
            // else { std:: cout << "cannot do the following unmove because it moves into check:\n" << printBoard(newMove) << std::endl;}
        }
        // Do not break, go through all moves
        return true;
    };
    // Unpromotion function, basically the same but call getUnpromotions.
    auto actOnUnpromotionPMO = [&](const ChessBoardState& b, const ChessPromotablePMO* pmo, size_t flatStartPos, piece_label_t unpromoted, piece_label_t promoted) {
        auto newMoves = pmo->getUnpromotions(b, Coords(flatStartPos), unpromoted, promoted);
        // Save all unmoves that do not uncheck opponent, i.e. a state where opponent ended their turn in check.
        for (auto newMove : newMoves) {
            if (!inCheck<64, ChessNPD, Coords, KING+1>(newMove, newMove.m_player)) {
                moves.push_back(newMove);
            }
        }
        // Do not break, go through all moves
        return true;
    };
    // reverse=true since we are generating unmoves for previous player-to-move, not current player.
    loopAllPMOs<64, ChessNPD, Coords, KING+1, decltype(actOnPMO), decltype(actOnUnpromotionPMO)>(b, actOnPMO, true, actOnUnpromotionPMO); // FIXME: temp hardcode
    return moves;
}

bool ChessCheckmateEvaluator::operator()(const ChessBoardState& b) {
    if (inMate<64, ChessNPD, Coords, KING+1>(b)) {
        // check, i.e. if this turn was skipped then next turn the king could be captured.
        if (inCheck<64, ChessNPD, Coords, KING+1>(b, !b.m_player)) {
            // check and mate
            return true; // TODO: should be return LOSS
        } else {
            // mate without check is stalemate in chess
            return false; // TODO: should be return DRAW
        }
    }
    // If not a mate, game is ongoing
    return false; // TODO: should be return ONGOING
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
