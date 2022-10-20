#ifndef CHECKMATE_GENERATION_HPP_ 
#define CHECKMATE_GENERATION_HPP_ 

#include <unordered_set>

#include "state_transition.hpp"
#include "permutation_generator.hpp"

// generate all checkmates for given game
template<::std::size_t FlattenedSz, typename NonPlacementDataType, ::std::size_t N, ::std::size_t rowSz, ::std::size_t colSz, typename CheckmateEvalFn,
  typename HorizontalSymFn = false_fn, typename VerticalSymFn = false_fn, typename IsValidBoardFn = null_type,
  typename ::std::enable_if<::std::is_base_of<CheckmateEvaluator<FlattenedSz, NonPlacementDataType>, CheckmateEvalFn>::value>::type* = nullptr>
auto generateAllCheckmates(const ::std::vector<piece_label_t>& noRoyaltyPieceset, 
    const ::std::vector<piece_label_t>& royaltyPieceset, CheckmateEvalFn eval, 
    HorizontalSymFn hzSymFn={}, VerticalSymFn vSymFn={}, 
    IsValidBoardFn isValidBoardFn={})
{
  ::std::vector<BoardState<FlattenedSz, NonPlacementDataType>> whiteWins;
  ::std::vector<BoardState<FlattenedSz, NonPlacementDataType>> whiteLosses;
  constexpr auto remaining_N = N - 2; // assumed that 2 royalty must be present 
  
  // https://rosettacode.org/wiki/Combinations#C.2B.2B
  ::std::string bitmask(remaining_N, 1); // K leading 1's
  ::std::string configId(remaining_N, ' ');
  ::std::string altConfigId(remaining_N, ' ');
  ::std::unordered_set<::std::string> visitedConfigs;
  bitmask.resize(noRoyaltyPieceset.size(), 0); // N-K trailing 0's

  ::std::vector<std::vector<piece_label_t>> configsToProcess;
  
  // generate all C(||noKingsPieceset||, remaining_N) combinations. This can be done serially since 
  // the number of combinations is always small. 
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
    
    // check for color symmetry
    if (visitedConfigs.find(configId) == visitedConfigs.end() 
        && visitedConfigs.find(altConfigId) == visitedConfigs.end())
    {
      configsToProcess.push_back(tmpPieceset);
      visitedConfigs.insert(configId);
    }

  } while (::std::prev_permutation(bitmask.begin(), bitmask.end()));
  
  // Generate state permutations parallelizing over the board configurations
#pragma omp parallel for 
  for (::std::size_t i = 0; i < configsToProcess.size(); ++i)
  {
    PermutationEvaluator<FlattenedSz, NonPlacementDataType, rowSz, colSz, CheckmateEvalFn, 
      HorizontalSymFn, VerticalSymFn, IsValidBoardFn> evaluator(hzSymFn, vSymFn, isValidBoardFn);
    evaluator(whiteWins, whiteLosses, configsToProcess[i], eval);
  }

  return ::std::make_tuple(whiteWins, whiteLosses);;
}

#endif
