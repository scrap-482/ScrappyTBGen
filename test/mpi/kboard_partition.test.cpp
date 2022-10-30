#include "../../src/retrograde_analysis/cluster_support.hpp"
#include <iostream>

struct null_type {};

// TODO: asses distribution of this scheme
int main()
{
  constexpr std::size_t BoardSz = 64;
  constexpr std::size_t num_processes = 16;
  
  BoardState<BoardSz, null_type> b;
  b.m_board[32] = 'K';
  b.m_board[17] = 'k';
  b.m_board[1] = 'P';
  b.m_board[63] = 'N';
  b.m_board[45] = 'n';
  
  KStateSpacePartition<BoardSz, num_processes, decltype(b)> partitioner;

  std::cout << "Board corresponds to node " << partitioner(b) << std::endl;

  return 0;
}
