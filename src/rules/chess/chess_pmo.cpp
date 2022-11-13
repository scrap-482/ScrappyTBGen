#include "chess_pmo.h"

#define inBounds(coords) ((coords.file >= 0 && coords.file < BOARD_WIDTH && coords.rank >= 0 && coords.rank < BOARD_HEIGHT))

::std::pair<::std::vector<ChessBoardState>, ::std::vector<Coords>> SlidePMO::getForwardsWithDisplacement(const ChessBoardState& b, Coords piecePos) const {
    return getForwardsWithDisplacement(b, piecePos, false);
}

::std::pair<::std::vector<ChessBoardState>, ::std::vector<Coords>> SlidePMO::getForwardsWithDisplacement(const ChessBoardState& b, Coords piecePos, bool otherPlayer) const {
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
            auto movedToContents = b.m_board.at(flattenCoords(pieceEndPos));
            if (!isEmpty(movedToContents)) {
                // if the player-to-move's color is the same as the piece we are moving to, not allowed.
                if (isWhite(movedToContents) == (!!b.m_player[0] ^ otherPlayer)) break;
                // Otherwise, this is a capture
                isCapture = true;
            } 
            ChessBoardState newState(b); // copy state. //TODO: is this an appropriately deep copy?

            newState.m_board.at(flattenCoords(pieceEndPos)) = newState.m_board.at(flattenCoords(piecePos));
            newState.m_board.at(flattenCoords(piecePos)) = '\0';
            // Any time we make a move, invert turn and reset en-passant. //TODO: smells bad doing this here, separate this functionality somehow.
            newState.m_player[0] = !newState.m_player[0]; 
            // TODO: reset en passant info

            resultStates.push_back(newState);
            resultDisplacements.push_back(displacementMultiplier * moveOffset);
            if (isCapture) break;
        }
    }
    return std::make_pair(std::move(resultStates), std::move(resultDisplacements));
}

void addUncaptures(const ChessBoardState& b, Coords piecePos, ::std::pair<::std::vector<ChessBoardState>, ::std::vector<Coords>>& res) {

    auto allowedUncapturesPieces = allowedUncapturesByPosAndCount(b, piecePos);
    auto loopLimit = res.first.size(); // specify this here since we are adding to vector.
    // for each unmove that does not uncapture
    for (size_t i = 0; i < loopLimit; ++i) {
        // for each type of piece we can uncapture
        for (int uncapTypeUncolored = 0; uncapTypeUncolored < allowedUncapturesPieces.size(); ++uncapTypeUncolored) {
            if (!allowedUncapturesPieces[uncapTypeUncolored]) continue;

            piece_label_t uncapType = TYPE_ENUM_TO_LABEL_T[uncapTypeUncolored];
            // Uncapture white when white to move, since prior turn black moved.
            uncapType = (b.m_player[0]? toWhite(uncapType) : toBlack(uncapType));

            // Write this uncapture as a new unmove
            auto newBoard = res.first.at(i); // copy
            newBoard.m_board.at(flattenCoords(piecePos)) = uncapType;
            res.first.push_back(newBoard);
            res.second.push_back(res.second.at(i)); // displacement of actual piece is the same
        }
    }
}

::std::pair<::std::vector<ChessBoardState>, ::std::vector<Coords>> SlidePMO::getReversesWithDisplacement(const ChessBoardState& b, Coords piecePos) const {
    // non-capture unmoves are same as forward moves, but with opposite person playing
    auto unmoves = getForwardsWithDisplacement(b, piecePos, true);
    addUncaptures(b, piecePos, unmoves);
    // then just add uncaptures
    return unmoves;
}

::std::pair<::std::vector<ChessBoardState>, ::std::vector<Coords>> JumpPMO::getForwardsWithDisplacement(const ChessBoardState& b, Coords piecePos) const {
    return getForwardsWithDisplacement(b, piecePos, false);
}

::std::pair<::std::vector<ChessBoardState>, ::std::vector<Coords>> JumpPMO::getForwardsWithDisplacement(const ChessBoardState& b, Coords piecePos, bool otherPlayer) const {
    std::vector<ChessBoardState> resultStates;
    // parallel to resultStates, describes the displacement of the moving piece's coords
    std::vector<Coords> resultDisplacements;

    for (auto moveOffset : moveOffsets) {
        Coords pieceEndPos = piecePos + moveOffset;
        if (!inBounds(pieceEndPos)) continue;

        auto movedToContents = b.m_board.at(flattenCoords(pieceEndPos));
        if (!isEmpty(movedToContents)) {
            // if the player-to-move's color is the same as the piece we are moving to, not allowed.
            if (isWhite(movedToContents) == (!!b.m_player[0] ^ otherPlayer)) continue;
            // Otherwise, this is a capture
        } 
        ChessBoardState newState(b); // copy state. //TODO: is this an appropriately deep copy?

        newState.m_board.at(flattenCoords(pieceEndPos)) = newState.m_board.at(flattenCoords(piecePos));
        newState.m_board.at(flattenCoords(piecePos)) = '\0';
        // Any time we make a move, invert turn and reset en-passant. //TODO: smells bad doing this here, separate this functionality somehow.
        newState.m_player[0] = !newState.m_player[0]; 
        // TODO: reset en passant info

        resultStates.push_back(newState);
        resultDisplacements.push_back(moveOffset);
    }
    
    return std::make_pair(std::move(resultStates), std::move(resultDisplacements));
}

