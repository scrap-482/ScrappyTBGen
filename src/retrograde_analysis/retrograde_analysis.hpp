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
  ::std::vector<BoardState> wins;
  ::std::vector<BoardState> losses;
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

#endif
