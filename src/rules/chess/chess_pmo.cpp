#include "chess_pmo.h"

// Implement this function, which is required for allowedUncaptures defined in core.
template<::std::size_t FS, typename NPDT, typename CT, size_t NumPieceTypes>
std::bitset<NumPieceTypes> allowedUncapturesByPosAndCount(const BoardState<FS, NPDT>& b,  CT piecePos) {
    std::bitset<NumPieceTypes> allowed = allowedUncapturesByCount<FS, NPDT, CT, NumPieceTypes>(b);
    // Check that pawns cannot be in first or last rank
    if (piecePos.rank == 0 || piecePos.rank == BOARD_HEIGHT-1) {
        allowed[PAWN] = 0;
    }
    // TODO: which file are pawns allowed on? Is there a limit to how many pawns can stack into a file I should care about?
    // TODO: bishops only allowed on opposite colors, unless we consider pawns which can promote to bishops.
    // TODO: This feels like it overlaps with ValidityChecker somehow

    return allowed;
}

std::string printBoard(const ChessBoardState& b) {
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
    /* --------------------------- Print player's turn -------------------------- */
    ret += (b.m_player? "White to move\n" : "Black to move\n");
    /* -------------------------------- Print NPD ------------------------------- */
    ret += "En passant rights: ";
    ret += (b.nonPlacementData.enpassantRights == -1)? "no" : ::std::to_string(b.nonPlacementData.enpassantRights);
    ret += "\n";
    return ret;
}
