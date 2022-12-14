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

#include "rules/chess/interface.h"
#include "rules/chess/pmo_specs.hpp"

#include <iostream>
#include <time.h>

/* -------------------------------------------------------------------------- */
// Test file for rulesets
/* -------------------------------------------------------------------------- */
// const ::std::array<piece_label_t, 64> EXAMPLE_ARRAY = { // Deep Blue vs Kasparov game 2
//   '\0','\0','\0','\0','r' ,'\0','\0','\0',
//   '\0','\0','\0','\0','\0','n' ,'\0','K' ,
//   'P' ,'P' ,'\0','\0','\0','p' ,'P' ,'P' ,
//   '\0','\0','\0','p' ,'\0','\0','\0','\0',
//   '\0','\0','\0','Q','\0','\0','N' ,'\0',
//   '\0','\0','\0','\0','\0','q' ,'\0','k' ,
//   '\0','\0','\0','\0','\0','\0','\0','R',
//   '\0','\0','\0','\0','\0','\0','\0','\0'

// };
const ::std::array<piece_label_t, 64> EXAMPLE_ARRAY = {
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0','K' ,'\0','\0','\0','\0','\0',
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0','\0','k','\0','\0','n' ,'\0',
  '\0','\0','\0','\0','\0','Q' ,'\0','\0',
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0','\0','\0','\0','\0','\0','\0'
};
const ::std::array<piece_label_t, 64> EXAMPLE_ARRAY_2 = {
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0','\0','K','\0','\0','n' ,'\0',
  '\0','\0','\0','\0','\0','q' ,'\0','\0',
  '\0','\0','\0','\0','\0','p' ,'p' ,'p' ,
  '\0','\0','R' ,'\0','\0','\0','k' ,'\0'
};
const ::std::array<piece_label_t, 64> EXAMPLE_ARRAY_3 = {
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0','\0','K' ,'\0','\0','\0','\0',
  '\0','\0','\0','\0','\0','\0','Q' ,'\0',
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0','\0','\0','\0','\0','\0','k'
};
const ::std::array<piece_label_t, 64> EXAMPLE_ARRAY_3_5 = {
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0','\0','\0','\0','\0','\0','\0',
  'K' ,'\0','\0','\0','\0','\0','\0','\0',
  '\0','\0','\0','\0','\0','\0','N' ,'\0',
  '\0','\0','\0','\0','\0','\0','\0','\0',
  'R' ,'\0','\0','\0','\0','\0','\0','k'
};
const ::std::array<piece_label_t, 64> EXAMPLE_ARRAY_3_6 = {
  '\0','\0','\0','\0','\0','\0','R' ,'\0',
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0','\0','\0','\0','\0','\0','\0',
  'K' ,'\0','\0','\0','\0','\0','\0','\0',
  '\0','\0','\0','\0','\0','\0','\0' ,'\0',
  '\0','\0','\0','\0','\0','R' ,'\0','\0',
  '\0','\0','\0','\0','\0','\0','\0','k'
};
const ::std::array<piece_label_t, 64> EXAMPLE_ARRAY_4 = {
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0','\0','\0','\0','P' ,'P' ,'P' ,
  '\0','\0','\0','\0','\0','P' ,'Q' ,'P' ,
  '\0','\0','\0','\0','\0','P' ,'\0','P' ,
  '\0','\0','\0','K' ,'\0','\0','k','\0'
};
const ::std::array<piece_label_t, 64> EXAMPLE_ARRAY_5 = {
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0','\0','\0','\0','\0','\0','r' ,
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0','\0','\0','\0','\0','q' ,'\0',
  '\0','\0','\0','\0','\0','\0','\0','B' ,
  '\0','\0','\0','\0','\0','k' ,'\0','K'
};
const ::std::array<piece_label_t, 64> EXAMPLE_ARRAY_6 = {
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0','\0','\0','\0','\0','\0','r' ,
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0','\0','\0','\0','k' ,'\0','K'
};
const ::std::array<piece_label_t, 64> EXAMPLE_ARRAY_7 = {
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0','\0','\0','\0','R' ,'\0','\0',
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0','\0','\0','\0','k' ,'\0','K'
};
const ::std::array<piece_label_t, 64> EXAMPLE_ARRAY_8 = {
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0','\0','\0','\0','k' ,'K' ,'\0'
};
/* -------------------- For testing reverseMoveGenerator -------------------- */
const ::std::array<piece_label_t, 64> EXAMPLE_ARRAY_1_1 = {
  'K' ,'n' ,'\0','\0','\0','\0','\0','\0',
  'n' ,'\0','\0','\0','\0','\0','\0','\0',
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0','\0','\0','\0','\0','\0','\0',
  'n' ,'\0','\0','\0','\0','\0','\0','\0',
  'R' ,'n' ,'\0','\0','\0','\0','\0','\0',
  '\0','\0','\0','\0','\0','\0','\0','k' 
};
/* -------------------------------------------------------------------------- */
const ::std::array<piece_label_t, 64> EXAMPLE_ARRAY_2_1 = {
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0','\0','K' ,'\0','\0','\0','\0',
  '\0','\0','\0','p' ,'\0','\0','\0','\0',
  '\0','\0','\0','\0','P' ,'\0','\0','\0',
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0','\0','\0','\0','\0','\0','k' 
};
const ::std::array<piece_label_t, 64> EXAMPLE_ARRAY_2_2 = {
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0','\0','K' ,'\0','P' ,'\0','\0',
  '\0','\0','P' ,'p' ,'\0','P','\0','\0',
  'P' ,'\0','\0','\0','P' ,'\0','\0','P' ,
  '\0','P' ,'\0','\0','\0','\0','\0','\0',
  'p' ,'p' ,'\0','\0','\0','\0','p' ,'\0',
  '\0','\0','\0','\0','\0','\0','\0','k' 
};
/* -------------------- For testing validEvaluator -------------------- */
const ::std::array<piece_label_t, 64> EXAMPLE_ARRAY_3_1 = {
  'K' ,'n' ,'\0','\0','\0','\0','\0','\0',
  'n' ,'\0','\0','\0','\0','\0','\0','\0',
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0','\0','\0','\0','\0','\0','\0',
  'n' ,'\0','\0','\0','\0','\0','\0','\0',
  'R' ,'n' ,'\0','\0','\0','\0','\0','\0',
  '\0','\0','p' ,'P' ,'\0','\0','\0','k' 
};
/* ------------------------ For testing (un)promotion ----------------------- */
const ::std::array<piece_label_t, 64> EXAMPLE_ARRAY_4_1 = {
  'K' ,'q' ,'\0','\0','\0','q' ,'\0','\0',
  '\0','\0','\0','\0','\0','\0','\0','q' ,
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0','\0','\0','\0','\0','\0','\0',
  'P' ,'\0','\0','\0','\0','\0','\0','\0',
  '\0','\0','P' ,'\0','\0','\0','\0','\0',
  '\0','\0','\0','Q' ,'\0','\0','\0','k' 
};
/* -------------------------------------------------------------------------- */
const ::std::array<piece_label_t, 64> EXAMPLE_ARRAY_5_1 = {
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0','\0','\0','\0','\0','\0','R' ,
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0','\0','\0','\0','K' ,'\0','k'
};


