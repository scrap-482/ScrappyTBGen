#ifndef STANDARD_PMO_H_
#define STANDARD_PMO_H_

#include "../../core/pmo_instantiable.hpp"
#include "../../core/pmo_mods.hpp"
#include "definitions.h"

#include <bitset>
#include <optional>
#include <array>
#include <memory.h>

using ChessPMO = PMO<64, ChessNPD, Coords>;
using ChessModdablePMO = ModdablePMO<64, ChessNPD, Coords, NUM_PIECE_TYPES>;
using ChessPromotablePMO = PromotablePMO<64, ChessNPD, Coords, NUM_PIECE_TYPES>;
using ChessPieceType = PieceType<64, ChessNPD, Coords>;

using ChessPMOPreMod = PMOPreMod<64, ChessNPD, Coords>;
using ChessPMOPostMod = PMOPostMod<64, ChessNPD, Coords>;
using ChessPMOPreModList = std::vector<const PMOPreMod<64, ChessNPD, Coords>*>;
using ChessPMOPostModList = std::vector<const PMOPostMod<64, ChessNPD, Coords>*>;

// TODO: this is an ugly syntax, is there a better way?
using ChessFwdCaptDepPMO = FwdCaptureDependentPMOPostMod<64, ChessNPD, Coords>;
using ChessBwdCaptDepPMO = BwdCaptureDependentPMOPostMod<64, ChessNPD, Coords>;

using ChessDirRegionMod = DirectedRegionPMOPreMod<64, ChessNPD, Coords>;
using ChessPromotionFwdPostMod = RegionalForcedSinglePromotionPMOPostMod<64, ChessNPD, Coords>;

using ChessSlidePMO = SlidePMO<64, ChessNPD, Coords, NUM_PIECE_TYPES>;
using ChessDirectedJumpPMO = DirectedJumpPMO<64, ChessNPD, Coords, NUM_PIECE_TYPES>;

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

    const ChessDirRegionMod startOnSecondRank(&isRank2, &isRank7);
    const ChessDirRegionMod startOnFourthRank(&isRank4, &isRank5);
    const ChessDirRegionMod startOnEighthRank(&isRank8, &isRank1);

    // Note: this only handles promotion of forward moves.
    const auto promoteOnEighthRank = ChessPromotionFwdPostMod{&isRank8, &isRank1};

    const ChessPMOPreModList noPreMods;
    const ChessPMOPostModList noPostMods;

    // premods applied to pawn unpromotions
    const ChessPMOPreModList pawnUnpromotionPreMods = {&startOnEighthRank};

    const ChessPMOPostModList pawnForwardPostFwdMods = {&fwdCaptureProhibitedMod, &promoteOnEighthRank};
    const ChessPMOPostModList pawnForwardPostBwdMods = {&bwdCaptureProhibitedMod};

    const ChessPMOPostModList pawnAttackPostFwdMods = {&fwdCaptureRequiredMod, &promoteOnEighthRank};
    const ChessPMOPostModList pawnAttackPostBwdMods = {&bwdCaptureRequiredMod};

    // TODO: Double jump does not set En Passant Rights, because implementing En Passant is painful and maybe impossible to do efficiency
    const ChessPMOPreModList  pawnDJPreFwdMods = {&startOnSecondRank};
    const ChessPMOPostModList pawnDJPostFwdMods = {&fwdCaptureProhibitedMod}; // don't include &promoteOnEighthRank because double jump to promotion zone
    const ChessPMOPreModList  pawnDJPreBwdMods = {&startOnFourthRank};
    const ChessPMOPostModList pawnDJPostBwdMods = {&bwdCaptureProhibitedMod};

    const auto pawnForward = ChessDirectedJumpPMO(std::vector<Coords>{{0, 1}}
        , noPreMods, pawnForwardPostFwdMods, noPreMods, pawnForwardPostBwdMods, pawnUnpromotionPreMods, noPostMods);
    const auto pawnAttack = ChessDirectedJumpPMO(std::vector<Coords>{{-1, 1}, {1, 1}}
        , noPreMods, pawnAttackPostFwdMods, noPreMods, pawnAttackPostBwdMods, pawnUnpromotionPreMods, noPostMods);
    const auto pawnDouble = ChessDirectedJumpPMO(std::vector<Coords>{{0, 2}}, std::vector<std::vector<Coords>>{{{0, 1}}}
        , pawnDJPreFwdMods, pawnDJPostFwdMods, pawnDJPreBwdMods, pawnDJPostBwdMods, pawnUnpromotionPreMods, noPostMods);
    const auto orthoSlide = ChessSlidePMO(std::vector<Coords>{{-1, 0}, {1, 0}, {0, -1}, {0, 1}});
    const auto diagSlide = ChessSlidePMO(std::vector<Coords>{{-1, -1}, {-1, 1}, {1, -1}, {1, 1}});
    const auto knightLeap = ChessDirectedJumpPMO(std::vector<Coords>{
        {-2, -1}, {-2, 1}, {2, -1}, {2, 1},
        {-1, -2}, {-1, 2}, {1, -2}, {1, 2}});
    const auto kingMove = ChessDirectedJumpPMO(std::vector<Coords>{
        {-1, 0}, {1, 0}, {0, -1}, {0, 1},
        {-1, -1}, {-1, 1}, {1, -1}, {1, 1}});
    const auto noopMove = ChessDirectedJumpPMO(std::vector<Coords>{});

    const size_t pawnPMOsCount = 3;
    const ChessPMO* const pawnPMOs [pawnPMOsCount] = {&pawnForward, &pawnAttack, &pawnDouble};
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
// Note: this has to be parallel to PIECE_TYPE_ENUM
// TODO: this syntax looks disgusting, is there a better way to write this?
template<::std::size_t FlattenedSz, typename NonPlacementDataType, typename Coords>
const PieceType<FlattenedSz, NonPlacementDataType, Coords>&
getPieceTypeData(piece_type_enum_t pieceEnum) {
       return PIECE_TYPE_DATA[pieceEnum];
}

