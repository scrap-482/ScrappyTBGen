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
    bool pathFound = false;
    std::cout << print(g) << std::endl;
    std::cout << "v=" << v << " succ v= ";
    for (const auto& succ : succs)
    {
      // exploring a draw state
      if (m.find(succ) == m.end())
        continue;
      auto lvlToEnd = m.at(succ);
      std::cout << lvlToEnd << " ";
      // levels are doubled up per move
      if (isWinIteration && lvlToEnd == v - 1) 
      {
        //std::cout << lvlToEnd << std::endl;
        g = succ;
        pathwayToEnd.push_back(g);
        ++depthToEnd;
        v = lvlToEnd;
        std::cout << lvlToEnd << " " << depthToEnd << std::endl;
        pathFound = true;
        break;
      }
      else if (!isWinIteration && lvlToEnd == v)
      {
        //std::cout << lvlToEnd << std::endl;
        g = succ;
        pathwayToEnd.push_back(g);
        ++depthToEnd;
        std::cout << lvlToEnd << " " << depthToEnd << std::endl;
        v = lvlToEnd;
        pathFound = true;
        break;
      }
    }
    std::cout << std::endl;
    assert(pathFound);
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

  constexpr ::std::size_t N      = 4;
  constexpr ::std::size_t ROW_SZ = 8;
  constexpr ::std::size_t COL_SZ = 8;
  
  // TODO: This is hardcoded for now. playout for correctness
  std::vector<piece_label_t> noRoyaltyPieceset = { 'q', 'R' };
  std::vector<piece_label_t> royaltyPieceset = { 'k', 'K' };

  std::vector<piece_label_t> fullPieceset = { 'K', 'k', 'q', 'R'};
  
  //auto checkmates = generateConfigCheckmates<64, ChessNPD, N, ROW_SZ, COL_SZ,
  //     decltype(winCondEvaluator)>(fullPieceset, winCondEvaluator);

  auto checkmates = generateParallelConfigCheckmates<64, ChessNPD, N, ROW_SZ, COL_SZ,
       decltype(winCondEvaluator)>(fullPieceset, winCondEvaluator);

  std::cout << checkmates.size() << std::endl;
  
  std::cout << "done with checkmates" << std::endl;

  //auto [wins, losses, dtm] = retrogradeAnalysisBaseImpl<64, ChessNPD, N, ROW_SZ, 
  //  COL_SZ, decltype(fwdMoveGenerator), decltype(revMoveGenerator)>(::std::move(checkmates),
  //    fwdMoveGenerator, revMoveGenerator);

  //std::cout << wins.size() << " " << losses.size() << std::endl;
  
  return 0;
}
