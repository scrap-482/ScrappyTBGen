// user must include this file in their own implementation

#ifndef STATE_TRANSITION_HPP_
#define STATE_TRANSITION_HPP_

#include "state.hpp"

template<::std::size_t FlattenedSz>
class GenerateForwardMoves 
{
  virtual std::vector<BoardState> 
  operator()(const BoardState<FlattenedSz>& b);
};

template<::std::size_t FlattenedSz>
class GenerateReverseMoves 
{
  virtual std::vector<BoardState> 
  operator()(const BoardState<FlattenedSz>& b);
};

#endif 
