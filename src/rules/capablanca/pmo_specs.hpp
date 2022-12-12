/*
* Copyright 2022 SCRAP
*
* This file is part of Scrappy Tablebase Generator.
* 
* Scrappy Tablebase Generator is free software: you can redistribute it and/or modify it under the terms 
* of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, 
* or (at your option) any later version.
* 
* Scrappy Tablebase Generator is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of 
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along with Scrappy Tablebase Generator. If not, see <https://www.gnu.org/licenses/>.
*/


#ifndef CAPABLANCA_PMO_H_
#define CAPABLANCA_PMO_H_

#include "../../core/pmo_instantiable.hpp"
#include "../../core/pmo_mods.hpp"
#include "definitions.h"

#include <bitset>
#include <optional>
#include <array>
#include <memory.h>

using CapablancaPMO = PMO<BOARD_FLAT_SIZE, CapablancaNPD, Coords>;
using CapablancaModdablePMO = ModdablePMO<BOARD_FLAT_SIZE, CapablancaNPD, Coords, NUM_PIECE_TYPES>;
using CapablancaPromotablePMO = PromotablePMO<BOARD_FLAT_SIZE, CapablancaNPD, Coords, NUM_PIECE_TYPES>;
using CapablancaPieceType = PieceType<BOARD_FLAT_SIZE, CapablancaNPD, Coords>;

using CapablancaPMOPreMod = PMOPreMod<BOARD_FLAT_SIZE, CapablancaNPD, Coords>;
using CapablancaPMOPostMod = PMOPostMod<BOARD_FLAT_SIZE, CapablancaNPD, Coords>;
using CapablancaPMOPreModList = std::vector<const PMOPreMod<BOARD_FLAT_SIZE, CapablancaNPD, Coords>*>;
using CapablancaPMOPostModList = std::vector<const PMOPostMod<BOARD_FLAT_SIZE, CapablancaNPD, Coords>*>;

// TODO: this is an ugly syntax, is there a better way?
using CapablancaFwdCaptDepPMO = FwdCaptureDependentPMOPostMod<BOARD_FLAT_SIZE, CapablancaNPD, Coords>;
using CapablancaBwdCaptDepPMO = BwdCaptureDependentPMOPostMod<BOARD_FLAT_SIZE, CapablancaNPD, Coords>;

using CapablancaDirRegionMod = DirectedRegionPMOPreMod<BOARD_FLAT_SIZE, CapablancaNPD, Coords>;
using CapablancaPromotionFwdPostMod = RegionalForcedSinglePromotionPMOPostMod<BOARD_FLAT_SIZE, CapablancaNPD, Coords>;

using CapablancaSlidePMO = SlidePMO<BOARD_FLAT_SIZE, CapablancaNPD, Coords, NUM_PIECE_TYPES>;
using CapablancaDirectedJumpPMO = DirectedJumpPMO<BOARD_FLAT_SIZE, CapablancaNPD, Coords, NUM_PIECE_TYPES>;

namespace CapablancaPMOs {
    // need these functions for pawns. //TODO: can these be lambdas or something nicer looking? Idk
    inline bool isRank1(Coords coords) { return coords.rank == 0;}
    inline bool isRank2(Coords coords) { return coords.rank == 1;}
    inline bool isRank3(Coords coords) { return coords.rank == 2;}
    inline bool isRank4(Coords coords) { return coords.rank == 3;}
    inline bool isRank5(Coords coords) { return coords.rank == 4;}
    inline bool isRank6(Coords coords) { return coords.rank == 5;}
    inline bool isRank7(Coords coords) { return coords.rank == 6;}
    inline bool isRank8(Coords coords) { return coords.rank == 7;}

    const auto fwdCaptureRequiredMod = CapablancaFwdCaptDepPMO(true);
    const auto fwdCaptureProhibitedMod = CapablancaFwdCaptDepPMO(false);
    const auto bwdCaptureRequiredMod = CapablancaBwdCaptDepPMO(true);
    const auto bwdCaptureProhibitedMod = CapablancaBwdCaptDepPMO(false);

    const CapablancaDirRegionMod startOnSecondRank(&isRank2, &isRank7);
    const CapablancaDirRegionMod startOnFourthRank(&isRank4, &isRank5);
    const CapablancaDirRegionMod startOnEighthRank(&isRank8, &isRank1);

    // Note: this only handles promotion of forward moves.
    const auto promoteOnEighthRank = CapablancaPromotionFwdPostMod{&isRank8, &isRank1};

    const CapablancaPMOPreModList noPreMods;
    const CapablancaPMOPostModList noPostMods;

    // premods applied to pawn unpromotions
    const CapablancaPMOPreModList pawnUnpromotionPreMods = {&startOnEighthRank};

    const CapablancaPMOPostModList pawnForwardPostFwdMods = {&fwdCaptureProhibitedMod, &promoteOnEighthRank};
    const CapablancaPMOPostModList pawnForwardPostBwdMods = {&bwdCaptureProhibitedMod};

    const CapablancaPMOPostModList pawnAttackPostFwdMods = {&fwdCaptureRequiredMod, &promoteOnEighthRank};
    const CapablancaPMOPostModList pawnAttackPostBwdMods = {&bwdCaptureRequiredMod};

