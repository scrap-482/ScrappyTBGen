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
