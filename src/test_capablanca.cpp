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


#include "retrograde_analysis/state_transition.hpp"
#include "retrograde_analysis/checkmate_generation.hpp"

#include "rules/capablanca/interface.h"
#include "rules/capablanca/pmo_specs.hpp"

#include <iostream>
#include <time.h>

/* -------------------------------------------------------------------------- */
// Test file for rulesets
/* -------------------------------------------------------------------------- */
const ::std::array<piece_label_t, 80> EXAMPLE_ARRAY = {
  '\0','\0','\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0','\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0','\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0','\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0','\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','K' ,'\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','R' ,'\0','\0','\0','\0','\0','\0','\0','\0',
  'k' ,'\0','\0','\0','\0','\0','\0','\0','\0','c' 
};

const CapablancaBoardState EXAMPLE_BOARD_STATE = {false, EXAMPLE_ARRAY, CapablancaNPD()};

int main()
{
  srand(time(NULL));
  std::vector<piece_label_t> noRoyaltyPieceset;
  for (int i=0; i < NUM_PIECE_TYPES; i++) {
    if (i == KING) continue;
    noRoyaltyPieceset.push_back(tolower(PIECE_TYPE_DATA[i].letter)); // FIXME: assumes upper and lowercase separate white/black. Need towhite/toblack function
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
  
  /* -------------------------------------------------------------------------- */


  auto fwdMoveGenerator = CapablancaGenerateForwardMoves();
  auto revMoveGenerator = CapablancaGenerateReverseMoves();
  auto winCondEvaluator = CapablancaCheckmateEvaluator();
  auto boardPrinter = CapablancaBoardPrinter();
  auto validityEvaluator = CapablancaValidBoardEvaluator();

  std::cout << "=============================================\n Forward Move Gen and Win Condition Testing\n =============================================\n" << std::endl;
  std::vector<CapablancaBoardState> statesToTest = {EXAMPLE_BOARD_STATE};
  // std::vector<CapablancaBoardState> statesToTest = {EXAMPLE_BLACK_CHECKMATE_1, EXAMPLE_BLACK_CHECKMATE_2};
  for (auto state : statesToTest) {
    std::cout << boardPrinter(state) << std::endl;;
    // std::cout << "In Mate= " << inMate<64, CapablancaNPD, Coords, KING+1>(state) << std::endl;;
    std::cout << "WDL = " << winCondEvaluator(state) << std::endl;;
    std::cout << "Validity Evaluator: " << validityEvaluator(state) << std::endl;

    auto fwdMoves = fwdMoveGenerator(state);
    std::cout << "num forward moves: " << fwdMoves.size() << std::endl;
    if (fwdMoves.size() > 0) {
      // random selection. Yes I know this is biased, no I do not care.
      std::vector<CapablancaBoardState>::iterator randIt = fwdMoves.begin();
      std::advance(randIt, std::rand() % fwdMoves.size());
      std::cout << "example forward move:\n" << boardPrinter(*randIt) << std::endl;
    }
    std::cout << "--------------------------------------------\n" << std::endl;
  }
  std::cout << "=============================================\n Reverse Move Gen Testing\n=============================================\n" << std::endl;

  statesToTest = {EXAMPLE_BOARD_STATE};
  for (auto state : statesToTest) {
    std::cout << boardPrinter(state) << std::endl;;
    auto revMoves = revMoveGenerator(state);
    std::cout << "num backwards moves: " << revMoves.size() << std::endl;
    if (revMoves.size() > 0) {
      // random selection. Yes I know this is biased, no I do not care.
      std::vector<CapablancaBoardState>::iterator randIt = revMoves.begin();
      std::advance(randIt, std::rand() % revMoves.size());
      std::cout << "example reverse move:\n" << boardPrinter(*randIt) << std::endl;
    }
    std::cout << "--------------------------------------------\n" << std::endl;
  }
  std::cout << "=============================================\n Promotion Testing\n=============================================\n" << std::endl;
  std::cout << "p promotions: ";
  for (auto p : promotionScheme.getPromotions('p')) std::cout << p;
  std::cout << std::endl;

  std::cout << "p unpromotions: ";
  for (auto p : promotionScheme.getUnpromotions('p')) std::cout << p;
  std::cout << std::endl;

  std::cout << "q promotions: ";
  for (auto p : promotionScheme.getPromotions('q')) std::cout << p;
  std::cout << std::endl;

  std::cout << "q unpromotions: ";
  for (auto p : promotionScheme.getUnpromotions('q')) std::cout << p;
  std::cout << std::endl;

  std::cout << "n promotions: ";
  for (auto p : promotionScheme.getPromotions('n')) std::cout << p;
  std::cout << std::endl;

  std::cout << "n unpromotions: ";
  for (auto p : promotionScheme.getUnpromotions('n')) std::cout << p;
  std::cout << std::endl;

  std::cout << "P promotions: ";
  for (auto p : promotionScheme.getPromotions('P')) std::cout << p;
  std::cout << std::endl;

  std::cout << "P unpromotions: ";
  for (auto p : promotionScheme.getUnpromotions('P')) std::cout << p;
  std::cout << std::endl;

  std::cout << "Q promotions: ";
  for (auto p : promotionScheme.getPromotions('Q')) std::cout << p;
  std::cout << std::endl;

  std::cout << "Q unpromotions: ";
  for (auto p : promotionScheme.getUnpromotions('Q')) std::cout << p;
  std::cout << std::endl;

  std::cout << "Done." << std::endl;
}
