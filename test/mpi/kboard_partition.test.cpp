#include "../../src/retrograde_analysis/cluster_support.hpp"

#include <iostream>

int main()
{
  constexpr std::size_t BoardSz = 64;
  std::size_t num_processes = 16;
  
  for (std::size_t i = 0; i < num_processes; ++i)
  {
    KStateSpacePartition<BoardSz> board(i, num_processes);
    auto [lower, upper] = board.getRange(); 

    std::cout << "id: " << i << " lower range: " << lower << " upper range: " << upper << std::endl;
  }

  return 0;
}
