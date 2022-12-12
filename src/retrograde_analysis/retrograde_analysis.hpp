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
