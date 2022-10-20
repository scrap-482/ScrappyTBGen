#ifndef RETROGRADE_ANALYSIS_H_
#define RETROGRADE_ANALYSIS_H_

#include <utility>
#include <tuple>
#include <unordered_map>
#include <deque>
#include "state_transition.hpp"

#ifdef MULTI_NODE_
# include "mpi.h"
#endif

enum class MachineType 
{
  SINGLE_NODE,
  MULTI_NODE
};

// TODO: MPI implementation
auto retrogradeAnalysisClusterImpl();

template<::std::size_t FlattenedSz, typename NonPlacementDataType, ::std::size_t N, typename CheckmateEvalFn,
  typename MoveGenerator, typename ReverseMoveGenerator,
  typename ::std::enable_if<::std::is_base_of<GenerateForwardMoves<FlattenedSz>, MoveGenerator>::value>::type* = nullptr,
  typename ::std::enable_if<::std::is_base_of<GenerateReverseMoves<FlattenedSz>, ReverseMoveGenerator>::value>::type* = nullptr>
auto retrogradeAnalysisBaseImpl(const ::std::vector<piece_label_t>& fullPieceSet, 
    const& MoveGenerator generateSuccessors,
    const& ReverseMoveGenerator generatePredecessors,
    CheckmateEvalFn checkmateEval)
{
	//todo nick you moron
   ::std::vector<BoardState<FlattenedSz, NonPlacementDataType>> wins;
   ::std::vector<BoardState<FlattenedSz, NonPlacementDataType>> losses;

  //identify checkmate positions
  // TODO: Matt fix
  //auto [wins, losses] = generateAllCheckmates(fullPieceSet, checkmateEval);

  // TODO: consider more intelligent data structure. Matt thinks heap ordering
  // states in a convenient way for reducing computation.
  ::std::deque<BoardState<FlattenedSz, NonPlacementDataType>> winFrontier;
  ::std::deque<BoardState<FlattenedSz, NonPlacementDataType>> loseFrontier;
  
  // T(p) : BoardState -> int
  ::std::unordered_map<BoardState<FlattenedSz, NonPlacementDataType>, int> position;
  for (const auto& w : wins)
  {
	auto preds = generatePredecessors(w);
	// TODO: need to actually append
	// winFrontier.push_back(preds);
	position[w] = 0;
  }
  
  for (const auto& l : losses)
  {
	auto preds = generatePredecessors(l);
	// TODO: needs to be append
	// loseFrontier.push_back(preds);
	position[l] = 0;
  }

  for(int v = 1; v > 0; v++){
	bool updateW = false;
	for (::std::size_t i = 0; i < loseFrontier.size(); ++i) // win iteration
	{
		if (wins.find(loseFrontier[i]) != wins.end()) {
			wins.insert(loseFrontier[i]);
			position[loseFrontier[i]] = v;
			updateW = true;
			auto preds = generatePredecessors(loseFrontier[i]);

			for (const auto& prev : preds)
			{
				if (wins.find(prev) == wins.end() && loseFrontier.find(prev) == losses.end())
				{
					winFrontier.push_back(preds[i]);
				}
			}
		}
		loseFrontier.erase(loseFrontier.begin() + i);
		--i;
	}
	if(updateW == false){
		return ::std::make_tuple(wins, losses);
	}

	bool updateL = false;
	for (::std::size_t i = 0; i < winFrontier.size(); ++i)
	{
		if (wins.find(winFrontier[i]) != wins.end() && losses.find(winFrontier[i]) == losses.end())
		{
			// TODO: Think if more optimized. maybe use ideas from the parallel algorithm or
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
					winFrontier.erase(winFrontier.begin() + i)
					--i;
				}
			}
			if (allWins)
			{
				losses.insert(winFrontier[i]);
				updateL = true;
				auto preds = generatePredecessors(winsFrontier[i]);

				for (const auto& prev : preds)
				{
					if (wins.find(prev) == wins.end() && loseFrontier.find(prev) == losses.end())
					{
						loseFrontier.push_back(preds[i]);
					}
				}
				
				winFrontier.erase(winFrontier.begin() + i)
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
	if(updateL == false){
		return ::std::make_tuple(wins, losses);
	}
  }	  
  // TODO
  return ::std::make_tuple(wins, losses);
}


template <MachineType t, typename... Args>
auto retrograde_analysis(Args&&... args)
{
  if constexpr (t == MachineType::SINGLE_NODE)
    return retrograde_analysis_base_impl(::std::forward<Args>(args)...);

  else
    return retrograde_analysis_cluster_impl(::std::forward<Args>(args)...);
}

template<::std::size_t FlattenedSz, typename NonPlacementDataType>
bool operator==(const BoardState<FlattenedSz, NonPlacementDataType>& x, const BoardState<FlattenedSz, NonPlacementDataType>& y){
	return x.m_board == y.m_board;
}

#endif
