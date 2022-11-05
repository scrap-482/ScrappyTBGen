#include "chess_pmo.h"

::std::pair<::std::vector<ChessBoardState>, ::std::vector<Coords>> SlidePMO::getForwardsWithDisplacement(const ChessBoardState& b, Coords piecePos) {
    // TODO
    
    return std::make_pair(std::vector<ChessBoardState>(), std::vector<Coords>());
}

::std::pair<::std::vector<ChessBoardState>, ::std::vector<Coords>> SlidePMO::getReversesWithDisplacement(const ChessBoardState& b, Coords piecePos) {
    return getForwardsWithDisplacement(b, piecePos);
}

::std::pair<::std::vector<ChessBoardState>, ::std::vector<Coords>> JumpPMO::getForwardsWithDisplacement(const ChessBoardState& b, Coords piecePos) {
    // TODO
    return std::make_pair(std::vector<ChessBoardState>(), std::vector<Coords>());
}

::std::pair<::std::vector<ChessBoardState>, ::std::vector<Coords>> JumpPMO::getReversesWithDisplacement(const ChessBoardState& b, Coords piecePos) {
    return getForwardsWithDisplacement(b, piecePos);
}
