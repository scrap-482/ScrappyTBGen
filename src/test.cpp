#include <iostream>
#include "retrograde_analysis/state_transition.hpp"
#include "retrograde_analysis/checkmate_generation.hpp"

#include "rules/chess/interface.h"

/* -------------------------------------------------------------------------- */
// Test file for rulesets
/* -------------------------------------------------------------------------- */
int main()
{
  std::vector<piece_label_t> noRoyaltyPieceset;
  for (int i=0; i < NUM_PIECE_TYPES; i++) {
    if (i == KING) continue;
    noRoyaltyPieceset.push_back(tolower(PIECE_TYPE_DATA[i].letter));
    noRoyaltyPieceset.push_back(toupper(PIECE_TYPE_DATA[i].letter));
  }
  std::vector<piece_label_t> royaltyPieceset = { 'k', 'K' };
  
  // need to fix
  auto symFn = [](const std::vector<piece_label_t>& v)
  {
    for (const auto& c : v)
    {
      if (c == 'p' || c =='P')
        return false;
    }
    return true;
  };
  
  auto c = cm_function<64, ChessNPD>{};
  auto [white_wins, white_losses] = generateAllCheckmates<64, ChessNPD, 3, 8, 8, decltype(c),
       decltype(symFn), decltype(symFn)>(noRoyaltyPieceset, royaltyPieceset, c, symFn, symFn);
  std::cout << white_wins.size() + white_losses.size() << std::endl;
  /* -------------------------------------------------------------------------- */


  auto fwdMoveGenerator = ChessGenerateForwardMoves();
  auto revMoveGenerator = ChessGenerateReverseMoves();
  auto winCondEvaluator = ChessCheckmateEvaluator();
  auto boardPrinter = ChessBoardPrinter();

  std::cout << boardPrinter(INIT_BOARD_STATE) << std::endl;;

  fwdMoveGenerator(INIT_BOARD_STATE);
  revMoveGenerator(INIT_BOARD_STATE);
  winCondEvaluator(INIT_BOARD_STATE);

  std::cout << "eep." << std::endl;
  std::cout << "Done." << std::endl;



}
