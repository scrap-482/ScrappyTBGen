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


#ifndef XIANGQI_PMO_H_
#define XIANGQI_PMO_H_

#include "../../core/pmo_instantiable.hpp"
#include "../../core/pmo_mods.hpp"
#include "definitions.h"

#include <bitset>
#include <optional>
#include <array>
#include <memory.h>

using XiangqiPMO = PMO<BOARD_FLAT_SIZE, XiangqiNPD, Coords>;
using XiangqiModdablePMO = ModdablePMO<BOARD_FLAT_SIZE, XiangqiNPD, Coords, NUM_PIECE_TYPES>;
using XiangqiPromotablePMO = PromotablePMO<BOARD_FLAT_SIZE, XiangqiNPD, Coords, NUM_PIECE_TYPES>;
using XiangqiPieceType = PieceType<BOARD_FLAT_SIZE, XiangqiNPD, Coords>;

using XiangqiPMOPreMod = PMOPreMod<BOARD_FLAT_SIZE, XiangqiNPD, Coords>;
using XiangqiPMOPostMod = PMOPostMod<BOARD_FLAT_SIZE, XiangqiNPD, Coords>;
using XiangqiPMOPreModList = std::vector<const PMOPreMod<BOARD_FLAT_SIZE, XiangqiNPD, Coords>*>;
using XiangqiPMOPostModList = std::vector<const PMOPostMod<BOARD_FLAT_SIZE, XiangqiNPD, Coords>*>;

// TODO: this is an ugly syntax, is there a better way?
using XiangqiFwdCaptDepPMO = FwdCaptureDependentPMOPostMod<BOARD_FLAT_SIZE, XiangqiNPD, Coords>;
using XiangqiBwdCaptDepPMO = BwdCaptureDependentPMOPostMod<BOARD_FLAT_SIZE, XiangqiNPD, Coords>;

using XiangqiDirRegionMod = DirectedRegionPMOPreMod<BOARD_FLAT_SIZE, XiangqiNPD, Coords>;
using XiangqiPromotionFwdPostMod = RegionalForcedSinglePromotionPMOPostMod<BOARD_FLAT_SIZE, XiangqiNPD, Coords>;

using XiangqiSlidePMO = SlidePMO<BOARD_FLAT_SIZE, XiangqiNPD, Coords, NUM_PIECE_TYPES>;
using XiangqiDirectedJumpPMO = DirectedJumpPMO<BOARD_FLAT_SIZE, XiangqiNPD, Coords, NUM_PIECE_TYPES>;

namespace XiangqiPMOs {
    // need these functions for pawns. //TODO: can these be lambdas or something nicer looking? Idk
    inline bool isRank1(Coords coords) { return coords.rank == 0;}
    inline bool isRank2(Coords coords) { return coords.rank == 1;}
    inline bool isRank3(Coords coords) { return coords.rank == 2;}
    inline bool isRank4(Coords coords) { return coords.rank == 3;}
    inline bool isRank5(Coords coords) { return coords.rank == 4;}
    inline bool isRank6(Coords coords) { return coords.rank == 5;}
    inline bool isRank7(Coords coords) { return coords.rank == 6;}
    inline bool isRank8(Coords coords) { return coords.rank == 7;}

    const auto fwdCaptureRequiredMod = XiangqiFwdCaptDepPMO(true);
    const auto fwdCaptureProhibitedMod = XiangqiFwdCaptDepPMO(false);
    const auto bwdCaptureRequiredMod = XiangqiBwdCaptDepPMO(true);
    const auto bwdCaptureProhibitedMod = XiangqiBwdCaptDepPMO(false);

    const XiangqiDirRegionMod startOnSecondRank(&isRank2, &isRank7);
    const XiangqiDirRegionMod startOnFourthRank(&isRank4, &isRank5);
    const XiangqiDirRegionMod startOnEighthRank(&isRank8, &isRank1);

    // Note: this only handles promotion of forward moves.
    const auto promoteOnEighthRank = XiangqiPromotionFwdPostMod{&isRank8, &isRank1};

    const XiangqiPMOPreModList noPreMods;
    const XiangqiPMOPostModList noPostMods;

    // premods applied to pawn unpromotions
    const XiangqiPMOPreModList pawnUnpromotionPreMods = {&startOnEighthRank};

    const XiangqiPMOPostModList pawnForwardPostFwdMods = {&fwdCaptureProhibitedMod, &promoteOnEighthRank};
    const XiangqiPMOPostModList pawnForwardPostBwdMods = {&bwdCaptureProhibitedMod};

    const XiangqiPMOPostModList pawnAttackPostFwdMods = {&fwdCaptureRequiredMod, &promoteOnEighthRank};
    const XiangqiPMOPostModList pawnAttackPostBwdMods = {&bwdCaptureRequiredMod};

