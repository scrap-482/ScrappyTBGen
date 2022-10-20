#ifndef PIECE_TYPE_H
#define PIECE_TYPE_H

#include "pmo.h"
#include "../utils/utils.h"

// An instance of this corresponds to a type of piece, e.g. there is a single Piece instance for pawns.
class PieceType {

    char letter;
    std::vector<PMO> pmoList;
    bool royalty; // TODO: does this need to encoode more than T/F?
    // note: in Chu Shogi, Drunk Elephant promotes to Prince (which is royal)

};

#endif