const ChessBoardState EXAMPLE_BOARD_STATE = {false, EXAMPLE_ARRAY, ChessNPD()};
const ChessBoardState EXAMPLE_CHECKMATE_STATE = {false, EXAMPLE_ARRAY_2, ChessNPD()};
const ChessBoardState EXAMPLE_STALEMATE_STATE = {false, EXAMPLE_ARRAY_3, ChessNPD()};
const ChessBoardState EXAMPLE_STALEMATE_STATE_1_2 = {false, EXAMPLE_ARRAY_3_5, ChessNPD()};
const ChessBoardState EXAMPLE_STALEMATE_STATE_1_3 = {false, EXAMPLE_ARRAY_3_6, ChessNPD()};
const ChessBoardState EXAMPLE_STALEMATE_STATE_2 = {true, EXAMPLE_ARRAY_5, ChessNPD()};
const ChessBoardState QUEEN_TEST_STATE = {true, EXAMPLE_ARRAY_4, ChessNPD()};
const ChessBoardState EXAMPLE_CHECK_STATE_1 = {true, EXAMPLE_ARRAY_6, ChessNPD()};
const ChessBoardState EXAMPLE_CHECK_STATE_2 = {false, EXAMPLE_ARRAY_7, ChessNPD()};
const ChessBoardState EXAMPLE_CHECK_STATE_3 = {false, EXAMPLE_ARRAY_8, ChessNPD()};
const ChessBoardState EXAMPLE_CHECK_STATE_4 = {true, EXAMPLE_ARRAY_8, ChessNPD()};