    // TODO: Double jump does not set En Passant Rights, because implementing En Passant is painful and maybe impossible to do efficiency
    const CapablancaPMOPreModList  pawnDJPreFwdMods = {&startOnSecondRank};
    const CapablancaPMOPostModList pawnDJPostFwdMods = {&fwdCaptureProhibitedMod}; // don't include &promoteOnEighthRank because double jump to promotion zone
    const CapablancaPMOPreModList  pawnDJPreBwdMods = {&startOnFourthRank};
    const CapablancaPMOPostModList pawnDJPostBwdMods = {&bwdCaptureProhibitedMod};

    const auto pawnForward = CapablancaDirectedJumpPMO(std::vector<Coords>{{0, 1}}
        , noPreMods, pawnForwardPostFwdMods, noPreMods, pawnForwardPostBwdMods, pawnUnpromotionPreMods, noPostMods);
    const auto pawnAttack = CapablancaDirectedJumpPMO(std::vector<Coords>{{-1, 1}, {1, 1}}
        , noPreMods, pawnAttackPostFwdMods, noPreMods, pawnAttackPostBwdMods, pawnUnpromotionPreMods, noPostMods);
    const auto pawnDouble = CapablancaDirectedJumpPMO(std::vector<Coords>{{0, 2}}, std::vector<std::vector<Coords>>{{{0, 1}}}
        , pawnDJPreFwdMods, pawnDJPostFwdMods, pawnDJPreBwdMods, pawnDJPostBwdMods, pawnUnpromotionPreMods, noPostMods);
    const auto orthoSlide = CapablancaSlidePMO(std::vector<Coords>{{-1, 0}, {1, 0}, {0, -1}, {0, 1}});
    const auto diagSlide = CapablancaSlidePMO(std::vector<Coords>{{-1, -1}, {-1, 1}, {1, -1}, {1, 1}});
    const auto knightLeap = CapablancaDirectedJumpPMO(std::vector<Coords>{
        {-2, -1}, {-2, 1}, {2, -1}, {2, 1},
        {-1, -2}, {-1, 2}, {1, -2}, {1, 2}});
    const auto kingMove = CapablancaDirectedJumpPMO(std::vector<Coords>{
        {-1, 0}, {1, 0}, {0, -1}, {0, 1},
        {-1, -1}, {-1, 1}, {1, -1}, {1, 1}});
    const auto noopMove = CapablancaDirectedJumpPMO(std::vector<Coords>{});

    const size_t pawnPMOsCount = 3;
    const CapablancaPMO* const pawnPMOs [pawnPMOsCount] = {&pawnForward, &pawnAttack, &pawnDouble};
    const size_t rookPMOsCount = 1;
    const CapablancaPMO* const rookPMOs [rookPMOsCount] = {&orthoSlide};
    const size_t bishopPMOsCount = 1;
    const CapablancaPMO* const bishopPMOs [bishopPMOsCount] = {&diagSlide};
    const size_t knightPMOsCount = 1;
    const CapablancaPMO* const knightPMOs [knightPMOsCount] = {&knightLeap};
    const size_t queenPMOsCount = 2;
    const CapablancaPMO* const queenPMOs [queenPMOsCount] = {&orthoSlide, &diagSlide};
    const size_t archbishopPMOsCount = 2;
    const CapablancaPMO* const archbishopPMOs [archbishopPMOsCount] = {&knightLeap, &diagSlide};
    const size_t chancellorPMOsCount = 2;
    const CapablancaPMO* const chancellorPMOs [chancellorPMOsCount] = {&knightLeap, &orthoSlide};
    const size_t kingPMOsCount = 1;
    const CapablancaPMO* const kingPMOs [kingPMOsCount] = {&kingMove};
}

// Note: this has to be parallel to PIECE_TYPE_ENUM
const CapablancaPieceType PIECE_TYPE_DATA[] = {
    {'p', CapablancaPMOs::pawnPMOs,   CapablancaPMOs::pawnPMOsCount,   false},
    {'r', CapablancaPMOs::rookPMOs,   CapablancaPMOs::rookPMOsCount,   false},
    {'n', CapablancaPMOs::knightPMOs, CapablancaPMOs::knightPMOsCount, false},
    {'b', CapablancaPMOs::bishopPMOs, CapablancaPMOs::bishopPMOsCount, false},
    {'q', CapablancaPMOs::queenPMOs,  CapablancaPMOs::queenPMOsCount,  false},
    {'a', CapablancaPMOs::archbishopPMOs,  CapablancaPMOs::archbishopPMOsCount,  false},
    {'c', CapablancaPMOs::chancellorPMOs,  CapablancaPMOs::chancellorPMOsCount,  false},
    {'k', CapablancaPMOs::kingPMOs,   CapablancaPMOs::kingPMOsCount,   true }
};
// Note: this has to be parallel to PIECE_TYPE_ENUM
// TODO: this syntax looks disgusting, is there a better way to write this?
template<::std::size_t FlattenedSz, typename NonPlacementDataType, typename Coords>
const PieceType<FlattenedSz, NonPlacementDataType, Coords>&
getPieceTypeData(piece_type_enum_t pieceEnum) {
       return PIECE_TYPE_DATA[pieceEnum];
}

#endif
