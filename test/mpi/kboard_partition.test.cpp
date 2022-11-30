#include <iostream>
#include <vector>
#include <array>
#include <cassert>
#include <algorithm>
#include <numeric>

#include "../../src/retrograde_analysis/state.hpp"

struct null_type {};

template <std::size_t FlattenedSz, typename NonPlacementDataType, typename Partitioner>
auto assert_operator_consistency(const Partitioner& partitioner, const std::vector<unsigned char>& pieceSet, int proc_id)
{
  ::std::array<::std::size_t, FlattenedSz> indexPermutations;
  auto [startFirstIdx, endFirstIdx] = partitioner.getRange(proc_id);
  ::std::size_t kPermute = pieceSet.size();
  std::size_t count = 0;
   
  // generates [3, ..., kPermute] sets of new checkmate positions.
  for (::std::size_t kPermute = 3; kPermute != pieceSet.size() + 1; ++kPermute)
  {
    indexPermutations[0] = startFirstIdx;
    int j = 0;
    for (::std::size_t i = 1; i < indexPermutations.size(); ++i)
    {
      if (j == startFirstIdx)
        ++j;
      indexPermutations[i] = j;
      ++j;
    }

    auto startBoard = indexPermutations;

    bool hasNext = false;
    do 
    {
      ++count;
      BoardState<FlattenedSz, NonPlacementDataType> currentBoard;
      currentBoard.m_player = false;

      for (::std::size_t i = 0; i != kPermute; ++i)
        currentBoard.m_board[indexPermutations[i]] = pieceSet[i]; // scatter pieces
      
      assert(partitioner(currentBoard) == proc_id);

      ::std::reverse(indexPermutations.begin() + kPermute, indexPermutations.end());
      hasNext = ::std::next_permutation(indexPermutations.begin(), indexPermutations.end());
    } while (hasNext && partitioner.checkInRange(startBoard, indexPermutations));
  }
  return count;
}

int main()
{
  constexpr std::size_t BoardSz = 64;
  constexpr std::size_t num_processes = 2;
  
  BoardState<BoardSz, null_type> b;
  b.m_board[12] = 'k';
  std::vector<unsigned char> pieceset = { 'k', 'K', 'r' };
  
  KStateSpacePartition<BoardSz, decltype(b)> partitioner('k', num_processes);

  std::size_t combinations = 0;
  for (int i = 0; i < num_processes; ++i)
  {
    combinations += assert_operator_consistency<BoardSz, null_type>(partitioner, pieceset, i);
  }
  auto gold = 249984; // P(64,3)
  assert(combinations == gold);
  std::cout << "test passed" << std::endl;
  return 0;
}
