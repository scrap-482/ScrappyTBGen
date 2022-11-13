#ifndef STANDARD_PMO_H_
#define STANDARD_PMO_H_

#include "../../core/pmo.hpp"
#include "definitions.h"

#include <bitset>
#include <optional>
#include <array>
#include <memory.h>

class SlidePMO : public ChessDisplacementPMO {
    // the increment in each direction, stored as flattened coordinates.
    std::vector<Coords> moveOffsets;

public:
    virtual ::std::pair<::std::vector<ChessBoardState>, ::std::vector<Coords>> 
    getForwardsWithDisplacement(const ChessBoardState& b,  Coords piecePos) const override;
    virtual ::std::pair<::std::vector<ChessBoardState>, ::std::vector<Coords>> 
    getReversesWithDisplacement(const ChessBoardState& b,  Coords piecePos) const override;

    // hack to allow getReverses to call this. Same as before but plays for person not moving.
    virtual ::std::pair<::std::vector<ChessBoardState>, ::std::vector<Coords>>
    getForwardsWithDisplacement(const ChessBoardState& b,  Coords piecePos, bool otherPlayer) const;

    SlidePMO(::std::vector<Coords> _moveOffsets) 
        : moveOffsets(_moveOffsets) { }

};

class JumpPMO : public ChessDisplacementPMO {
    // the displacement in each direction, stored as flattened coordinates.
    std::vector<Coords> moveOffsets;

public:
    virtual ::std::pair<::std::vector<ChessBoardState>, ::std::vector<Coords>> 
    getForwardsWithDisplacement(const ChessBoardState& b,  Coords piecePos) const override;
    virtual ::std::pair<::std::vector<ChessBoardState>, ::std::vector<Coords>> 
    getReversesWithDisplacement(const ChessBoardState& b,  Coords piecePos) const override;

    // hack to allow getReverses to call this. Same as before but plays for person not moving.
    virtual ::std::pair<::std::vector<ChessBoardState>, ::std::vector<Coords>> 
    getForwardsWithDisplacement(const ChessBoardState& b, Coords piecePos, bool otherPlayer) const;

    JumpPMO(::std::vector<Coords> _moveOffsets) 
        : moveOffsets(_moveOffsets) { }
};


// JumpPMO but moveOffsets are negated for black.
class DirectedJumpPMO : public JumpPMO {
    virtual ::std::pair<::std::vector<ChessBoardState>, ::std::vector<Coords>> 
    getForwardsWithDisplacement(const ChessBoardState& b,  Coords piecePos) const override;
    virtual ::std::pair<::std::vector<ChessBoardState>, ::std::vector<Coords>> 
    getReversesWithDisplacement(const ChessBoardState& b,  Coords piecePos) const override;
};

// TODO: remake this class as a restriction
// A jumpPMO which also needs to check that a nearby square contains a certain piece.
class AdjacentCondJumpPMO : public JumpPMO {
    // the place to check stored as flattened coordinates, parallel to moveOffets.
    std::vector<Coords> adjOffsets;

    // What the adjacent piece is. Set to '\0' for empty. 
    piece_label_t adjPiece;
    //TODO: I don't like hardcoding these \0's everywhere, it smells bad to me

    // True if this space must be the adjPiece, false if it must not be.
    bool whitelistAdj;
    // Xiangqi cannon is specified as adjPiece='\0', whitelistAdj=false which means that it must be a non-empty space.
};

namespace ChessPMOs {
    // const auto pawnForward = PawnForwardPMO(); // TODO
    // const auto pawnAttack = PawnCapturePMO(); // TODO
    const auto orthoSlide = SlidePMO(std::vector<Coords>{{-1, 0}, {1, 0}, {0, -1}, {0, 1}});
    const auto diagSlide = SlidePMO(std::vector<Coords>{{-1, -1}, {-1, 1}, {1, -1}, {1, 1}});
    const auto knightLeap = JumpPMO(std::vector<Coords>{
        {-2, -1}, {-2, 1}, {2, -1}, {2, 1},
        {-1, -2}, {-1, 2}, {1, -2}, {1, 2}});
    const auto kingMove = JumpPMO(std::vector<Coords>{
        {-1, 0}, {1, 0}, {0, -1}, {0, 1},
        {-1, -1}, {-1, 1}, {1, -1}, {1, 1}});
    const auto noopMove = JumpPMO(std::vector<Coords>{});