::std::pair<::std::vector<ChessBoardState>, ::std::vector<Coords>> JumpPMO::getReversesWithDisplacement(const ChessBoardState& b, Coords piecePos) const {
    // non-capture unmoves are same as forward moves, but with opposite person playing
    auto unmoves = getForwardsWithDisplacement(b, piecePos, true);
    addUncaptures(b, piecePos, unmoves);
    // then just add uncaptures
    return unmoves;
}

std::unique_ptr<std::array<int, 2*NUM_PIECE_TYPES>> countPiecesOnBoard(const ChessBoardState& b) {
    auto res = std::make_unique<std::array<int, 2*NUM_PIECE_TYPES>>();
    res->fill(0); // TODO: not sure if this line is needed, or zero initialization already guaranteed
    for (piece_label_t p : b.m_board) {
        if (isEmpty(p)) continue;
        ++(*res)[toColoredTypeIndex(p)];
    }
    return std::move(res);
}

// Consider allowed uncaptures purely by number of pieces on the board
std::bitset<NUM_PIECE_TYPES> allowedUncapturesByCount(const ChessBoardState& b) {
    auto piecesCountByType = countPiecesOnBoard(b);
    std::bitset<NUM_PIECE_TYPES> res;

    int totalPieces = 0;
    for (auto c : *piecesCountByType) totalPieces += c;
    if (totalPieces >= MAN_LIMIT) return std::bitset<NUM_PIECE_TYPES>();
    // otherwise, we haven't hit the man limit. Check limit of number of pieces.

    // Consider when it is white's turn to play, black had the previous move and could have captured a white piece;
    // therefore we uncapture the color of the turn to play.
    int loopStart = !!b.m_player[0]? 0               : NUM_PIECE_TYPES;
    int loopEnd =   !!b.m_player[0]? NUM_PIECE_TYPES : 2 * NUM_PIECE_TYPES;
    int i = 0; // need to index of res
    for (size_t pieceType = loopStart; pieceType < loopEnd; ++pieceType) {
        if (piecesCountByType->at(pieceType) < MAX_PIECES_BY_TYPE.at(pieceType)) {
            res[i] = 1;
        } else {
            res[i] = 0;
        }
        ++i;
    }
    return res;
}

// Consider allowed uncaptures by count and position of uncaptured piece.
std::bitset<NUM_PIECE_TYPES> allowedUncapturesByPosAndCount(const ChessBoardState& b,  Coords piecePos) {
    std::bitset<NUM_PIECE_TYPES> allowed = allowedUncapturesByCount(b);
    // Check that pawns cannot be in first or last rank
    if (piecePos.rank == 0 || piecePos.rank == BOARD_HEIGHT-1) {
        allowed[PAWN] = 0;
    }
    // TODO: which file are pawns allowed on? Is there a limit to how many pawns can stack into a file I should care about?
    // TODO: bishops only allowed on opposite colors, unless we consider pawns which can promote to bishops.

    return allowed;
}

bool inCheck(const ChessBoardState& b, bool isWhiteAttacking) {
    // std::cout << "in inCheck start, isWhiteAttacking: " << isWhiteAttacking << std::endl;

    // TODO: check if multiple royal pieces. If like Chu Shogi all royal need to be captured, then return false if num royal > 0

    // TODO: seems kinda silly we need to copy the whole board just to flip player-to-move...
    // copy state so it is not const
    ChessBoardState bRev(b);
    // we will generate moves for side attacking
    bRev.m_player[0] = isWhiteAttacking;

    //std::cout << "in inCheck start, after copy bRev.m_player[0]: " << bRev.m_player[0] << std::endl;

    // Check is just saying that if the player were to move again, they could capture opponent king.
    // So check for king captures.
    bool isCheck = false;
    loopAllPMOs(bRev, 
    [&](const ChessBoardState& bRev, const ChessPMO* pmo, size_t flatStartPos) {
        auto startPos = unflatten(flatStartPos);
        auto newMovesWithDisplacement = ((ChessDisplacementPMO*) pmo)->getForwardsWithDisplacement(bRev, startPos); // Warning: cast assumes all moves are displacements
        
        for (size_t i = 0; i < newMovesWithDisplacement.first.size(); ++i) {
            auto move = newMovesWithDisplacement.first.at(i); // FIXME: Don't actually care about move board, remove this line
            auto displacement = newMovesWithDisplacement.second.at(i);

            // Check if this move is the capture of a king
            // ASSUMPTION: a piece can only capture a king by ending its turn on king's position
            auto endPos = startPos + displacement;

            if (getTypeEnumFromPieceLabel(bRev.m_board.at(flattenCoords(endPos))) == KING) {
            // TODO: if counting was fast, we could just use that on royal pieces, something like:
            // size_t kingIndex = toColoredTypeIndex(!bRev.m_player[0], KING);
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
        auto newMoves = pmo->getForwards(b, unflatten(flatStartPos));
        // Save all moves that do not move self into check
        for (auto newMove : newMoves) {
            // prune out moves where player-to-move checks their own king
            if (inCheck(newMove, newMove.m_player[0])) {
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
    ret += (b.m_player[0]? "White to move\n" : "Black to move\n");
    /* -------------------------------- Print NPD ------------------------------- */
    ret += "En passant rights: ";
    ret += (b.nonPlacementData.enpassantRights == -1)? "no" : ::std::to_string(b.nonPlacementData.enpassantRights);
    ret += "\n";
    return ret;
}