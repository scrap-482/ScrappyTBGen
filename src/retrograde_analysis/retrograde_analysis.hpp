#ifndef RETROGRADE_ANALYSIS_H_
#define RETROGRADE_ANALYSIS_H_

#include <utility>
#include <tuple>
#include "state.h"

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

template<::std::size_t FlattenedSz, ::std::size_t N, typename CheckmateEvalFn>
auto retrogradeAnalysisBaseImpl(const ::std::vector<piece_label_t>& fullPieceSet, 
    CheckmateEvalFn checkmateEval)
{
	//todo nick you moron
  ::std::vector<BoardState> wins;
  ::std::vector<BoardState> losses;

  //identify checkmate positions

  for(; ;){

	int v = 1;
	bool updateW = false;
	for(int b = 0; b < wins.size(); b++){ //identify win moves
		::std::vector<BoardState> prev = generatePredecessors(wins[b]);
		for(int i = 0; i < prev.size(); i++){
			if(::std::count(losses.begin(), losses.end(), prev[i])){ //&& move is v-1 
				//find method to check if BoardStates are equivalent (== operator?)
				wins.push_back(prev[i]);
				//
				updateW = true;
			}
		}
	}
	if(updateW == false){
		return ::std::make_tuple(wins, losses);
	}

	bool updateL = false;
	for(int b = 0; b < losses.size(); b++){ //identify loss moves
		::std::vector<BoardState> prev = generatePredecessors(losses[b]);
		for(int i = 0; i < prev.size(); i++){
			if(::std::count(wins.begin(), wins.end(), prev[i])){
				losses.push_back(prev[i]);
				//
				updateL = true;
			}
		}
	}
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

bool operator==(const BoardState& x, const BoardState& y){
	return x.m_board == y.m_board;
}

#endif
