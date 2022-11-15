#include "../../src/retrograde_analysis/state_transition.hpp"
#include "../../src/retrograde_analysis/checkmate_generation.hpp"
#include "../../src/retrograde_analysis/retrograde_analysis.hpp"

#include "../../src/rules/chess/interface.h"

#include <iostream>

//void print_win(auto w, int v)
//{
//#pragma omp critical
//  {
//    std::cout << "found win at v=" << v << std::endl;
//    std::cout << "Player: " << w.m_player << std::endl;
//    auto b = w.m_board;
//    
//    int count = 0;
//    for (auto c : b)
//    {
//      if (c == '\0')
//        std::cout << "x ";
//      else
//        std::cout << c << " ";
//      ++count;
//      if (count % 8 == 0)
//        std::cout << std::endl;
//    }
//  }
//}

int main()
{
  auto fwdMoveGenerator = ChessGenerateForwardMoves();
  auto revMoveGenerator = ChessGenerateReverseMoves();
  auto winCondEvaluator = ChessCheckmateEvaluator();
  auto boardPrinter = ChessBoardPrinter();

  constexpr ::std::size_t N      = 3;
  constexpr ::std::size_t ROW_SZ = 8;
  constexpr ::std::size_t COL_SZ = 8;
  
  // TODO: This is hardcoded for now. playout for correctness
  std::vector<piece_label_t> noRoyaltyPieceset = { 'Q' };
  std::vector<piece_label_t> royaltyPieceset = { 'k', 'K' };

  std::vector<piece_label_t> fullPieceset = { 'k', 'K', 'q' };
  
  //auto checkmates = generateAllCheckmates<64, ChessNPD, N, ROW_SZ, COL_SZ,
  //  decltype(winCondEvaluator)>(noRoyaltyPieceset, royaltyPieceset, evaluator);
  
  auto checkmates = generateConfigCheckmates<64, ChessNPD, N, ROW_SZ, COL_SZ,
       decltype(winCondEvaluator)>(fullPieceset, winCondEvaluator);
  
  std::cout << "done with checkmates" << std::endl;

  auto [wins, losses] = retrogradeAnalysisBaseImpl<64, ChessNPD, 3, ROW_SZ, 
    COL_SZ, decltype(fwdMoveGenerator), decltype(revMoveGenerator)>(::std::move(checkmates),
      fwdMoveGenerator, revMoveGenerator);
  
  std::cout << wins.size() << " " << losses.size() << std::endl;
  //for (auto& l : checkmates)
  //  print_win(l, 0);

  return 0;
}
