#ifndef STANDARD_PMO_H_
#define STANDARD_PMO_H_

#include "../../core/pmo.hpp"
#include "definitions.h"

#include <bitset>
#include <optional>
#include <array>
#include <memory.h>

class SlidePMO : public ChessModdablePMO {
    // the increment in each direction, stored as flattened coordinates.
    std::vector<Coords> moveOffsets;

public:
    virtual ::std::pair<::std::vector<ChessBoardState>, ::std::vector<Coords>> 
    getModdableMovesWithDisplacement(const ChessBoardState& b,  Coords piecePos) const override;
    virtual ::std::pair<::std::vector<ChessBoardState>, ::std::vector<Coords>> 
    getModdableUnmovesWithDisplacement(const ChessBoardState& b,  Coords piecePos) const override;

    SlidePMO(::std::vector<Coords> _moveOffsets) 
        : moveOffsets(_moveOffsets) { }

};

// class JumpPMO : public ChessModdablePMO { // Note: DirectedJump is just like Jump, but with a few more lines of code, so for now at least I am making all jumps directed.
//     // the displacement in each direction, stored as flattened coordinates.
//     std::vector<Coords> moveOffsets;

// public:
//     virtual ::std::pair<::std::vector<ChessBoardState>, ::std::vector<Coords>> 
//     getModdableMovesWithDisplacement(const ChessBoardState& b,  Coords piecePos) const override;
//     virtual ::std::pair<::std::vector<ChessBoardState>, ::std::vector<Coords>> 
//     getModdableUnmovesWithDisplacement(const ChessBoardState& b,  Coords piecePos) const override;

//     JumpPMO(::std::vector<Coords> _moveOffsets) 
//         : moveOffsets(_moveOffsets) { }
// };

// JumpPMO but moveOffsets are negated for black.
class DirectedJumpPMO : public ChessModdablePMO {
public:
    // The possible displacements for a white piece, stored as flattened coordinates.
    // ASSUMPTION: can just be negated for black pieces. Sufficient if rectangular board and moves is horizontally symmetric.
    std::vector<Coords> moveOffsets;

    // Parallel to moveOffsets, each vector for a moveOffset specifies which tiles must be empty so this piece can 'pass through' it (e.g. Xiangqi horse, double-jumping pawn).
    std::vector<std::vector<Coords>> obstructOffsets;

    virtual ::std::pair<::std::vector<ChessBoardState>, ::std::vector<Coords>> 
    getModdableMovesWithDisplacement(const ChessBoardState& b,  Coords piecePos) const override;
    virtual ::std::pair<::std::vector<ChessBoardState>, ::std::vector<Coords>> 
    getModdableUnmovesWithDisplacement(const ChessBoardState& b,  Coords piecePos) const override;

    DirectedJumpPMO(::std::vector<Coords> _moveOffsets) 
        : moveOffsets(_moveOffsets), obstructOffsets(_moveOffsets.size(), std::vector<Coords>()) { }

    DirectedJumpPMO(::std::vector<Coords> _moveOffsets, ::std::vector<std::vector<Coords>> _obstructOffsets) 
        : moveOffsets(_moveOffsets), obstructOffsets(_obstructOffsets) { }

    DirectedJumpPMO(::std::vector<Coords> _moveOffsets
        , ChessPMOPreModList preFwdMods, ChessPMOPostModList postFwdMods, ChessPMOPreModList preBwdMods, ChessPMOPostModList postBwdMods) 
        : moveOffsets(_moveOffsets), ChessModdablePMO(preFwdMods, postFwdMods, preBwdMods, postBwdMods) { }

    DirectedJumpPMO(::std::vector<Coords> _moveOffsets, ::std::vector<std::vector<Coords>> _obstructOffsets
        , ChessPMOPreModList preFwdMods, ChessPMOPostModList postFwdMods, ChessPMOPreModList preBwdMods, ChessPMOPostModList postBwdMods) 
        : moveOffsets(_moveOffsets), obstructOffsets(_obstructOffsets)
        , ChessModdablePMO(preFwdMods, postFwdMods, preBwdMods, postBwdMods) { }
};

namespace ChessPMOs {
    // need these functions for pawns. //TODO: can these be lambdas or something nicer looking? Idk
    inline bool isRank1(Coords coords) { return coords.rank == 0;}
    inline bool isRank2(Coords coords) { return coords.rank == 1;}
    inline bool isRank3(Coords coords) { return coords.rank == 2;}
    inline bool isRank4(Coords coords) { return coords.rank == 3;}
    inline bool isRank5(Coords coords) { return coords.rank == 4;}
    inline bool isRank6(Coords coords) { return coords.rank == 5;}
    inline bool isRank7(Coords coords) { return coords.rank == 6;}
    inline bool isRank8(Coords coords) { return coords.rank == 7;}

