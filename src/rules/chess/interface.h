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


#ifndef STANDARD_CHESS_INTERFACE_HPP_
#define STANDARD_CHESS_INTERFACE_HPP_

#include "definitions.h"
#include "../../retrograde_analysis/state_transition.hpp"
#include "../../core/interface_logic.hpp"

class ChessGenerateForwardMoves : public GenerateForwardMoves<64, ChessNPD> {
  public:
  ::std::vector<ChessBoardState> operator()(const ChessBoardState& b);
};

class ChessGenerateReverseMoves : public GenerateReverseMoves<64, ChessNPD> {
  public:
  ::std::vector<ChessBoardState> operator()(const ChessBoardState& b);
};

class ChessCheckmateEvaluator : public CheckmateEvaluator<64, ChessNPD> {
  public:
  bool operator()(const ChessBoardState& b);
};

class ChessBoardPrinter : public BoardPrinter<64, ChessNPD> {
  public:
  ::std::string operator()(const ChessBoardState& b);
};

class ChessValidBoardEvaluator : ValidBoardEvaluator<64, ChessNPD> {
public:
  bool operator()(const ChessBoardState& b);
};

#endif
