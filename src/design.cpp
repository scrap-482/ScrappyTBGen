#include <vector>


class Coordinates { // for now, this is just a wrapper for std:pair<int, int>

};

// An instance of this corresponds to a type of piece, e.g. there is a single Piece instance for pawns.
class PieceType {

    char letter;
    std::vector<PMO> pmoList;
    bool royalty; // TODO: does this need to encoode more than T/F?
    // note: in Chu Shogi, Drunk Elephant promotes to Prince (which is royal)

};

// The different sections of the game board
class Region {
    // Used by Chinese chess as rectangles
    // Chess pawn promotion can use regions
    // 
    // If single rectangular region, only one element. Weirdly shaped regions would require multiple
    // std::vector<std::pair<Coordinates, Coordinates>> regionSpan;
    std::pair<Coordinates, Coordinates> regionSpan;
};

enum class tribool {REQUIRED, PROHIBITED, OPTIONAL};

// How the piece's movement is affected by the precense of other pieces in its path.
// E.g. Rooks are sliding, Knights are leaping, Xiangqi Cannons are Cannon
enum class LocomotionMode {SLIDING, LEAPING, CANNON}; // More can, and probably will need to be added

class PMO {
    // like how en passant or castling affect flags in board state
    affectFlag; // type probably will be POD of which flag, function reference
};

// All other PMOs lol
class StdPMO extends PMO {
    std::vector<Region> allowedStartingRegions;
    std::vector<Region> allowedEndingRegions;
    LocomotionMode locomotionMode;

    tribool canCapture;

    // changes the PieceType
    std::optional<std::vector<PieceType>> promoteOptions;
    // Can on 
    std::optional<std::vector<PieceType>> captureWhitelist;

    // move geometry
    std::vector<Coordinates> moveVector;
    std::bitset<4> moveDirection;
    // for knights, horses, etc.
    std::optional<std::vector<Coordinates>> secondMoveVector;
    std::optional<std::bitset<4>> relativeSecondMoveDirection;

};

// like castling
class FixedPMO extends PMO {
    struct tileConditions {
        Coordinate coord; 
        std::optional<PieceType> piece; // TODO: does this need to be a set of pieces? I don't think so right now
        tribool underAttack;
    }; 
    std::vector<tileConditions> conditions;
};