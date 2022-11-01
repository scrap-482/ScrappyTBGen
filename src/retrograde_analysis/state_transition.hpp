// user must include this file in their own implementation

#ifndef STATE_TRANSITION_HPP_
#define STATE_TRANSITION_HPP_

#include <vector>
#include <string>

#include "state.hpp"

template<::std::size_t FlattenedSz, typename NonPlacementDataType>
class GenerateForwardMoves 
{
  virtual ::std::vector<BoardState<FlattenedSz, NonPlacementDataType>> 
  operator()(const BoardState<FlattenedSz, NonPlacementDataType>& b) = 0;
};

template<::std::size_t FlattenedSz, typename NonPlacementDataType>
class GenerateReverseMoves 
{
  virtual ::std::vector<BoardState<FlattenedSz, NonPlacementDataType>> 
  operator()(const BoardState<FlattenedSz, NonPlacementDataType>& b) = 0;
};

template<::std::size_t FlattenedSz, typename NonPlacementDataType>
class CheckmateEvaluator
{
  virtual bool
  operator()(const BoardState<FlattenedSz, NonPlacementDataType>& b) = 0;
};

template<::std::size_t FlattenedSz, typename NonPlacementDataType>
class BoardPrinter
{
  virtual ::std::string
  operator()(const BoardState<FlattenedSz, NonPlacementDataType>& b) = 0;
};

#endif 
