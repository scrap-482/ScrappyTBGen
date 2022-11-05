#include "chess_pmo.h"

#define inBounds(coords) ((coords.file >= 0 && coords.file < BOARD_WIDTH && coords.rank >= 0 && coords.rank < BOARD_HEIGHT))

::std::pair<::std::vector<ChessBoardState>, ::std::vector<Coords>> SlidePMO::getForwardsWithDisplacement(const ChessBoardState& b, Coords piecePos) const {
    std::vector<ChessBoardState> resultStates;
    // parallel to resultStates, describes the displacement of the moving piece's coords
    std::vector<Coords> resultDisplacements;

    for (auto moveOffset : moveOffsets) {
        // Keep moving in this direction until we have to stop
        Coords pieceEndPos = piecePos;
        while(true) {
            pieceEndPos += moveOffset;

            if (!inBounds(pieceEndPos)) break;

            bool isCapture = false;
            auto movedToContents = b.m_board.at(flattenCoords(pieceEndPos));
            if (!isEmpty(movedToContents)) {
                // if the player-to-move's color is the same as the piece we are moving to, not allowed.
                if (isWhite(movedToContents) == !!b.m_player[0]) break;
                // Otherwise, this is a capture
                isCapture = true;
            } 
            ChessBoardState newState(b); // copy state. //TODO: is this an appropriately deep copy?

            newState.m_board.at(flattenCoords(pieceEndPos)) = newState.m_board.at(flattenCoords(piecePos));
            newState.m_board.at(flattenCoords(piecePos)) = '\0';

            resultStates.push_back(newState);
            resultDisplacements.push_back(moveOffset);
            if (isCapture) break;
        }
    }
    return std::make_pair(std::move(resultStates), std::move(resultDisplacements));
}

::std::pair<::std::vector<ChessBoardState>, ::std::vector<Coords>> SlidePMO::getReversesWithDisplacement(const ChessBoardState& b, Coords piecePos) const {
    // TODO: un-captures
    return getForwardsWithDisplacement(b, piecePos);
}

::std::pair<::std::vector<ChessBoardState>, ::std::vector<Coords>> JumpPMO::getForwardsWithDisplacement(const ChessBoardState& b, Coords piecePos) const {
    std::vector<ChessBoardState> resultStates;
    // parallel to resultStates, describes the displacement of the moving piece's coords
    std::vector<Coords> resultDisplacements;

    for (auto moveOffset : moveOffsets) {
        Coords pieceEndPos = piecePos + moveOffset;
        if (!inBounds(pieceEndPos)) continue;

        auto movedToContents = b.m_board.at(flattenCoords(pieceEndPos));
        if (!isEmpty(movedToContents)) {
            // if the player-to-move's color is the same as the piece we are moving to, not allowed.
            if (isWhite(movedToContents) == !!b.m_player[0]) continue;
            // Otherwise, this is a capture
        } 
        ChessBoardState newState(b); // copy state. //TODO: is this an appropriately deep copy?

        newState.m_board.at(flattenCoords(pieceEndPos)) = newState.m_board.at(flattenCoords(piecePos));
        newState.m_board.at(flattenCoords(piecePos)) = '\0';

        resultStates.push_back(newState);
        resultDisplacements.push_back(moveOffset);
    }
    
    return std::make_pair(std::move(resultStates), std::move(resultDisplacements));
}

::std::pair<::std::vector<ChessBoardState>, ::std::vector<Coords>> JumpPMO::getReversesWithDisplacement(const ChessBoardState& b, Coords piecePos) const {
    // TODO: un-captures
    return getForwardsWithDisplacement(b, piecePos);
}