const ChessBoardState EXAMPLE_REV_CHECK_1 = {false, EXAMPLE_ARRAY_1_1, ChessNPD()};

const ChessBoardState EXAMPLE_PAWN_CHECK_1 = {false, EXAMPLE_ARRAY_2_2, ChessNPD()};
const ChessBoardState EXAMPLE_PAWN_CHECK_2 = {true, EXAMPLE_ARRAY_2_2, ChessNPD()};

const ChessBoardState EXAMPLE_INVALID_1 = {true, EXAMPLE_ARRAY_3_1, ChessNPD()};

const ChessBoardState EXAMPLE_PROMOTION_1 = {true, EXAMPLE_ARRAY_4_1, ChessNPD()};
const ChessBoardState EXAMPLE_PROMOTION_2 = {false, EXAMPLE_ARRAY_4_1, ChessNPD()};

const ChessBoardState EXAMPLE_BLACK_CHECKMATE_1 = {true, EXAMPLE_ARRAY_5_1, ChessNPD()};
const ChessBoardState EXAMPLE_BLACK_CHECKMATE_2 = {false, EXAMPLE_ARRAY_5_1, ChessNPD()};


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


  auto fwdMoveGenerator = ChessGenerateForwardMoves();
  auto revMoveGenerator = ChessGenerateReverseMoves();
  auto winCondEvaluator = ChessCheckmateEvaluator();
  auto boardPrinter = ChessBoardPrinter();
  auto validityEvaluator = ChessValidBoardEvaluator();

  std::cout << "=============================================\n Forward Move Gen and Win Condition Testing\n =============================================\n" << std::endl;
  std::vector<ChessBoardState> statesToTest = {INIT_BOARD_STATE, EXAMPLE_BOARD_STATE, EXAMPLE_CHECKMATE_STATE, EXAMPLE_STALEMATE_STATE, EXAMPLE_STALEMATE_STATE_1_2, EXAMPLE_STALEMATE_STATE_1_3, EXAMPLE_STALEMATE_STATE_2, QUEEN_TEST_STATE, EXAMPLE_CHECK_STATE_1, EXAMPLE_CHECK_STATE_2, EXAMPLE_CHECK_STATE_3, EXAMPLE_CHECK_STATE_4, EXAMPLE_INVALID_1, EXAMPLE_PROMOTION_1, EXAMPLE_PROMOTION_2, EXAMPLE_BLACK_CHECKMATE_1, EXAMPLE_BLACK_CHECKMATE_2};
  // std::vector<ChessBoardState> statesToTest = {EXAMPLE_BLACK_CHECKMATE_1, EXAMPLE_BLACK_CHECKMATE_2};
  for (auto state : statesToTest) {
    std::cout << boardPrinter(state) << std::endl;;
    // std::cout << "In Mate= " << inMate<64, ChessNPD, Coords, KING+1>(state) << std::endl;;
    std::cout << "WDL = " << winCondEvaluator(state) << std::endl;;
    std::cout << "Validity Evaluator: " << validityEvaluator(state) << std::endl;

    auto fwdMoves = fwdMoveGenerator(state);
    std::cout << "num forward moves: " << fwdMoves.size() << std::endl;
    if (fwdMoves.size() > 0) {
      // random selection. Yes I know this is biased, no I do not care.
      std::vector<ChessBoardState>::iterator randIt = fwdMoves.begin();
      std::advance(randIt, std::rand() % fwdMoves.size());
      std::cout << "example forward move:\n" << boardPrinter(*randIt) << std::endl;
    }
    std::cout << "--------------------------------------------\n" << std::endl;
  }
  std::cout << "=============================================\n Reverse Move Gen Testing\n=============================================\n" << std::endl;

  statesToTest = {EXAMPLE_PAWN_CHECK_1, EXAMPLE_PAWN_CHECK_2, EXAMPLE_PROMOTION_1, EXAMPLE_PROMOTION_2, EXAMPLE_REV_CHECK_1};
  for (auto state : statesToTest) {
    std::cout << boardPrinter(state) << std::endl;;
    auto revMoves = revMoveGenerator(state);
    std::cout << "num backwards moves: " << revMoves.size() << std::endl;
    if (revMoves.size() > 0) {
      // random selection. Yes I know this is biased, no I do not care.
      std::vector<ChessBoardState>::iterator randIt = revMoves.begin();
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
