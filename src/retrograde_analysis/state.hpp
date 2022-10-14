#ifndef STATE_HPP_
#define STATE_HPP_

#include <array>
#include <vector>
#include <numeric>
#include <algorithm>
#include <iostream>
#include <type_traits>
#include <unordered_set>

using piece_label_t = unsigned char;

// fallback for optional function templating 
struct null_type {};

#if 1
template<::std::size_t FlattenedSz>
struct BoardState
{
  // 1 for white move. 0 for black move
  //::std::bitset<1> m_player;
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

// "unmove" operations matt todo
template<::std::size_t FlattenedSz>
auto
generatePredecessors(const BoardState<FlattenedSz>& board);

// Generate all permutations of up to n-man board positions, enumerating the checkmate states
// Permutations generator from https://stackoverflow.com/questions/28711797/generating-n-choose-k-permutations-in-c
template<::std::size_t FlattenedSz, typename CheckmateEvalFn, class IsValidBoardFn = null_type>
::std::vector<BoardState<FlattenedSz>>
generateCheckmatePermutations(::std::vector<BoardState<FlattenedSz>> checkmates,
    const ::std::vector<piece_label_t>& pieceSet,
    CheckmateEvalFn checkmateEval,
    IsValidBoardFn boardValidityEval = {})
{
  ::std::array<::std::size_t, FlattenedSz> indexPermutations;
  ::std::iota(indexPermutations.begin(), indexPermutations.end(), 0);
  
  for (::std::size_t k_permute = 2; k_permute != pieceSet.size() + 1; ++k_permute)
  {
    do 
    {
      BoardState<FlattenedSz> currentBoard;
      for (::std::size_t i = 0; i != k_permute; ++i)
        currentBoard.m_board[indexPermutations[i]] = pieceSet[i]; // scatter pieces

      if constexpr (!::std::is_same<null_type, IsValidBoardFn>::value)
        if (!IsValidBoardFn(currentBoard))
          continue;

      if (checkmateEval(currentBoard))
        ; //checkmates.push_back(currentBoard);

      ::std::reverse(indexPermutations.begin() + k_permute, indexPermutations.end());
    } while (::std::next_permutation(indexPermutations.begin(), indexPermutations.end()));
  }

  return ::std::move(checkmates);
}

// todo: analyze performance tomorrow.
template<::std::size_t FlattenedSz, ::std::size_t N, typename CheckmateEvalFn> // need to add null type
auto generateAllCheckmates(const ::std::vector<piece_label_t>& noRoyaltyPieceset, 
    const ::std::vector<piece_label_t>& royaltyPieceset, CheckmateEvalFn eval)
{
  ::std::vector<BoardState<FlattenedSz>> checkmates;
  constexpr auto remaining_N = N - 2; // 2 kings must be present 
  
  // https://rosettacode.org/wiki/Combinations#C.2B.2B
  ::std::string bitmask(remaining_N, 1); // K leading 1's
  ::std::string configId(remaining_N, ' ');
  ::std::string altConfigId(remaining_N, ' ');
  ::std::unordered_set<::std::string> visitedConfigs;
  bitmask.resize(noRoyaltyPieceset.size(), 0); // N-K trailing 0's
  
  // generate all C(||noKingsPieceset||, remaining_N) combinations
  do {
    std::vector<piece_label_t> tmpPieceset = royaltyPieceset;
    tmpPieceset.resize(N);
    
    ::std::size_t idx = 2;
    for (::std::size_t i = 0; i < bitmask.size(); ++i)
    {
      if (bitmask[i])
      {
        tmpPieceset[idx] = noRoyaltyPieceset[i];
        configId[idx++ - 2] = noRoyaltyPieceset[i]; 
      }
    }
    
    for (::std::size_t i = 0; i < configId.size(); ++i)
      altConfigId[i] = ::std::isupper(configId[i]) ? ::std::tolower(configId[i]) : ::std::toupper(configId[i]);

    ::std::sort(configId.begin(), configId.end());
    ::std::sort(altConfigId.begin(), altConfigId.end());

    if (visitedConfigs.find(configId) == visitedConfigs.end() 
        && visitedConfigs.find(altConfigId) == visitedConfigs.end())
    {
      checkmates = generateCheckmatePermutations<FlattenedSz>(::std::move(checkmates), tmpPieceset, eval);
      visitedConfigs.insert(configId);
    }

  } while (::std::prev_permutation(bitmask.begin(), bitmask.end()));
  
  return checkmates;
}

#endif
