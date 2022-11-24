#include "../../src/retrograde_analysis/state_transition.hpp"
#include "../../src/retrograde_analysis/checkmate_generation.hpp"
#include "../../src/retrograde_analysis/retrograde_analysis.hpp"

#include "../../src/rules/chess/interface.h"

#include <cassert>
#include <iostream>

template<typename BoardType, typename MapType, typename SuccFn>
auto probe(const BoardType& b, const MapType& m, SuccFn succFn, bool isWinIteration)
{
  BoardType g = b;
  int depthToEnd = 0;
  auto v = m.at(g);
  auto print = ChessBoardPrinter();
  std::vector<BoardType> pathwayToEnd = {g};
  for (;;)
  {
    auto succs = succFn(g);
    if (succs.size() == 0)
      break;
    std::cout << print(g) << std::endl;
    for (const auto& succ : succs)
    {
      // exploring a draw state
      if (m.find(succ) == m.end())
        continue;
      auto lvlToEnd = m.at(succ);
      // levels are doubled up per move
      if (isWinIteration && lvlToEnd == v - 1) 
      {
        g = succ;
        pathwayToEnd.push_back(g);
        ++depthToEnd;
        v = lvlToEnd;
        break;
      }
      else if (!isWinIteration && lvlToEnd == v)
      {
        g = succ;
        pathwayToEnd.push_back(g);
        ++depthToEnd;
        v = lvlToEnd;
        break;
      }
    }
    if (v == 0)
      break;
    isWinIteration = !isWinIteration;
  }
  return std::make_tuple(depthToEnd, pathwayToEnd);
}

int main()
{
  auto fwdMoveGenerator = ChessGenerateForwardMoves();
  auto revMoveGenerator = ChessGenerateReverseMoves();
  auto winCondEvaluator = ChessCheckmateEvaluator();
  auto boardPrinter = ChessBoardPrinter();
  auto validityEvaluator = ChessValidBoardEvaluator();

  constexpr ::std::size_t N      = 3;
  constexpr ::std::size_t ROW_SZ = 8;
  constexpr ::std::size_t COL_SZ = 8;
  
  // TODO: This is hardcoded for now. playout for correctness
  std::vector<piece_label_t> noRoyaltyPieceset = { 'q' };
  std::vector<piece_label_t> royaltyPieceset = { 'k', 'K' };

  std::vector<piece_label_t> fullPieceset = { 'k', 'K', 'q' };
  
  //auto checkmates = generateConfigCheckmates<64, ChessNPD, N, ROW_SZ, COL_SZ,
  //     decltype(winCondEvaluator)>(fullPieceset, winCondEvaluator);

  auto checkmates = generateParallelConfigCheckmates<64, ChessNPD, N, ROW_SZ, COL_SZ,
       decltype(winCondEvaluator), decltype(validityEvaluator)>(fullPieceset, winCondEvaluator, validityEvaluator);

  std::cout << checkmates.size() << std::endl;
  
  std::cout << "done with checkmates" << std::endl;

  auto [wins, losses, dtm] = retrogradeAnalysisBaseImpl<64, ChessNPD, N, ROW_SZ, 
    COL_SZ, decltype(fwdMoveGenerator), decltype(revMoveGenerator)>(::std::move(checkmates),
      fwdMoveGenerator, revMoveGenerator);

  std::cout << wins.size() << " " << losses.size() << std::endl;

  // Farthest state to end with optimal play for 3 man.
  // This is essentially the last state visited in the retrograde
  // analysis implementation
  BoardState<64, ChessNPD> longest3ManDTM
  {
    true,
    {
      '\0','\0','\0','\0','\0','\0','\0','\0',
      '\0','\0','\0','\0','\0','\0','\0','\0',
      '\0','\0','\0','\0','\0','\0','\0','\0',
      '\0','\0','\0','\0','\0','\0','\0','\0',
      '\0','\0','\0','\0', 'K','\0','\0','\0',
      '\0','\0','\0','\0','\0','\0','\0','\0',
      '\0', 'q','\0','\0','\0','\0','\0','\0',
      'k', '\0','\0','\0','\0','\0','\0','\0',
    },
    {-1}
  };

  auto [lengthToEnd, pathway] = probe(longest3ManDTM, dtm, fwdMoveGenerator, false);
  std::cout << "Number of moves until checkmate with optimal play: " << lengthToEnd << std::endl;
  
  assert(lengthToEnd == 20);
  std::cout << "test passed" << std::endl;
  return 0;
}
