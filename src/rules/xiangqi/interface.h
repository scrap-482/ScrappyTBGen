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


#ifndef XIANGQI_INTERFACE_HPP_
#define XIANGQI_INTERFACE_HPP_

#include "definitions.h"
#include "../../retrograde_analysis/state_transition.hpp"
#include "../../core/interface_logic.hpp"

class XiangqiGenerateForwardMoves : public GenerateForwardMoves<BOARD_FLAT_SIZE, XiangqiNPD> {
  public:
  ::std::vector<XiangqiBoardState> operator()(const XiangqiBoardState& b);
};

class XiangqiGenerateReverseMoves : public GenerateReverseMoves<BOARD_FLAT_SIZE, XiangqiNPD> {
  public:
  ::std::vector<XiangqiBoardState> operator()(const XiangqiBoardState& b);
};

class XiangqiCheckmateEvaluator : public CheckmateEvaluator<BOARD_FLAT_SIZE, XiangqiNPD> {
  public:
  bool operator()(const XiangqiBoardState& b);
};

class XiangqiBoardPrinter : public BoardPrinter<BOARD_FLAT_SIZE, XiangqiNPD> {
  public:
  ::std::string operator()(const XiangqiBoardState& b);
};

class XiangqiValidBoardEvaluator : ValidBoardEvaluator<BOARD_FLAT_SIZE, XiangqiNPD> {
public:
  bool operator()(const XiangqiBoardState& b);
};

#endif
