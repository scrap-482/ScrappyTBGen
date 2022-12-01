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

bool inCheck(const ChessBoardState& b, bool isWhiteAttacking) {
    // std::cout << "in inCheck start, isWhiteAttacking: " << isWhiteAttacking << std::endl;

    // TODO: check if multiple royal pieces. If like Chu Shogi all royal need to be captured, then return false if num royal > 0

    // TODO: seems kinda silly we need to copy the whole board just to flip player-to-move...
    // copy state so it is not const
    ChessBoardState bRev(b);
    // we will generate moves for side attacking
    bRev.m_player = isWhiteAttacking;

    //std::cout << "in inCheck start, after copy bRev.m_player: " << bRev.m_player << std::endl;

    // Check is just saying that if the player were to move again, they could capture opponent king.
    // So check for king captures.
    bool isCheck = false;
    loopAllPMOs(bRev, 
    [&](const ChessBoardState& bRev, const ChessPMO* pmo, size_t flatStartPos) {
        auto startPos = Coords(flatStartPos);
        auto newMovesWithDisplacement = ((ChessModdablePMO*) pmo)->getForwardsWithDisplacement(bRev, startPos); // Warning: cast assumes all moves are displacements.
        
        for (size_t i = 0; i < newMovesWithDisplacement.first.size(); ++i) {
            // auto move = newMovesWithDisplacement.first.at(i);
            auto displacement = newMovesWithDisplacement.second.at(i);

            // Check if this move is the capture of a king
            // ASSUMPTION: a piece can only capture a king by ending its turn on king's position
            auto endPos = startPos + displacement;

            if (getTypeEnumFromPieceLabel(bRev.m_board.at(endPos.flatten())) == KING) {
            // TODO: if counting was fast, we could just use that on royal pieces, something like:
            // size_t kingIndex = toColoredTypeIndex(!bRev.m_player, KING);
            // if (countPiecesOnBoard(bRev)->at(kingIndex) == 0) {

                isCheck = true;
                // stop search
                return false;
            } 
        }
        return true;
    });
    return isCheck;
}


bool inMate(const ChessBoardState& b) {
    bool isMate = true;
    loopAllPMOs(b, 
    [&](const ChessBoardState& b, const ChessPMO* pmo, size_t flatStartPos) {
        auto newMoves = pmo->getForwards(b, Coords(flatStartPos));
        // Save all moves that do not move self into check
        for (auto newMove : newMoves) {
            // prune out moves where player-to-move checks their own king
            if (inCheck(newMove, newMove.m_player)) {
                continue;
            }
            // otherwise, we found a valid move
            isMate = false;
            // stop search
            return false;
        }
        return true;
    });
    return isMate;
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
