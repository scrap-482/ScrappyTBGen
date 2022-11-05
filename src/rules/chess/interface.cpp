#include "interface.h"
#include "chess_pmo.h"

::std::vector<ChessBoardState> ChessGenerateForwardMoves::operator()(const ChessBoardState& b) {
    std::vector<ChessBoardState> moves;
    // TODO: consider using a list of the positions of every type of piece, rather than brute-force checking all tiles
    for (size_t flatStartPos = 0; flatStartPos < 64; ++flatStartPos) {
        piece_label_t thisPiece = b.m_board.at(flatStartPos);
        if (isEmpty(thisPiece)) continue;
        // Check if thisPiece is not the color of the player whose turn it is
        if (isWhite(thisPiece) != !!b.m_player[0]) continue;
        PIECE_TYPE_ENUM type = getTypeEnumFromPieceLabel(thisPiece);

        for (size_t i = 0; i < PIECE_TYPE_DATA[type].pmoListSize; ++i) {
            auto pmo = PIECE_TYPE_DATA[type].pmoList[i];
            auto newMoves = pmo->getForwards(b, unflatten(flatStartPos));
            moves.insert(moves.end(), newMoves.begin(), newMoves.end());
        }
    }
    return moves;
}

::std::vector<ChessBoardState> ChessGenerateReverseMoves::operator()(const ChessBoardState& b) {
    std::vector<ChessBoardState> moves;
    // TODO: consider using a list of the positions of every type of piece, rather than brute-force checking all tiles
    for (size_t flatStartPos = 0; flatStartPos < 64; ++flatStartPos) {
        piece_label_t thisPiece = b.m_board.at(flatStartPos);
        if (isEmpty(thisPiece)) continue;
        // Check if thisPiece is not the color of the player whose turn it is
        if (isWhite(thisPiece) != !!b.m_player[0]) continue;
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