    // TODO: Double jump does not set En Passant Rights, because implementing En Passant is painful and maybe impossible to do efficiency
    const XiangqiPMOPreModList  pawnDJPreFwdMods = {&startOnSecondRank};
    const XiangqiPMOPostModList pawnDJPostFwdMods = {&fwdCaptureProhibitedMod}; // don't include &promoteOnEighthRank because double jump to promotion zone
    const XiangqiPMOPreModList  pawnDJPreBwdMods = {&startOnFourthRank};
    const XiangqiPMOPostModList pawnDJPostBwdMods = {&bwdCaptureProhibitedMod};

    const auto pawnForward = XiangqiDirectedJumpPMO(std::vector<Coords>{{0, 1}}
        , noPreMods, pawnForwardPostFwdMods, noPreMods, pawnForwardPostBwdMods, pawnUnpromotionPreMods, noPostMods);
    const auto pawnAttack = XiangqiDirectedJumpPMO(std::vector<Coords>{{-1, 1}, {1, 1}}
        , noPreMods, pawnAttackPostFwdMods, noPreMods, pawnAttackPostBwdMods, pawnUnpromotionPreMods, noPostMods);
    const auto pawnDouble = XiangqiDirectedJumpPMO(std::vector<Coords>{{0, 2}}, std::vector<std::vector<Coords>>{{{0, 1}}}
        , pawnDJPreFwdMods, pawnDJPostFwdMods, pawnDJPreBwdMods, pawnDJPostBwdMods, pawnUnpromotionPreMods, noPostMods);
    const auto orthoSlide = XiangqiSlidePMO(std::vector<Coords>{{-1, 0}, {1, 0}, {0, -1}, {0, 1}});
    const auto diagSlide = XiangqiSlidePMO(std::vector<Coords>{{-1, -1}, {-1, 1}, {1, -1}, {1, 1}});
    const auto knightLeap = XiangqiDirectedJumpPMO(std::vector<Coords>{
        {-2, -1}, {-2, 1}, {2, -1}, {2, 1},
        {-1, -2}, {-1, 2}, {1, -2}, {1, 2}});
    const auto kingMove = XiangqiDirectedJumpPMO(std::vector<Coords>{
        {-1, 0}, {1, 0}, {0, -1}, {0, 1},
        {-1, -1}, {-1, 1}, {1, -1}, {1, 1}});
    const auto noopMove = XiangqiDirectedJumpPMO(std::vector<Coords>{});

    const size_t pawnPMOsCount = 3;
    const XiangqiPMO* const pawnPMOs [pawnPMOsCount] = {&pawnForward, &pawnAttack, &pawnDouble};
    const size_t rookPMOsCount = 1;
    const XiangqiPMO* const rookPMOs [rookPMOsCount] = {&orthoSlide};
    const size_t bishopPMOsCount = 1;
    const XiangqiPMO* const bishopPMOs [bishopPMOsCount] = {&diagSlide};
    const size_t knightPMOsCount = 1;
    const XiangqiPMO* const knightPMOs [knightPMOsCount] = {&knightLeap};
    const size_t queenPMOsCount = 2;
    const XiangqiPMO* const queenPMOs [queenPMOsCount] = {&orthoSlide, &diagSlide};
    const size_t archbishopPMOsCount = 2;
    const XiangqiPMO* const archbishopPMOs [archbishopPMOsCount] = {&knightLeap, &diagSlide};
    const size_t chancellorPMOsCount = 2;
    const XiangqiPMO* const chancellorPMOs [chancellorPMOsCount] = {&knightLeap, &orthoSlide};
    const size_t kingPMOsCount = 1;
    const XiangqiPMO* const kingPMOs [kingPMOsCount] = {&kingMove};
}

// Note: this has to be parallel to PIECE_TYPE_ENUM
const XiangqiPieceType PIECE_TYPE_DATA[] = {
    {'p', XiangqiPMOs::pawnPMOs,   XiangqiPMOs::pawnPMOsCount,   false},
    {'r', XiangqiPMOs::rookPMOs,   XiangqiPMOs::rookPMOsCount,   false},
    {'n', XiangqiPMOs::knightPMOs, XiangqiPMOs::knightPMOsCount, false},
    {'b', XiangqiPMOs::bishopPMOs, XiangqiPMOs::bishopPMOsCount, false},
    {'q', XiangqiPMOs::queenPMOs,  XiangqiPMOs::queenPMOsCount,  false},
    {'a', XiangqiPMOs::archbishopPMOs,  XiangqiPMOs::archbishopPMOsCount,  false},
    {'c', XiangqiPMOs::chancellorPMOs,  XiangqiPMOs::chancellorPMOsCount,  false},
    {'k', XiangqiPMOs::kingPMOs,   XiangqiPMOs::kingPMOsCount,   true }
};
// Note: this has to be parallel to PIECE_TYPE_ENUM
// TODO: this syntax looks disgusting, is there a better way to write this?
template<::std::size_t FlattenedSz, typename NonPlacementDataType, typename Coords>
const PieceType<FlattenedSz, NonPlacementDataType, Coords>&
getPieceTypeData(piece_type_enum_t pieceEnum) {
       return PIECE_TYPE_DATA[pieceEnum];
}

#endif
