/*
* Copyright 2022 SCRAP
*
* This file is part of Scrappy Tablebase Generator.
* 
* Scrappy Tablebase Generator is free software: you can redistribute it and/or modify it under the terms 
* of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, 
* or (at your option) any later version.
* 
* Scrappy Tablebase Generator is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of 
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along with Scrappy Tablebase Generator. If not, see <https://www.gnu.org/licenses/>.
*/

/*
 * The checkmate generation header implementations are responsible for invoking the appropriate board permuters
 * and to identify all checkmate states with the 
 */


#ifndef CHECKMATE_GENERATION_HPP_ 
#define CHECKMATE_GENERATION_HPP_ 

#include <unordered_set>
#include <iostream>
#include "state_transition.hpp"
#include "permutation_generator.hpp"

// generate all N-man piece configurations for a given game 
template<::std::size_t FlattenedSz, typename NonPlacementDataType, ::std::size_t N,
  ::std::size_t rowSz, ::std::size_t colSz, typename CheckmateEvalFn,
  typename HorizontalSymFn = false_fn, typename VerticalSymFn = false_fn, typename IsValidBoardFn = null_type,
  typename ::std::enable_if<::std::is_base_of<CheckmateEvaluator<FlattenedSz, NonPlacementDataType>, CheckmateEvalFn>::value>::type* = nullptr>
auto generateAllConfigs(const ::std::vector<piece_label_t>& noRoyaltyPieceset, 
    const ::std::vector<piece_label_t>& royaltyPieceset, CheckmateEvalFn eval, 
    HorizontalSymFn hzSymFn={}, VerticalSymFn vSymFn={}, 
    IsValidBoardFn isValidBoardFn={})
{
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
  return configsToProcess;
}

// generate all checkmates of all piece configurations for given game
template<::std::size_t FlattenedSz, typename NonPlacementDataType, ::std::size_t N, 
  ::std::size_t rowSz, ::std::size_t colSz, typename CheckmateEvalFn,
  typename HorizontalSymFn = false_fn, typename VerticalSymFn = false_fn, typename IsValidBoardFn = null_type,
  typename ::std::enable_if<::std::is_base_of<CheckmateEvaluator<FlattenedSz, NonPlacementDataType>, CheckmateEvalFn>::value>::type* = nullptr>
auto generateAllCheckmates(const ::std::vector<piece_label_t>& noRoyaltyPieceset, 
    const ::std::vector<piece_label_t>& royaltyPieceset, CheckmateEvalFn eval, 
    HorizontalSymFn hzSymFn={}, VerticalSymFn vSymFn={}, 
    IsValidBoardFn isValidBoardFn={})
{
  using board_set_t = ::std::unordered_set<BoardState<FlattenedSz, NonPlacementDataType>, 
        BoardStateHasher<FlattenedSz, NonPlacementDataType>>;
  ::std::vector<::std::tuple<::std::vector<piece_label_t>, board_set_t>> checkmates;

  // Generate state permutations parallelizing over the board configurations
  auto configsToProcess = 
    generateAllConfigs<FlattenedSz, NonPlacementDataType, N, rowSz, colSz, CheckmateEvalFn, HorizontalSymFn, VerticalSymFn, IsValidBoardFn>
    (noRoyaltyPieceset, royaltyPieceset, eval, hzSymFn, vSymFn, isValidBoardFn);
  
  for (const auto& c : configsToProcess)
    checkmates.emplace_back(c, board_set_t{});

#pragma omp parallel for 
  for (::std::size_t i = 0; i < configsToProcess.size(); ++i)
  {
    board_set_t losses;
    PermutationEvaluator<FlattenedSz, NonPlacementDataType, rowSz, colSz, CheckmateEvalFn, 
      HorizontalSymFn, VerticalSymFn, IsValidBoardFn> evaluator(hzSymFn, vSymFn, isValidBoardFn);
    evaluator(::std::get<1>(checkmates[i]), configsToProcess[i], eval);
  }
  return checkmates;
}

// for a given pieceset configuration, identify all checkmate states of its valid permutations
template<::std::size_t FlattenedSz, typename NonPlacementDataType, ::std::size_t N, 
  ::std::size_t rowSz, ::std::size_t colSz, typename CheckmateEvalFn,
  typename HorizontalSymFn = false_fn, typename VerticalSymFn = false_fn, typename IsValidBoardFn = null_type,
  typename ::std::enable_if<::std::is_base_of<CheckmateEvaluator<FlattenedSz, NonPlacementDataType>, CheckmateEvalFn>::value>::type* = nullptr>
