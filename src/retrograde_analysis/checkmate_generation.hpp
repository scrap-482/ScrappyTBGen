#ifndef CHECKMATE_GENERATION_HPP_ 
#define CHECKMATE_GENERATION_HPP_ 

#include <vector>
#include <type_traits>
#include <unordered_set>
#include <iostream>
#include <algorithm>
#include <numeric>
#include <omp.h>

#include "state.hpp"
#include "permutation_generator.hpp"

// Generate all permutations of up to n-man board positions, enumerating the checkmate states
// Permutations generator from https://stackoverflow.com/questions/28711797/generating-n-choose-k-permutations-in-c
template<::std::size_t FlattenedSz, typename CheckmateEvalFn, class IsValidBoardFn = null_type>
void
parallelCheckmateGenerator(::std::vector<BoardState<FlattenedSz>>& checkmates,
    const ::std::vector<piece_label_t>& pieceSet,
    CheckmateEvalFn checkmateEval,
    IsValidBoardFn boardValidityEval = {})
{
  ::std::array<::std::size_t, FlattenedSz> indexPermutations;
  ::std::iota(indexPermutations.begin(), indexPermutations.end(), 0);
  
  // currently generates checkmates from 2 to n
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
      {
#pragma omp critical 
        {
          checkmates.push_back(currentBoard);
        }
      }
      
      currentBoard.m_player.set();

      if (checkmateEval(currentBoard))
      {
#pragma omp critical
        {
          checkmates.push_back(currentBoard);
        }
      }

      ::std::reverse(indexPermutations.begin() + k_permute, indexPermutations.end());
    } while (::std::next_permutation(indexPermutations.begin(), indexPermutations.end()));
  }
}

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

  ::std::vector<std::vector<piece_label_t>> configsToProcess;
  
  // generate all C(||noKingsPieceset||, remaining_N) combinations. This can probably be done serially
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
      configsToProcess.push_back(tmpPieceset);
      std::cout << configId << std::endl;
      visitedConfigs.insert(configId);
    }

  } while (::std::prev_permutation(bitmask.begin(), bitmask.end()));

  // permute the states in parallel
#pragma omp parallel for 
  for (::std::size_t i = 0; i < configsToProcess.size(); ++i)
  {
    PermutationEvaluator<FlattenedSz, CheckmateEvalFn /* add other function templates*/> evaluator;
    evaluator(checkmates, configsToProcess[i], eval);
  }

  return checkmates;
}

#endif
