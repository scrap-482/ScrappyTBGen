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

/* 
 * user must include this file in their own implementation
 * This header defines many of the user callbacks that must be inherited and implemented
 */

#ifndef STATE_TRANSITION_HPP_
#define STATE_TRANSITION_HPP_

#include <vector>
#include <string>

#include "state.hpp"

// given a board state, generate all forward moves 
template<::std::size_t FlattenedSz, typename NonPlacementDataType>
class GenerateForwardMoves 
{
public:
  virtual ::std::vector<BoardState<FlattenedSz, NonPlacementDataType>> 
  operator()(const BoardState<FlattenedSz, NonPlacementDataType>& b) = 0;
};

// generate all moves that lead to the given board state 
template<::std::size_t FlattenedSz, typename NonPlacementDataType>
class GenerateReverseMoves 
{
public:
  virtual ::std::vector<BoardState<FlattenedSz, NonPlacementDataType>> 
  operator()(const BoardState<FlattenedSz, NonPlacementDataType>& b) = 0;
};

// determine if the given board state is a checkmate / end of game
template<::std::size_t FlattenedSz, typename NonPlacementDataType>
class CheckmateEvaluator
{
public:
  virtual bool
  operator()(const BoardState<FlattenedSz, NonPlacementDataType>& b) = 0;
};

// print the board in an unicode friendly manner 
template<::std::size_t FlattenedSz, typename NonPlacementDataType>
class BoardPrinter
{
  virtual ::std::string
  operator()(const BoardState<FlattenedSz, NonPlacementDataType>& b) = 0;
};

// determine if the given board state is indeed valid
template<::std::size_t FlattenedSz, typename NonPlacementDataType>
class ValidBoardEvaluator 
{
public:
  virtual bool
  operator()(const BoardState<FlattenedSz, NonPlacementDataType>& b) = 0;
};

#endif 
