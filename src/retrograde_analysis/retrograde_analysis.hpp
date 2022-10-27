/*
 * The jist of the retrograde analysis implementation exists within this file. The implementation is
 * heavily templated to promote aggressive optimization of certain compile time data (ex: board row and column size) 
 * along with checking that functors implement the appropriate virtual parent functors. 
 * This has yielded significant measurable benefit in terms of performance thus far.
 */
#ifndef RETROGRADE_ANALYSIS_H_
#define RETROGRADE_ANALYSIS_H_

#include <utility>
#include <cstdlib>
#include <tuple>
#include <unordered_map>
#include <deque>

#include "state_transition.hpp"
#include "checkmate_generation.hpp"

#ifdef TRACK_RETROGRADE_ANALYSIS
# include <iostream>
#endif

#ifdef MULTI_NODE_
# include <mpi.h>
#endif

// used to deduce implementation to invoke at compile time
enum class MachineType 
{
  SINGLE_NODE,
  MULTI_NODE
};

// helper function when tracking board win states
void print_win(auto w, int v)
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
    if (count % 4 == 0)
      std::cout << std::endl;
  }
}

// helper function when tracking board loss states
void print_loss(auto l, int v)
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

// TODO: MPI implementation
auto retrogradeAnalysisClusterImpl();

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
  
  // TODO: consider more intelligent data structure. Matt thinks heap ordering
  // states in a convenient way for reducing computation.
  using frontier_t = ::std::deque<BoardState<FlattenedSz, NonPlacementDataType>>;
  using board_map_t = ::std::unordered_map<BoardState<FlattenedSz, NonPlacementDataType>, int, 
    BoardStateHasher<FlattenedSz, NonPlacementDataType>>;

  // the commented code below is currently validation by manually entering checkmate states. this will be removed once rulesets
  // are fully implemented and can be ignored for now.
#if 1
  // 1. identify checkmate positions
  board_set_t wins;
  board_set_t losses = ::std::move(checkmates);
#else
  // matt's hardcoded validation test
  board_set_t wins;
  board_set_t losses;
  BoardState<FlattenedSz, NonPlacementDataType> b;
  b.m_board[4] = 'R';
  b.m_board[10] = 'K';
  b.m_board[12] = 'k';
  b.m_player = 0;
  losses.insert(b);
#endif
  frontier_t winFrontier;
  frontier_t loseFrontier;
  
  // T(p) : BoardState -> int
  board_map_t position;
  for (const auto& l : losses)
  {
	  auto preds = generatePredecessors(l);
    loseFrontier.insert(::std::end(loseFrontier), ::std::begin(preds), ::std::end(preds)); 
	  position[l] = 0;
  }
  
  for(int v = 1; v > 0; v++) {
    // 2. Win iteration - add immediate wins (at least one successor is a loss for the opposing player) to the win set
	  bool updateW = false;
	  for (::std::size_t i = 0; i < loseFrontier.size(); ++i)
	  {
	  	if (wins.find(loseFrontier[i]) == wins.end()) {
#ifdef TRACK_RETROGRADE_ANALYSIS
        print_win(loseFrontier[i], v);
#endif
	  		wins.insert(loseFrontier[i]);
	  		position[loseFrontier[i]] = v;
	  		updateW = true;
	  		auto preds = generatePredecessors(loseFrontier[i]);

	  		for (const auto& prev : preds)
	  		{
	  			if (wins.find(prev) == wins.end() /*&& loseFrontier.find(prev) == losses.end()*/) // TODO check
	  			{
	  				winFrontier.push_back(preds[i]);
	  			}
	  		}
	  	}
	  	loseFrontier.erase(loseFrontier.begin() + i);
	  	--i;
	  }

    // 3. Lose iteration - add immediate losses (all successors are win for opponent) to the lose set
	  if(updateW == false){
	  	return ::std::make_tuple(wins, losses);
	  }

	  bool updateL = false;
	  for (::std::size_t i = 0; i < winFrontier.size(); ++i)
	  {
	  	if (wins.find(winFrontier[i]) == wins.end() && losses.find(winFrontier[i]) == losses.end())
	  	{
	  		// TODO: Think about better optimization. maybe use ideas from the parallel algorithm or
	  		// more intelligent checking.
	  		auto succs = generateSuccessors(winFrontier[i]);
	  		bool allWins = true;
	  		for (const auto& succ : succs)
	  		{
	  			if (wins.find(succ) == wins.end())
	  			{
	  				allWins = false;
	  			}
	  			if (losses.find(succ) != losses.end())
	  			{
	  				loseFrontier.push_back(winFrontier[i]);
	  				winFrontier.erase(winFrontier.begin() + i);
	  				--i;
	  			}
	  		}
	  		if (allWins)
	  		{
#ifdef TRACK_RETROGRADE_ANALYSIS
          print_loss(winFrontier[i], v);
#endif

	  			losses.insert(winFrontier[i]);
				  position[winFrontier[i]] = v;
	  			updateL = true;
	  			auto preds = generatePredecessors(winFrontier[i]);

	  			for (const auto& prev : preds)
	  			{
	  				if (wins.find(prev) == wins.end() /*&& loseFrontier.find(prev) == losses.end()*/) // TODO: reason about
	  				{
	  					loseFrontier.push_back(preds[i]);
	  				}
	  			}
	  			
	  			winFrontier.erase(winFrontier.begin() + i);
	  			--i;
	  		}
	  	}
	  	else 
	  	{
	  		winFrontier.erase(winFrontier.begin() + i);
	  		--i;
	  	}
	  }
	  if(updateL == false) {
	  	return ::std::make_tuple(wins, losses);
	  }
  }
  return ::std::make_tuple(wins, losses);
}

// chooses correct implementation at compile time 
template <MachineType t, typename... Args>
auto retrograde_analysis(Args&&... args)
{
  if constexpr (t == MachineType::SINGLE_NODE)
    return retrogradeAnalysisBaseImpl(::std::forward<Args>(args)...);

  else
    return retrograde_analysis_cluster_impl(::std::forward<Args>(args)...);
}

#endif
