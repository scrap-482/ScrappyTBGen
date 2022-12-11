/*
 * Wrappers around the base implementations of retrograde analysis
 * are provided within this header file. They are not currently used
 * within main.cpp but this may be used for convenience by users in the future
 * so that for both single node and cluster systems, the same function may be invoked
 * with different template and parameter arguments 
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
