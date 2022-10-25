#ifndef RETROGRADE_ANALYSIS_H_
#define RETROGRADE_ANALYSIS_H_

#include <utility>
#include <cstdlib>
#include <tuple>
#include <iostream>
#include <unordered_map>
#include <deque>
#include "state_transition.hpp"
#include "checkmate_generation.hpp"

#ifdef MULTI_NODE_
# include "mpi.h"
#endif

enum class MachineType 
{
  SINGLE_NODE,
  MULTI_NODE
};

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

template<::std::size_t FlattenedSz, typename NonPlacementDataType, ::std::size_t N, 
  ::std::size_t rowSz, ::std::size_t colSz, typename CheckmateEvalFn,
  typename MoveGenerator, typename ReverseMoveGenerator, typename HorizontalSymFn=false_fn, typename VerticalSymFn=false_fn, typename IsValidBoardFn=null_type, 
  typename ::std::enable_if<::std::is_base_of<GenerateForwardMoves<FlattenedSz, NonPlacementDataType>, MoveGenerator>::value>::type* = nullptr,
  typename ::std::enable_if<::std::is_base_of<GenerateReverseMoves<FlattenedSz, NonPlacementDataType>, ReverseMoveGenerator>::value>::type* = nullptr>
auto retrogradeAnalysisBaseImpl(const ::std::vector<piece_label_t>& noRoyaltyPieceset, 
    const ::std::vector<piece_label_t>& royaltyPieceset,
    MoveGenerator generateSuccessors,
    ReverseMoveGenerator generatePredecessors,
    CheckmateEvalFn checkmateEval,
    HorizontalSymFn hzSymFn={}, VerticalSymFn vSymFn={}, 
    IsValidBoardFn isValidBoardFn={})
{
  // 1. identify checkmate positions
#if 0
  auto [wins, losses] = generateAllCheckmates<FlattenedSz, NonPlacementDataType, N, rowSz, colSz,
       decltype(checkmateEval)>(noRoyaltyPieceset, royaltyPieceset, checkmateEval);
#else
  // matt's hardcoded validation test
  ::std::unordered_set<BoardState<FlattenedSz, NonPlacementDataType>, BoardStateHasher<FlattenedSz, NonPlacementDataType>> wins;
  ::std::unordered_set<BoardState<FlattenedSz, NonPlacementDataType>, BoardStateHasher<FlattenedSz, NonPlacementDataType>> losses;
  BoardState<FlattenedSz, NonPlacementDataType> b;
  b.m_board[4] = 'R';
  b.m_board[10] = 'K';
  b.m_board[12] = 'k';
  b.m_player = 0;
  losses.insert(b);
#endif

  // TODO: consider more intelligent data structure. Matt thinks heap ordering
  // states in a convenient way for reducing computation.
  ::std::deque<BoardState<FlattenedSz, NonPlacementDataType>> winFrontier;
  ::std::deque<BoardState<FlattenedSz, NonPlacementDataType>> loseFrontier;
  
  // T(p) : BoardState -> int
  ::std::unordered_map<BoardState<FlattenedSz, NonPlacementDataType>, int, 
    BoardStateHasher<FlattenedSz, NonPlacementDataType>> position;

  for (const auto& w : wins)
  {
	  auto preds = generatePredecessors(w);
    winFrontier.insert(::std::end(winFrontier), ::std::begin(preds), ::std::end(preds)); 
	  position[w] = 0;
  }
  
  for (const auto& l : losses)
  {
	  auto preds = generatePredecessors(l);
    loseFrontier.insert(::std::end(loseFrontier), ::std::begin(preds), ::std::end(preds)); 
	  position[l] = 0;
  }
  
  for(int v = 1; v > 0; v++) {
    // 2. Win iteration
	  bool updateW = false;
	  for (::std::size_t i = 0; i < loseFrontier.size(); ++i)
	  {
	  	if (wins.find(loseFrontier[i]) == wins.end()) {
        print_win(loseFrontier[i], v);

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

    // 3. Lose iteration
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
          print_loss(winFrontier[i], v);

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
	  // TODO: maybe just check queue 
	  if(updateL == false) {
	  	return ::std::make_tuple(wins, losses);
	  }
  }	  
  return ::std::make_tuple(wins, losses);
}


template <MachineType t, typename... Args>
auto retrograde_analysis(Args&&... args)
{
  if constexpr (t == MachineType::SINGLE_NODE)
    return retrogradeAnalysisBaseImpl(::std::forward<Args>(args)...);

  else
    return retrograde_analysis_cluster_impl(::std::forward<Args>(args)...);
}

#endif