    const auto fwdCaptureRequiredMod = ChessFwdCaptDepPMO(true);
    const auto fwdCaptureProhibitedMod = ChessFwdCaptDepPMO(false);
    const auto bwdCaptureRequiredMod = ChessBwdCaptDepPMO(true);
    const auto bwdCaptureProhibitedMod = ChessBwdCaptDepPMO(false);
    const ChessPMOPreModList noPreMods;
    const ChessPMOPostModList noPostMods;

    const ChessPMOPostModList pawnForwardPostFwdMods = {&fwdCaptureProhibitedMod};
    const ChessPMOPostModList pawnForwardPostBwdMods = {&bwdCaptureProhibitedMod};

    const ChessPMOPostModList pawnAttackPostFwdMods = {&fwdCaptureRequiredMod};
    const ChessPMOPostModList pawnAttackPostBwdMods = {&bwdCaptureRequiredMod};


    const ChessDirRegionMod startOnSecondRank(&isRank2, &isRank7);
    const ChessDirRegionMod startOnFourthRank(&isRank4, &isRank5);

    const ChessPMOPreModList  pawnDJPreFwdMods = {&startOnSecondRank};
    const ChessPMOPostModList pawnDJPostFwdMods = {&fwdCaptureProhibitedMod};
    const ChessPMOPreModList  pawnDJPreBwdMods = {&startOnFourthRank};
    const ChessPMOPostModList pawnDJPostBwdMods = {&bwdCaptureProhibitedMod};

    const auto pawnForward = DirectedJumpPMO(std::vector<Coords>{{0, 1}}
        , noPreMods, pawnForwardPostFwdMods, noPreMods, pawnForwardPostBwdMods);
    const auto pawnAttack = DirectedJumpPMO(std::vector<Coords>{{-1, 1}, {1, 1}}
        , noPreMods, pawnAttackPostFwdMods, noPreMods, pawnAttackPostBwdMods);
    const auto pawnDouble = DirectedJumpPMO(std::vector<Coords>{{0, 2}}, std::vector<std::vector<Coords>>{{{0, 1}}}
        , pawnDJPreFwdMods, pawnDJPostFwdMods, pawnDJPreBwdMods, pawnDJPostBwdMods);
    const auto orthoSlide = SlidePMO(std::vector<Coords>{{-1, 0}, {1, 0}, {0, -1}, {0, 1}});
    const auto diagSlide = SlidePMO(std::vector<Coords>{{-1, -1}, {-1, 1}, {1, -1}, {1, 1}});
    const auto knightLeap = DirectedJumpPMO(std::vector<Coords>{
        {-2, -1}, {-2, 1}, {2, -1}, {2, 1},
        {-1, -2}, {-1, 2}, {1, -2}, {1, 2}});
    const auto kingMove = DirectedJumpPMO(std::vector<Coords>{
        {-1, 0}, {1, 0}, {0, -1}, {0, 1},
        {-1, -1}, {-1, 1}, {1, -1}, {1, 1}});
    const auto noopMove = DirectedJumpPMO(std::vector<Coords>{});

    const size_t pawnPMOsCount = 1;
    const ChessPMO* const pawnPMOs [pawnPMOsCount] = {&pawnDouble}; // {&pawnForward, &pawnAttack};
    const size_t rookPMOsCount = 1;
    const ChessPMO* const rookPMOs [rookPMOsCount] = {&orthoSlide};
    const size_t bishopPMOsCount = 1;
    const ChessPMO* const bishopPMOs [bishopPMOsCount] = {&diagSlide};
    const size_t knightPMOsCount = 1;
    const ChessPMO* const knightPMOs [knightPMOsCount] = {&knightLeap};
    const size_t queenPMOsCount = 2;
    const ChessPMO* const queenPMOs [queenPMOsCount] = {&orthoSlide, &diagSlide};
    const size_t kingPMOsCount = 1;
    const ChessPMO* const kingPMOs [kingPMOsCount] = {&noopMove};
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
// Can generate all PMOs for the player whose turn it is (forPlayerToMove=true) or for the other.
// TODO: move this functionality into core
template <typename ForEachPMOFunc>
void loopAllPMOs(const ChessBoardState& b, ForEachPMOFunc actOnPMO, bool forPlayerToMove=true) {
    // TODO: consider using a list of the positions of every type of piece, rather than brute-force checking all tiles
    for (size_t flatStartPos = 0; flatStartPos < 64; ++flatStartPos) {
        piece_label_t thisPiece = b.m_board.at(flatStartPos);
        if (isEmpty(thisPiece)) continue;
        // Ignore this piece if it is not for the player-to-move. Invert this result if forPlayerToMove=false.
        if (isWhite(thisPiece) ^ b.m_player ^ !forPlayerToMove) continue;

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
