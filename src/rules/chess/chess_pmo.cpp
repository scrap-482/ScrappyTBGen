#include "chess_pmo.h"

::std::pair<::std::vector<ChessBoardState>, ::std::vector<Coords>> SlidePMO::getModdableMovesWithDisplacement(const ChessBoardState& b, Coords piecePos) const {
    std::vector<ChessBoardState> resultStates;
    // parallel to resultStates, describes the displacement of the moving piece's coords
    std::vector<Coords> resultDisplacements;

    for (auto moveOffset : moveOffsets) {
        // Keep moving in this direction until we have to stop
        Coords pieceEndPos = piecePos;
        for (int displacementMultiplier = 1; true; ++displacementMultiplier) {
            pieceEndPos += moveOffset;

            if (!inBounds(pieceEndPos)) break;

            bool isCapture = false;
            auto movedToContents = b.m_board.at(pieceEndPos.flatten());
            if (!isEmpty(movedToContents)) {
                // if the player-to-move's color is the same as the piece we are moving to, not allowed.
                if (isWhite(movedToContents) == b.m_player) break;
                // Otherwise, this is a capture
                isCapture = true;
            } 
            ChessBoardState newState(b); // copy state

            newState.m_board.at(pieceEndPos.flatten()) = newState.m_board.at(piecePos.flatten());
            newState.m_board.at(piecePos.flatten()) = '\0';
            // Any time we make a move, invert turn and reset en-passant. //TODO: smells bad doing this here, separate this functionality somehow.
            newState.m_player = !newState.m_player; 
            // TODO: reset en passant info

            resultStates.push_back(newState);
            resultDisplacements.push_back(displacementMultiplier * moveOffset);
            if (isCapture) break;
        }
    }
    return std::make_pair(std::move(resultStates), std::move(resultDisplacements));
}

// TODO: can probably shared code between fwd and backward move generators, but I don't know quite what that looks like yet.
::std::pair<::std::vector<ChessBoardState>, ::std::vector<Coords>> SlidePMO::getModdableUnmovesWithDisplacement(const ChessBoardState& b, Coords piecePos) const {
    std::vector<ChessBoardState> resultStates;
    // parallel to resultStates, describes the displacement of the moving piece's coords
    std::vector<Coords> resultDisplacements;

    for (auto moveOffset : moveOffsets) {
        // Keep moving in this direction until we have to stop
        Coords pieceEndPos = piecePos;
        for (int displacementMultiplier = 1; true; ++displacementMultiplier) {
            pieceEndPos += moveOffset;

            if (!inBounds(pieceEndPos)) break;

            auto movedToContents = b.m_board.at(pieceEndPos.flatten());
            if (!isEmpty(movedToContents)) break; 
            ChessBoardState newState(b); // copy state

            newState.m_board.at(pieceEndPos.flatten()) = newState.m_board.at(piecePos.flatten());
            newState.m_board.at(piecePos.flatten()) = '\0';
            // Any time we make a move, invert turn
            newState.m_player = !newState.m_player; 

            resultStates.push_back(newState);
            resultDisplacements.push_back(displacementMultiplier * moveOffset);
        }
    }
    auto unmoves = std::make_pair(std::move(resultStates), std::move(resultDisplacements));
    addUncaptures<64, ChessNPD, Coords, KING+1>(b, piecePos, unmoves); //FIXME: hack
    // then just add uncaptures
    return unmoves;
}

