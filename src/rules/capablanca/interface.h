#ifndef CAPABLANCA_INTERFACE_HPP_
#define CAPABLANCA_INTERFACE_HPP_

#include "definitions.h"
#include "../../retrograde_analysis/state_transition.hpp"
#include "../../core/interface_logic.hpp"

class CapablancaGenerateForwardMoves : public GenerateForwardMoves<BOARD_FLAT_SIZE, CapablancaNPD> {
  public:
  ::std::vector<CapablancaBoardState> operator()(const CapablancaBoardState& b);
};

class CapablancaGenerateReverseMoves : public GenerateReverseMoves<BOARD_FLAT_SIZE, CapablancaNPD> {
  public:
  ::std::vector<CapablancaBoardState> operator()(const CapablancaBoardState& b);
};

class CapablancaCheckmateEvaluator : public CheckmateEvaluator<BOARD_FLAT_SIZE, CapablancaNPD> {
  public:
  bool operator()(const CapablancaBoardState& b);
};

class CapablancaBoardPrinter : public BoardPrinter<BOARD_FLAT_SIZE, CapablancaNPD> {
  public:
  ::std::string operator()(const CapablancaBoardState& b);
};

class CapablancaValidBoardEvaluator : ValidBoardEvaluator<BOARD_FLAT_SIZE, CapablancaNPD> {
public:
  bool operator()(const CapablancaBoardState& b);
};

#endif
