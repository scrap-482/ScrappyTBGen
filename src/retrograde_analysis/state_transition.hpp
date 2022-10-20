// user must include this file in their own implementation

#ifndef STATE_TRANSITION_HPP_
#define STATE_TRANSITION_HPP_

#include <vector>

#include "state.hpp"

template<::std::size_t FlattenedSz>
class GenerateForwardMoves 
{
  virtual ::std::vector<BoardState<FlattenedSz>> 
  operator()(const BoardState<FlattenedSz>& b) = 0;
};

template<::std::size_t FlattenedSz>
class GenerateReverseMoves 
{
  virtual ::std::vector<BoardState<FlattenedSz>> 
  operator()(const BoardState<FlattenedSz>& b) = 0;
};

template<::std::size_t FlattenedSz>
class CheckmateEvaluator
{
  virtual bool
  operator()(const BoardState<FlattenedSz>& b) = 0;
};

#endif 
