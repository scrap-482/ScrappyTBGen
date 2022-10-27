// user must include this file in their own implementation

#ifndef STATE_TRANSITION_HPP_
#define STATE_TRANSITION_HPP_

#include <vector>

#include "state.hpp"

template<::std::size_t FlattenedSz, typename NonPlacementDataType>
class GenerateForwardMoves 
{
public:
  virtual ::std::vector<BoardState<FlattenedSz, NonPlacementDataType>> 
  operator()(const BoardState<FlattenedSz, NonPlacementDataType>& b) = 0;
};

template<::std::size_t FlattenedSz, typename NonPlacementDataType>
class GenerateReverseMoves 
{
public:
  virtual ::std::vector<BoardState<FlattenedSz, NonPlacementDataType>> 
  operator()(const BoardState<FlattenedSz, NonPlacementDataType>& b) = 0;
};

template<::std::size_t FlattenedSz, typename NonPlacementDataType>
class CheckmateEvaluator
{
public:
  virtual bool
  operator()(const BoardState<FlattenedSz, NonPlacementDataType>& b) = 0;
};

#endif 
