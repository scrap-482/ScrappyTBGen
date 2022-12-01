#ifndef SINGLE_NODE_IMPL_HPP_
#define SINGLE_NODE_IMPL_HPP_

#include <iostream>
#include <utility>
#include <cstdlib>
#include <cassert> 
#include <tuple>
#include <unordered_map>
#include <deque>

#include "state_transition.hpp"
#include "checkmate_generation.hpp"

#ifdef TRACK_RETROGRADE_ANALYSIS
// helper function when tracking board win states
void print_win(auto w, int v)
{
#pragma omp critical
  {
    std::cout << "found win at v=" << v << std::endl;
    std::cout << "Player: " << w.m_player << std::endl;
    auto b = w.m_board;
    
    int count = 0;
    for (auto c : b)
    {
      if (c == '\0')
        std::cout << "x ";
      else
        std::cout << c << " ";
      ++count;
      if (count % 8 == 0)
        std::cout << std::endl;
    }
  }
}

// helper function when tracking board loss states
void print_loss(auto l, int v)
{
#pragma omp critical
  {
    std::cout << "found loss at v=" << v << std::endl;
    std::cout << "Player: " << l.m_player << std::endl;
    
    auto b = l.m_board;
    
    int count = 0;
    for (auto c : b)
    {
      if (c == '\0')
        std::cout << "x ";
      else
        std::cout << c << " ";
      ++count;
      if (count % 4 == 0)
        std::cout << std::endl;
    }
  }
}

#endif

// This function is the base implementation for the single-node implementation
template<::std::size_t FlattenedSz, typename NonPlacementDataType, ::std::size_t N, 
  ::std::size_t rowSz, ::std::size_t colSz,
  typename MoveGenerator, typename ReverseMoveGenerator, typename HorizontalSymFn=false_fn, 
  typename VerticalSymFn=false_fn, typename IsValidBoardFn=null_type, 
  typename ::std::enable_if<::std::is_base_of<GenerateForwardMoves<FlattenedSz, NonPlacementDataType>, 
    MoveGenerator>::value>::type* = nullptr,
  typename ::std::enable_if<::std::is_base_of<GenerateReverseMoves<FlattenedSz, NonPlacementDataType>, 
    ReverseMoveGenerator>::value>::type* = nullptr>
auto retrogradeAnalysisBaseImpl(::std::unordered_set<BoardState<FlattenedSz, NonPlacementDataType>, BoardStateHasher<FlattenedSz, NonPlacementDataType>> checkmates,
    MoveGenerator generateSuccessors,
    ReverseMoveGenerator generatePredecessors,
    HorizontalSymFn hzSymFn={}, VerticalSymFn vSymFn={}, 
    IsValidBoardFn isValidBoardFn={})
{
  using board_set_t = ::std::unordered_set<BoardState<FlattenedSz, NonPlacementDataType>, 
        BoardStateHasher<FlattenedSz, NonPlacementDataType>>;
  
  using local_frontier_t = ::std::vector<BoardState<FlattenedSz, NonPlacementDataType>>;
  using frontier_t = ::std::unordered_set<BoardState<FlattenedSz, NonPlacementDataType>, 
    BoardStateHasher<FlattenedSz, NonPlacementDataType>>;
  using board_map_t = ::std::unordered_map<BoardState<FlattenedSz, NonPlacementDataType>, int, 
    BoardStateHasher<FlattenedSz, NonPlacementDataType>>;

  auto numThreads = omp_get_num_threads();

  // 1. identify checkmate positions 
  board_set_t wins;
  board_set_t losses = ::std::move(checkmates);
  
  frontier_t winFrontier;
  frontier_t loseFrontier;
  
  // T(p) : BoardState -> int
  board_map_t depthToMate;
  for (const auto& l : losses)
  {
    auto preds = generatePredecessors(l);
    loseFrontier.insert(::std::begin(preds), ::std::end(preds)); 
    depthToMate[l] = 0;
  }
  
  for(int v = 1; v > 0; v++) {
    // 2. Win iteration - add immediate wins (at least one successor is a loss for the opposing player) to the win set
    bool updateW = false;
#pragma omp parallel
    {
      local_frontier_t localWins;
      localWins.reserve(loseFrontier.size() / numThreads);
      local_frontier_t localPreds;
      localPreds.reserve(loseFrontier.size());

      // parallel for loop to collect in local buckets 
      // identify win states and generate predecessors.
      // Parallelizing over std::unordered_set - https://stackoverflow.com/questions/26034642/openmp-gnu-parallel-for-an-unordered-map
#pragma omp for nowait
      for (::std::size_t i = 0; i < loseFrontier.bucket_count(); ++i)
      for (auto bState = loseFrontier.begin(i); bState != loseFrontier.end(i); ++bState)
      {
#ifdef TRACK_RETROGRADE_ANALYSIS
        print_win(bState, v);
#endif
        localWins.push_back(*bState);

        if (!updateW)
        {
#pragma omp critical
          {
            updateW = true;
          }
        }
        auto preds = generatePredecessors(*bState);
        localPreds.insert(::std::end(localPreds), ::std::begin(preds), ::std::end(preds));
      }
    // critical section - each thread adds to win buffer
#pragma omp critical
      {
        for (const auto& prev : localPreds)
        {
          if (wins.find(prev) == wins.end() && losses.find(prev) == losses.end())
            winFrontier.insert(prev);
        }
        for (const auto& localWin : localWins)
        {
          wins.insert(localWin);
          depthToMate[localWin] = v;
        }
      }
    }

    loseFrontier.clear(); 

    if(updateW == false){
      return ::std::make_tuple(wins, losses, depthToMate);
    }
    bool updateL = false;
    // 3. Lose iteration - add immediate losses (all successors are win for opponent) to the lose set
#pragma omp parallel
    {
      local_frontier_t localLosses;
      localLosses.reserve(winFrontier.size() / numThreads);
      local_frontier_t localPreds;
      localPreds.reserve(winFrontier.size());
            
#pragma omp for nowait
      for (::std::size_t i = 0; i < winFrontier.bucket_count(); ++i)
      for (auto bState = winFrontier.begin(i); bState != winFrontier.end(i); ++bState)
      {
        // omp start parallel section
        auto succs = generateSuccessors(*bState);
        bool allWins = true;
        for (const auto& succ : succs)
        {
          if (wins.find(succ) == wins.end())
          {
            allWins = false;
            break;
          }
        }
        if (allWins)
        {
#ifdef TRACK_RETROGRADE_ANALYSIS
          print_loss(bState, v);
#endif
          localLosses.push_back(*bState);
          auto preds = generatePredecessors(*bState);
          localPreds.insert(::std::end(localPreds), ::std::begin(preds), ::std::end(preds));
        }
      }
#pragma omp critical
      {
        for (const auto& prev : localPreds)
        {
          if (wins.find(prev) == wins.end() && losses.find(prev) == losses.end())
            loseFrontier.insert(prev);
        }
        for (const auto& localLoss : localLosses)
        {
          if (!updateL)
            updateL = true;
          losses.insert(localLoss);
          depthToMate[localLoss] = v;
        }
      }
    }
    // clear the whole win frontier
    winFrontier.clear();
    std::cout << "done with v=" << v << " " << loseFrontier.size() << std::endl;
    if(updateL == false) {
      return ::std::make_tuple(wins, losses, depthToMate);
    }
  }
  return ::std::make_tuple(wins, losses, depthToMate);
}

#endif
