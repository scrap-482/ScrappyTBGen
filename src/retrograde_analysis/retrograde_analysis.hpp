/*
 * The jist of the retrograde analysis implementation exists within this file. The implementation is
 * heavily templated to promote aggressive optimization of certain compile time data (ex: board row and column size) 
 * along with checking that functors implement the appropriate virtual parent functors. 
 * This has yielded significant measurable benefit in terms of performance thus far.
 */
#ifndef RETROGRADE_ANALYSIS_H_
#define RETROGRADE_ANALYSIS_H_

#ifdef TRACK_RETROGRADE_ANALYSIS
# include <iostream>
#endif

#ifdef MULTI_NODE
# include "multi_node_impl.hpp"
#else
# include "single_node_impl.hpp"
#endif

// used to deduce implementation to invoke at compile time
enum class MachineType 
{
  SINGLE_NODE,
  CLUSTER 
};

// chooses correct internal implementation at compile time 
template <MachineType t, typename... Args>
auto retrograde_analysis(Args&&... args)
{
  if constexpr (t == MachineType::SINGLE_NODE)
    return retrogradeAnalysisBaseImpl(::std::forward<Args>(args)...);

  else
    return retrogradeAnalysisClusterInvoker(::std::forward<Args>(args)...);
}

#endif
