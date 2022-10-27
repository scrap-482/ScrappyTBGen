#include "../../src/retrograde_analysis/cluster_support.hpp"

#include <iostream>

int main()
{
  constexpr std::size_t BoardSz = 16;
  std::size_t num_processes = 4;
  
  for (std::size_t i = 0; i < num_processes; ++i)
  {
    KBoardPartition<BoardSz> board(i, num_processes);
    auto [lower, upper] = board.getRange(); 

    std::cout << "id: " << i << " lower range: " << lower << " upper range: " << upper << std::endl;
  }

  return 0;
}
