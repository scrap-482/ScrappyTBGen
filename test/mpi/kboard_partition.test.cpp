#include "../../src/retrograde_analysis/multi_node_impl.hpp"
#include <iostream>
#include <cassert>

// TODO: asses distribution of this scheme
int main()
{
  constexpr std::size_t BoardSz = 64;
  constexpr std::size_t num_processes = 16;
  
  BoardState<BoardSz, null_type> b;
  b.m_board[21] = 'K';
  b.m_board[4] = 'k';
  b.m_board[1] = 'P';
  b.m_board[63] = 'N';
  b.m_board[45] = 'n';
  
  KStateSpacePartition<BoardSz, decltype(b)> partitioner('k', num_processes);

  for (std::size_t i = 0; i < num_processes; ++i)
  {
    auto [start, end] = partitioner.getRange(i);
    std::cout << "proc: " << i << " start: " << start << " end: " << end << std::endl;
  }
  
  auto targ_node = partitioner(b);
  std::cout << "Board corresponds to node " << targ_node << std::endl;
  assert(targ_node == 1);

  std::vector<std::size_t> indexPermutations = { 4, 0, 1, 2, 3, 5 };
  for (int i = 6; i < 64; ++i)
    indexPermutations.push_back(i);
  
  std::vector<std::size_t> startBoard(64);
  std::iota(startBoard.begin(), startBoard.end(), 0);

  auto in_range = partitioner.checkInRange(startBoard, indexPermutations);
  std::cout << in_range << std::endl;
  assert(in_range == false);

  return 0;
}
