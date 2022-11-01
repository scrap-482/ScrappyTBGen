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

std::string ChessBoardPrinter::operator()(const ChessBoardState& b) {
    std::string ret = "";
    /* -------------------------- Print placement data -------------------------- */
    // go reverse in rank since the y-axis is mirrored (lower rank is further down on output)
    for (size_t rank = BOARD_HEIGHT-1; rank != (size_t)-1; --rank) {
        for (size_t file = 0; file < BOARD_WIDTH; ++file) {
            auto piece = b.m_board.at(file + rank*BOARD_WIDTH);
            ret += (piece == '\0')? '.' : piece;
        }
        ret += "\n";
    }
    /* -------------------------------- Print NPD ------------------------------- */
    ret += "En passant rights: ";
    ret += (b.nonPlacementData.enpassantRights == -1)? "no" : ::std::to_string(b.nonPlacementData.enpassantRights);
    ret += "\n";
    return ret;
}