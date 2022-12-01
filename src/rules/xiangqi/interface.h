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
