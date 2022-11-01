#ifndef PIECE_TYPE_H
#define PIECE_TYPE_H

#include "pmo.h"
#include "../utils/utils.h"

// An instance of this corresponds to a type of piece, e.g. there is a single Piece instance for pawns.
struct PieceType {

    char letter;
    std::vector<PMO> pmoList;
    bool royalty;
    // note: in Chu Shogi, Drunk Elephant promotes to Prince (which is royal)
    // But since these are given different PieceTypes (E vs E+), the royalty is
    // only assigned to the promoted piece's PieceType.

};

#endif