::std::pair<::std::vector<ChessBoardState>, ::std::vector<Coords>> DirectedJumpPMO::getModdableMovesWithDisplacement(const ChessBoardState& b, Coords piecePos) const {
    std::vector<ChessBoardState> resultStates;
    // parallel to resultStates, describes the displacement of the moving piece's coords
    std::vector<Coords> resultDisplacements;

    for (size_t offsetIndex = 0; offsetIndex < moveOffsets.size(); ++offsetIndex) {
        auto moveOffset = moveOffsets.at(offsetIndex);
        // this is just the piece we are moving
        auto movedFromContents = b.m_board.at(piecePos.flatten());
        // negate if black
        if (!isWhite(movedFromContents)) moveOffset *= -1;

        Coords pieceEndPos = piecePos + moveOffset;
        if (!inBounds(pieceEndPos)) continue;

        bool obstructed = false;
        for (auto obstructOffset : obstructOffsets.at(offsetIndex)) {
            // negate if black
            if (!isWhite(movedFromContents)) obstructOffset *= -1;

            auto obstrContents = b.m_board.at((piecePos + obstructOffset).flatten());
            if (obstrContents != '\0') {
                obstructed = true;
                break;
            }
        }
        if (obstructed) continue;

        auto movedToContents = b.m_board.at(pieceEndPos.flatten());

        if (!isEmpty(movedToContents)) {
            // if the player-to-move's color is the same as the piece we are moving to, not allowed.
            if (isWhite(movedToContents) == b.m_player) continue;
            // Otherwise, this is a capture
        }
        ChessBoardState newState(b); // copy state.

        newState.m_board.at(pieceEndPos.flatten()) = newState.m_board.at(piecePos.flatten());
        newState.m_board.at(piecePos.flatten()) = '\0';
        // Any time we make a move, invert turn and reset en-passant. //TODO: smells bad doing this here, separate this functionality somehow.
        newState.m_player = !newState.m_player; 
        // TODO: reset en passant info

        resultStates.push_back(newState);
        resultDisplacements.push_back(moveOffset);
    }
    
    return std::make_pair(std::move(resultStates), std::move(resultDisplacements));
}

::std::pair<::std::vector<ChessBoardState>, ::std::vector<Coords>> DirectedJumpPMO::getModdableUnmovesWithDisplacement(const ChessBoardState& b, Coords piecePos) const {
    std::vector<ChessBoardState> resultStates;
    // parallel to resultStates, describes the displacement of the moving piece's coords
    std::vector<Coords> resultDisplacements;

    for (size_t offsetIndex = 0; offsetIndex < moveOffsets.size(); ++offsetIndex) {
        auto moveOffset = moveOffsets.at(offsetIndex);
        // this is just the piece we are moving
        auto movedFromContents = b.m_board.at(piecePos.flatten());
        // negate since this is UNmove, then negate again if black
        if (isWhite(movedFromContents)) moveOffset *= -1;

        Coords pieceEndPos = piecePos + moveOffset;
        if (!inBounds(pieceEndPos)) continue;

        bool obstructed = false;
        for (auto obstructOffset : obstructOffsets.at(offsetIndex)) {
            // negate since this is UNmove, then negate again if black
            if (isWhite(movedFromContents)) obstructOffset *= -1;

            auto obstrContents = b.m_board.at((piecePos + obstructOffset).flatten());
            if (obstrContents != '\0') {
                obstructed = true;
                break;
            }
        }
        if (obstructed) continue;

        auto movedToContents = b.m_board.at(pieceEndPos.flatten());
        // ASSUMPTION: moves leave behind empty tile
        if (!isEmpty(movedToContents)) continue; // TODO: consider abstracting fwd and rev move to have less repetition
        ChessBoardState newState(b); // copy state.

        newState.m_board.at(pieceEndPos.flatten()) = newState.m_board.at(piecePos.flatten());
        newState.m_board.at(piecePos.flatten()) = '\0';
        // Any time we make an unmove, invert turn.
        newState.m_player = !newState.m_player; 

        resultStates.push_back(newState);
        resultDisplacements.push_back(moveOffset);
    }
    
    auto unmoves = std::make_pair(std::move(resultStates), std::move(resultDisplacements));
    // non-capture unmoves are same as forward moves, but with opposite person playing
    addUncaptures<64, ChessNPD, Coords, KING+1>(b, piecePos, unmoves); // FIXME: hack
    // then just add uncaptures
    return unmoves;
}

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
