#include "interface.h"
#include "chess_pmo.h"

::std::vector<ChessBoardState> ChessGenerateForwardMoves::operator()(const ChessBoardState& b) {
    ::std::vector<ChessBoardState> moves;
    loopAllPMOs(b, 
        [&](const ChessBoardState& b, const ChessPMO* pmo, size_t flatStartPos) {
            auto newMoves = pmo->getForwards(b, unflatten(flatStartPos));
            // Save all moves that do not move self into check
            for (auto newMove : newMoves) {
                if (!inCheck(newMove, newMove.m_player[0])) {
                    moves.push_back(newMove);
                }
                // else { std:: cout << "cannot do the following move because it moves into check:\n" << printBoard(newMove) << std::endl;}
            }
            // Do not break, go through all moves
            return true;
        });
    return moves;
}

::std::vector<ChessBoardState> ChessGenerateReverseMoves::operator()(const ChessBoardState& b) {
    std::vector<ChessBoardState> moves;
    loopAllPMOs(b, 
        [&](const ChessBoardState& b, const ChessPMO* pmo, size_t flatStartPos) {
            auto newMoves = pmo->getReverses(b, unflatten(flatStartPos));
            // Save all unmoves that do not uncheck opponent, i.e. a state where opponent ended their turn in check.
            for (auto newMove : newMoves) {
                if (!inCheck(newMove, newMove.m_player[0])) {
                    moves.push_back(newMove);
                }
                // else { std:: cout << "cannot do the following unmove because it moves into check:\n" << printBoard(newMove) << std::endl;}
            }
            // Do not break, go through all moves
            return true;
        // forPlayerToMove=false since we are generating unmoves for previous player-to-move, not current player.
        }, false);
    // TODO: this is the same as fwd, but with a single line different. Maybe generalize?
    return moves;
}

bool ChessCheckmateEvaluator::operator()(const ChessBoardState& b) {
    if (inMate(b)) {
        // check, i.e. if this turn was skipped then next turn the king could be captured.
        if (inCheck(b, !b.m_player[0])) {
            // check and mate
            std::cout << "LOSS" << std::endl;
            return true; // TODO: should be return LOSS
        } else {
            // mate without check is stalemate in chess
            std::cout << "DRAW" << std::endl;
            return false; // TODO: should be return DRAW
        }
    }
    // If not a mate, game is ongoing
    std::cout << "ONGOING" << std::endl;
    return false; // TODO: should be return ONGOING
}

std::string ChessBoardPrinter::operator()(const ChessBoardState& b) {
    return printBoard(b);
}