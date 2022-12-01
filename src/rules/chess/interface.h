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

// State of the game returned by evaluator function. //TODO: rename and move this enum
enum GAME_VALUE_ENUM {LOSS, WIN, DRAW, ONGOING};

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
