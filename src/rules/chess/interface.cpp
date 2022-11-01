#include "interface.h"

::std::vector<ChessBoardState> ChessGenerateForwardMoves::operator()(const ChessBoardState& b) {
    // TODO
    return std::vector<ChessBoardState>();
}

::std::vector<ChessBoardState> ChessGenerateReverseMoves::operator()(const ChessBoardState& b) {
    // TODO
    return std::vector<ChessBoardState>();
}

bool ChessCheckmateEvaluator::operator()(const ChessBoardState& b) {
    // TODO
    return false;
}
