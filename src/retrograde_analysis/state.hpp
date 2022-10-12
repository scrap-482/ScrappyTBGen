#ifndef STATE_HPP_
#define STATE_HPP_

#include <array>
#include <vector>
#include <numeric>
#include <algorithm>
#include <iostream>

using piece_label_t = unsigned char;

#if 1
template<::std::size_t FlattenedSz>
struct BoardState
{
  ::std::array<piece_label_t, FlattenedSz> m_board;
};
#else // TODO: Potential future bitboard optimization 
template<::std::size_t FlattenedSz, ::std::size_t NumUniquePieces>
struct bb_state
{
  using bitboard_t = ::std::bitset<FlattenedSz>;

  ::std::array<bitboard_t, NumUniquePieces> m_piece_bb;

  bitboard_t m_white_bit_board;
  bitboard_t m_black_bit_board;
};
#endif

// TODO:
// it is necessary to make copies. all retrograde analysis
// implementations do this because the algorithm is not a tree 
// meaning that multiple predecessors may exist. it is necessary to store each state

template<::std::size_t FlattenedSz>
auto
generatePredecessors(const BoardState<FlattenedSz>& board);

// TODO:
// Games like chess have unreachable positions.
// we can have an is_valid_check position potentially. Not immediately pressing.

// Generate all permutations of up to n-man board positions, enumerating the checkmate states
// Permutations generator from https://stackoverflow.com/questions/28711797/generating-n-choose-k-permutations-in-c
template<typename CheckmateEvalFn, ::std::size_t FlattenedSz>
std::vector<BoardState<FlattenedSz>>
generateAllCheckmates(const std::vector<piece_label_t>& pieceSet,
    CheckmateEvalFn checkmateEval)
{
  std::vector<BoardState<FlattenedSz>> checkmates;
  std::array<std::size_t, FlattenedSz> indexPermutations;
  std::iota(indexPermutations.begin(), indexPermutations.end(), 0);
  
  for (auto k_permute = 2; k_permute != pieceSet.size()+1; ++k_permute)
  {
    do 
    {
      BoardState<FlattenedSz> currentBoard;
      for (auto i = 0; i != k_permute; ++i)
        currentBoard.m_board[indexPermutations[i]] = pieceSet[i]; // scatter pieces

      if (checkmateEval(currentBoard))
        checkmates.push_back(currentBoard);

      std::reverse(indexPermutations.begin() + k_permute, indexPermutations.end());
    } while (std::next_permutation(indexPermutations.begin(), indexPermutations.end()));
  }

  return checkmates;
}

#endif
