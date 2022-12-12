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
 * This header is utilized to probe a tablebase once stored in memory
 */
#ifndef PROBE_HPP_

#include <vector>
#include "state.hpp"
#include <iostream>

/*
 * Probing technique inspired by the following paper
 * Makhnychev Vladimir Sergeevich, “Parallelization of retroanalysis algorithms for solving enumeration problems in computing 
 * systems without shared memory,” Moscow State University, Russia, 2012.
 *
 * The following probing function is to be invoked after a tablebase is generated. The function
 * returns the depth-to-mate depth along with a pathway from the original state to the checkmate state. 
 * Furthermore, the pathway is printed to the terminal with the BoardPrinter through execution
 */
template<typename BoardType, typename MapType, typename SuccFn,
  typename BoardPrinter>
auto probe(const BoardType& b, const MapType& m, SuccFn succFn, bool isWinIteration,
    BoardPrinter print)
{
  BoardType g = b;
  int depthToEnd = 0;
  auto v = m.at(g);
  std::vector<BoardType> pathwayToEnd = {g};
  for (;;)
  {
    std::cout << print(g) << std::endl;
    auto succs = succFn(g);
    if (succs.size() == 0) // checkmate
      break;
    for (const auto& succ : succs)
    {
      // exploring a draw state
      if (m.find(succ) == m.end())
        continue;
      auto lvlToEnd = m.at(succ);
      // levels are doubled up per move
      if (isWinIteration && lvlToEnd == v - 1) 
      {
        g = succ;
        pathwayToEnd.push_back(g);
        ++depthToEnd;
        v = lvlToEnd;
        break;
      }
      else if (!isWinIteration && lvlToEnd == v)
      {
        g = succ;
        pathwayToEnd.push_back(g);
        ++depthToEnd;
        v = lvlToEnd;
        break;
      }
    }
    isWinIteration = !isWinIteration;
  }
  return std::make_tuple(depthToEnd, pathwayToEnd);
}
#endif
