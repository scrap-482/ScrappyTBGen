#ifndef STANDARD_CHESS_INTERFACE_HPP_
#define STANDARD_CHESS_INTERFACE_HPP_

#include "definitions.h"
#include "../../retrograde_analysis/state_transition.hpp"

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


#endif