// Looping over all PMOs is common functionality, but what to do with them varies; as such, this is implemented as a 
// template function.
// actOnPMO will act on every move (except unpromotions), while actOnUnpromotionPMO acts only on unpromotions.
// Set reverse=true to flip the color of who we are generating moves for (which is needed for all backwards moves).
// ForEachPMOFunc is assumed to have the type:
//     (std::vector<ChessBoardState> newMoves, const ChessBoardState& b) -> bool
// ForEachPMOUnpromotionFunc is assumed to have the type:
//     (std::vector<ChessBoardState> newMoves, const ChessBoardState& b, piece_label_t unpromotedPlt, piece_label_t thisPiece) -> bool
// Can generate all PMOs for the player whose turn it is (forPlayerToMove=true) or for the other.
// TODO: move this functionality into core
template <typename ForEachPMOFunc>
void loopAllPMOs(const ChessBoardState& b, ForEachPMOFunc actOnPMO, bool reverse=false) {
    const auto noopLambda = [&](const ChessBoardState& b, const ChessPMO* pmo, size_t flatStartPos, piece_label_t unpromoted, piece_label_t promoted) { return false; };
    loopAllPMOs<ForEachPMOFunc>(b, actOnPMO, reverse, noopLambda);
}
template <typename ForEachPMOFunc, typename ForEachPMOUnpromotionFunc>
void loopAllPMOs(const ChessBoardState& b, ForEachPMOFunc actOnPMO, bool reverse, ForEachPMOUnpromotionFunc actOnUnpromotionPMO) {
    // TODO: consider using a list of the positions of every type of piece, rather than brute-force checking all tiles
    for (size_t flatStartPos = 0; flatStartPos < 64; ++flatStartPos) {
        piece_label_t thisPiece = b.m_board.at(flatStartPos);
        if (isEmpty(thisPiece)) continue;
        // Ignore this piece if it is not for the player-to-move. Invert this result if forPlayerToMove=false.
        if (isWhite(thisPiece) ^ b.m_player ^ reverse) continue;

        PIECE_TYPE_ENUM type = getTypeEnumFromPieceLabel(thisPiece);

        for (size_t i = 0; i < getPieceTypeData<64, ChessNPD, Coords>(type).pmoListSize; ++i) {
            auto pmo = getPieceTypeData<64, ChessNPD, Coords>(type).pmoList[i];
            // Break if function returns false
            if (!actOnPMO(b, pmo, flatStartPos)) return;
        }

        // Now check for unpromotion PMOs
        if (reverse) {
            for (auto unpromotedPlt : promotionScheme.getUnpromotions(thisPiece)) {
                PIECE_TYPE_ENUM unpromotedType = getTypeEnumFromPieceLabel(unpromotedPlt);

                for (size_t i = 0; i < getPieceTypeData<64, ChessNPD, Coords>(unpromotedType).pmoListSize; ++i) {
                    // ASSUMPTION: every PMO held by a promotable piece is a PromotablePMO.
                    auto pmo = (ChessPromotablePMO*) getPieceTypeData<64, ChessNPD, Coords>(unpromotedType).pmoList[i];
                    // Break if function returns false
                    if (!actOnUnpromotionPMO(b, pmo, flatStartPos, unpromotedPlt, thisPiece)) return;
                }
            }
        }
    }
    return;
}

// Determine if player isWhiteAttacking is attacking opponent's king
bool inCheck(const ChessBoardState& b, bool isWhiteAttacking);

bool inMate(const ChessBoardState& b);

std::string printBoard(const ChessBoardState& b);


#endif