    const size_t pawnPMOsCount = 1;
    const ChessPMO* const pawnPMOs [pawnPMOsCount] = {&kingMove}; // {&pawnForward, &pawnAttack}; //TODO:for now, pawns move like the king.
    const size_t rookPMOsCount = 1;
    const ChessPMO* const rookPMOs [rookPMOsCount] = {&orthoSlide};
    const size_t bishopPMOsCount = 1;
    const ChessPMO* const bishopPMOs [bishopPMOsCount] = {&diagSlide};
    const size_t knightPMOsCount = 1;
    const ChessPMO* const knightPMOs [knightPMOsCount] = {&knightLeap};
    const size_t queenPMOsCount = 2;
    const ChessPMO* const queenPMOs [queenPMOsCount] = {&orthoSlide, &diagSlide};
    const size_t kingPMOsCount = 1;
    const ChessPMO* const kingPMOs [kingPMOsCount] = {&kingMove};
}

// Note: this has to be parallel to PIECE_TYPE_ENUM
const ChessPieceType PIECE_TYPE_DATA[] = {
    {'p', ChessPMOs::pawnPMOs,   ChessPMOs::pawnPMOsCount,   false},
    {'r', ChessPMOs::rookPMOs,   ChessPMOs::rookPMOsCount,   false},
    {'n', ChessPMOs::knightPMOs, ChessPMOs::knightPMOsCount, false},
    {'b', ChessPMOs::bishopPMOs, ChessPMOs::bishopPMOsCount, false},
    {'q', ChessPMOs::queenPMOs,  ChessPMOs::queenPMOsCount,  false},
    {'k', ChessPMOs::kingPMOs,   ChessPMOs::kingPMOsCount,   true }
};

// Give res as non-capturing unmoves, this adds legal uncaptures to it.
void addUncaptures(const ChessBoardState& b, Coords piecePos, ::std::pair<::std::vector<ChessBoardState>, ::std::vector<Coords>>& res);

// Consider allowed uncaptures by count and position of uncaptured piece.
// Returns bitset corresponding to PIECE_TYPE_ENUM, where 1 means uncapture of this type allowed.
std::bitset<NUM_PIECE_TYPES> allowedUncapturesByPosAndCount(const ChessBoardState& b,  Coords piecePos);
// Consider allowed uncaptures purely by number of pieces on the board
// Returns bitset corresponding to PIECE_TYPE_ENUM, where 1 means uncapture of this type allowed.
std::bitset<NUM_PIECE_TYPES> allowedUncapturesByCount(const ChessBoardState& b);

std::unique_ptr<std::array<int, 2*NUM_PIECE_TYPES>> countPiecesOnBoard(const ChessBoardState& b);

// Looping over all PMOs is common functionality, but what to do with them varies; as such, this is implemented as a 
// template function.
// ForEachPMOFunc is assumed to have the type (std::vector<ChessBoardState> newMoves, const ChessBoardState& b) -> bool // TODO: update description
// TODO: move this functionality into core
template <typename ForEachPMOFunc>
void loopAllPMOs(const ChessBoardState& b, ForEachPMOFunc actOnPMO) {
    // TODO: consider using a list of the positions of every type of piece, rather than brute-force checking all tiles
    for (size_t flatStartPos = 0; flatStartPos < 64; ++flatStartPos) {
        piece_label_t thisPiece = b.m_board.at(flatStartPos);
        if (isEmpty(thisPiece)) continue;
        // Check if thisPiece is not the color of the player whose turn it is
        if (isWhite(thisPiece) != !!b.m_player[0]) continue;

        PIECE_TYPE_ENUM type = getTypeEnumFromPieceLabel(thisPiece);

        for (size_t i = 0; i < PIECE_TYPE_DATA[type].pmoListSize; ++i) {
            auto pmo = PIECE_TYPE_DATA[type].pmoList[i];
            // auto newMoves = pmo->getForwards(b, unflatten(flatStartPos)); // TODO: since all chess moves are DisplacementPMOs, consider using this information for speedup.

            // Break if function returns false
            // if (!onMoveFound(newMoves, b)) return;
            if (!actOnPMO(b, pmo, flatStartPos)) return;
        }
    }
    return;
}

// Determine if player isWhiteAttacking is attacking opponent's king
bool inCheck(const ChessBoardState& b, bool isWhiteAttacking);

bool inMate(const ChessBoardState& b);

std::string printBoard(const ChessBoardState& b);


#endif