auto generateConfigCheckmates(const ::std::vector<piece_label_t>& pieceSet, 
    CheckmateEvalFn eval, 
    IsValidBoardFn isValidBoardFn={},
    HorizontalSymFn hzSymFn={}, VerticalSymFn vSymFn={}) 
{
  ::std::unordered_set<BoardState<FlattenedSz, NonPlacementDataType>, BoardStateHasher<FlattenedSz, NonPlacementDataType>> losses;
  
  PermutationEvaluator<FlattenedSz, NonPlacementDataType, rowSz, colSz, CheckmateEvalFn, 
    HorizontalSymFn, VerticalSymFn, IsValidBoardFn> evaluator(hzSymFn, vSymFn, isValidBoardFn);
  evaluator(losses, pieceSet, eval, isValidBoardFn);
  return losses;
}

// a parallelized search of all permutations in the game. Parallelization occurs over the permutations themselves
// based upon lexicographical ordering.
template<::std::size_t FlattenedSz, typename NonPlacementDataType, typename EvalFn,
  typename IsValidBoardFn=null_type>
auto inline generatePartitionCheckmates(int k, const KStateSpacePartition<FlattenedSz, BoardState<FlattenedSz, NonPlacementDataType>>& partitioner,
    ::std::unordered_set<BoardState<FlattenedSz, NonPlacementDataType>, BoardStateHasher<FlattenedSz, NonPlacementDataType>>&& losses,
    const ::std::vector<piece_label_t>& pieceSet,
    EvalFn checkmateEval,
    IsValidBoardFn boardValidityEval = {})
{
  ::std::array<::std::size_t, FlattenedSz> indexPermutations;
  auto [startFirstIdx, endFirstIdx] = partitioner.getRange(k);
   
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

    // k-permutation iteration inspired by answer from Vaughn Cato last updated on May 23, 2017
    // answer: https://stackoverflow.com/a/28712605
    // author: https://stackoverflow.com/users/951890/vaughn-cato
    bool hasNext = false;
    do 
    {
      BoardState<FlattenedSz, NonPlacementDataType> currentBoard;
      currentBoard.m_player = false;

      for (::std::size_t i = 0; i != kPermute; ++i)
        currentBoard.m_board[indexPermutations[i]] = pieceSet[i]; // scatter pieces

      if constexpr (!::std::is_same<null_type, IsValidBoardFn>::value)
      {
        if (!boardValidityEval(currentBoard))
        {
          ::std::reverse(indexPermutations.begin() + kPermute, indexPermutations.end());
          hasNext = ::std::next_permutation(indexPermutations.begin(), indexPermutations.end());
          continue;
        }
      }
      
      // checking if black loses (white wins) 
      if (checkmateEval(currentBoard))
        losses.insert(currentBoard);
      
      currentBoard.m_player = true;
      
      // checking if white loses (black wins)
      if (checkmateEval(currentBoard))
      {
        losses.insert(currentBoard);
      }

      ::std::reverse(indexPermutations.begin() + kPermute, indexPermutations.end());
      hasNext = ::std::next_permutation(indexPermutations.begin(), indexPermutations.end());
    } while (hasNext && partitioner.checkInRange(startBoard, indexPermutations));
  }
  return ::std::move(losses);
}

// identifies all checkmates across a given configuration with OpenMP parallelism
template<::std::size_t FlattenedSz, typename NonPlacementDataType, ::std::size_t N, 
  ::std::size_t rowSz, ::std::size_t colSz, typename CheckmateEvalFn,
  typename HorizontalSymFn = false_fn, typename VerticalSymFn = false_fn, typename IsValidBoardFn = null_type,
  typename ::std::enable_if<::std::is_base_of<CheckmateEvaluator<FlattenedSz, NonPlacementDataType>, CheckmateEvalFn>::value>::type* = nullptr>
auto generateParallelConfigCheckmates(const ::std::vector<piece_label_t>& pieceSet, 
    CheckmateEvalFn eval, 
    IsValidBoardFn isValidBoardFn={},
    HorizontalSymFn hzSymFn={}, VerticalSymFn vSymFn={}) 
{
  ::std::unordered_set<BoardState<FlattenedSz, NonPlacementDataType>, BoardStateHasher<FlattenedSz, NonPlacementDataType>> losses;
#pragma omp parallel
    {
      decltype(losses) localLosses;

      int totalThreads = omp_get_num_threads();
      int threadId = omp_get_thread_num();
      
      KStateSpacePartition<FlattenedSz, BoardState<FlattenedSz, NonPlacementDataType>> partitioner(pieceSet[0], totalThreads);
      localLosses = generatePartitionCheckmates<FlattenedSz, NonPlacementDataType>(threadId, partitioner, 
          std::move(localLosses), pieceSet, eval, isValidBoardFn);
#pragma omp critical
      {
        for (const auto& l : localLosses)
          losses.insert(l);
      }
    }
  return losses;
}

#endif
