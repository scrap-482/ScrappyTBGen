#ifndef RETROGRADE_ANALYSIS_H_
#define RETROGRADE_ANALYSIS_H_

#include <utility>
#include <tuple>
#include <unordered_map>
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

  unordered_map<BoardState<FlattenedSz>, int> position;

  for(int v = 1; v > 0; v++){

	bool updateW = false;
	for(int b = 0; b < wins.size(); b++){ //identify win moves
		::std::vector<BoardState<FlattenedSz>> prev = generatePredecessors(wins[b]);
		for(int i = 0; i < prev.size(); i++){
			if(::std::count(losses.begin(), losses.end(), prev[i]) && position[prev[i]] == (v-1)){
				wins.push_back(wins[b]);
				position[wins[b]] = v;
				updateW = true;
			}
		}
	}
	if(updateW == false){
		return ::std::make_tuple(wins, losses);
	}

	bool updateL = false;
	for(int b = 0; b < losses.size(); b++){ //identify loss moves
		::std::vector<BoardState<FlattenedSz>> prev = generatePredecessors(losses[b]);
		for(int i = 0; i < prev.size(); i++){
			if(::std::count(wins.begin(), wins.end(), prev[i]) && position[losses[b]] <= v){
				losses.push_back(losses[b]);
				position[losses[b]] = v;
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

bool operator==(const BoardState<FlattenedSz>& x, const BoardState<FlattenedSz>& y){
	return x.m_board == y.m_board;
}

#endif
