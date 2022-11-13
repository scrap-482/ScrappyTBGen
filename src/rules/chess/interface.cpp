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
    // TODO: consider using a list of the positions of every type of piece, rather than brute-force checking all tiles
    for (size_t flatStartPos = 0; flatStartPos < 64; ++flatStartPos) {
        piece_label_t thisPiece = b.m_board.at(flatStartPos);
        if (isEmpty(thisPiece)) continue;
        // Check if thisPiece is not the color of the player whose turn it was.
        // Note that since we are going in reverse, the player to unmove is opposite of the player to move.
        if (isWhite(thisPiece) == !!b.m_player[0]) continue;
        PIECE_TYPE_ENUM type = getTypeEnumFromPieceLabel(thisPiece);

        for (size_t i = 0; i < PIECE_TYPE_DATA[type].pmoListSize; ++i) {
            auto pmo = PIECE_TYPE_DATA[type].pmoList[i];
            auto newMoves = pmo->getReverses(b, unflatten(flatStartPos));
            moves.insert(moves.end(), newMoves.begin(), newMoves.end());
        }
    }
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