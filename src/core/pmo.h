#ifndef PMO_H
#define PMO_H

#include "region.h"
#include "piece_type.h"

#include "../utils/utils.h"
#include "../utils/coords.hpp"

#include <optional>
#include <bitset>

// How the piece's movement is affected by the precense of other pieces in its path.
// E.g. Rooks are sliding, Knights are leaping, Xiangqi Cannons are Cannon
enum class LocomotionMode {SLIDING, LEAPING, CANNON}; // More can, and probably will need to be added

class PMO {
    // like how en passant or castling affect flags in board state
    void* affectFlag; // type probably will be POD of which flag, function reference
};

// All other PMOs lol
class StdPMO : public PMO {
    std::vector<Region> allowedStartingRegions;
    std::vector<Region> allowedEndingRegions;
    LocomotionMode locomotionMode;

    tribool canCapture;

    // changes the PieceType
    std::optional<std::vector<PieceType>> promoteOptions;
    // Can on 
    std::optional<std::vector<PieceType>> captureWhitelist;

    // move geometry
    std::vector<Coords> moveVector;
    std::bitset<4> moveDirection;
    // for knights, horses, etc.
    std::optional<std::vector<Coords>> secondMoveVector;
    std::optional<std::bitset<4>> relativeSecondMoveDirection;

};

// like castling
class FixedPMO : public PMO {
    struct tileConditions {
        Coords coord; 
        std::optional<PieceType> piece; // TODO: does this need to be a set of pieces? I don't think so right now
        tribool underAttack;
    }; 
    std::vector<tileConditions> conditions;
};


#endif