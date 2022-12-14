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
 * The multinode implementation is designed for execution on clusters with MPI support
 */


#ifndef MULTI_NODE_IMPL_HPP_

#include <cmath>
#include <list>
#include <cassert>
#include <iostream>
#include <cstddef>
#include <algorithm>

#include <mpi.h>

#include "state.hpp"
#include "checkmate_generation.hpp"

// Global MPI type definitions. Must be initialized in main with initialize_comm_structs
MPI_Datatype MPI_NodeCommData;
MPI_Datatype MPI_NonPlacementDataType;
MPI_Datatype MPI_BoardState;

// Data sent between MPI nodes
template <::std::size_t FlattenedSz, typename NonPlacementDataType> 
struct NodeCommData 
{
  bool winLabel;
  mutable short G;
  BoardState<FlattenedSz, NonPlacementDataType> b; 
};

template<::std::size_t FlattenedSz, typename NonPlacementDataType>
bool operator==(const NodeCommData<FlattenedSz, NonPlacementDataType>& x, const NodeCommData<FlattenedSz, NonPlacementDataType>& y){
	return x.b == y.b;
}

template<::std::size_t FlattenedSz, typename NonPlacementDataType>
struct NodeCommHasher 
{
  BoardStateHasher<FlattenedSz, NonPlacementDataType> m_boardHasher;

  NodeCommHasher(void) = default;

  auto operator()(const NodeCommData<FlattenedSz, NonPlacementDataType>& commData) const
  {
    return m_boardHasher(commData.b); 
  }
};

struct NodeEstimateData
{
  short T;
  short M;
  short C;
};

// Works under the assumption that NonPlacement DataType is already
// serialized in the MPI_NonPlacementDataType global variable. The user must manually do this previous step
template <::std::size_t FlattenedSz, typename NonPlacementDataType> 
void initialize_comm_structs(void)
{
  // MPI struct serialization inspired by 'Gilles' explanation last updated on May 23, 2017
  // Answer: https://stackoverflow.com/a/33624425
  // Author: https://stackoverflow.com/users/5239503/gilles
  { // serialize Board State
    // C++ preprocessor does not understand template syntax so this is necessary
    typedef BoardState<FlattenedSz, NonPlacementDataType> board_state_t;

    int count = 3;
    int blocklengths[] = { 1, FlattenedSz, 1 };

    MPI_Aint displacements[] = 
    { 
      offsetof(board_state_t, m_player), 
      offsetof(board_state_t, m_board),
      offsetof(board_state_t, nonPlacementData)
    };
    
    MPI_Datatype types[] = { MPI_C_BOOL, MPI_CHAR, MPI_NonPlacementDataType };
    
    MPI_Datatype tmp;
    MPI_Aint lowerBound;
    MPI_Aint extent;

    MPI_Type_create_struct(count, blocklengths, displacements, types,
      &tmp);

    MPI_Type_get_extent(tmp, &lowerBound, &extent);
    MPI_Type_create_resized(tmp, lowerBound, extent, &MPI_BoardState);
    MPI_Type_commit(&MPI_BoardState);
  }
  
  // MPI struct serialization inspired by 'Gilles' explanation last updated on May 23, 2017
  // Answer: https://stackoverflow.com/a/33624425
  // Author: https://stackoverflow.com/users/5239503/gilles
  { // Serialize NodeCommData

    // C++ preprocessor does not understand template syntax so this is necessary
    typedef NodeCommData<FlattenedSz, NonPlacementDataType> node_comm_data_t;

    int count = 3;
    int blocklengths[] = { 1, 1, 1 };

    MPI_Aint displacements[] = 
    { 
      offsetof(node_comm_data_t, winLabel),
      offsetof(node_comm_data_t, G), 
      offsetof(node_comm_data_t, b)
    };

    MPI_Datatype types[] = { MPI_C_BOOL, MPI_SHORT, MPI_BoardState };
    
    MPI_Datatype tmp;
    MPI_Aint lowerBound;
    MPI_Aint extent;

    MPI_Type_create_struct(count, blocklengths, displacements, types,
      &tmp);
    MPI_Type_get_extent(tmp, &lowerBound, &extent);
    MPI_Type_create_resized(tmp, lowerBound, extent, &MPI_NodeCommData);
    MPI_Type_commit(&MPI_NodeCommData);
  }
}

// The major iteration of retrograde analysis. Win states are identified in this iteration
template <typename WinFrontier, typename LoseFrontier, typename Partitioner, typename PredStore, typename EndGameSet,
  typename PredecessorGen, typename BoardMap>
inline auto do_majorIteration(int id, short v, int numProcs, const Partitioner& p, PredStore& predStore, 
    BoardMap&& boardMap, LoseFrontier& loseFrontier, WinFrontier&& winFrontier,
    const EndGameSet& losses, EndGameSet&& wins, PredecessorGen predFn)
{
  // TODO: See if finer grain parallelism can be employed with OpenMP (otherwise just launch more
  // MPI processes)
  bool b_localAssignedWork = false;
  ::std::vector<MPI_Request*> sendRequests;
  for (auto& frontierState : loseFrontier)
  {
    if (wins.find(frontierState.b) == wins.end())
    {
      auto preds = predFn(frontierState.b);
      wins.insert(frontierState.b);

      for (auto&& pred : preds)
      {
        auto targetId = p(pred);
        if (targetId == id)
        {
          auto [_, b_insert] = winFrontier.insert({true, v, ::std::move(pred)}); // tell the predecessor that the current state wins in v moves.
          if (b_insert)
            b_localAssignedWork = true;
        }
        else
        {
          predStore.emplace_back(true, v, ::std::move(pred));
          b_localAssignedWork = true;
          // perform MPI send from current to targetId
          MPI_Request* r = new MPI_Request(); // receiver responsible for knowing 
          sendRequests.push_back(r);
          MPI_Isend(&(predStore.back()), 1, MPI_NodeCommData, targetId, 0,
            MPI_COMM_WORLD, r);
        }
      }
    }
  }
  // currently use null board. can be extended in the future to pack the last message
  for (int i = 0; i < numProcs; ++i)
  {
    if (i != id)
    {
      MPI_Request* r = new MPI_Request();
      sendRequests.push_back(r);
      
      if (loseFrontier.empty())
          loseFrontier.insert({false, 0, { false, {}, {} }});
      
      if (b_localAssignedWork)
      { 
        MPI_Isend(&(*loseFrontier.begin()), 1, MPI_NodeCommData, i, 1,
          MPI_COMM_WORLD, r);
      }
      else
      {
        MPI_Isend(&(*loseFrontier.begin()), 1, MPI_NodeCommData, i, 2,
          MPI_COMM_WORLD, r);
      }
    }
  }
  return ::std::make_tuple(b_localAssignedWork, sendRequests,
    ::std::move(boardMap), ::std::move(winFrontier), ::std::move(wins));
}

// Performs the minor iteration of retrograde analysis where loss moves are identified
template <typename WinFrontier, typename LoseFrontier, typename Partitioner, typename PredStore, typename EndGameSet, 
  typename PredecessorGen, typename SuccessorGen, typename BoardMap>
inline auto do_minorIteration(int id, int v, int numProcs, const Partitioner& p, PredStore& predStore,
    BoardMap&& boardMap, LoseFrontier&& loseFrontier, WinFrontier& winFrontier,
    EndGameSet&& losses, const EndGameSet& wins, PredecessorGen predFn, SuccessorGen succFn)
{
  // TODO: See if finer grain parallelism can be employed with OpenMP (otherwise just launch more
  // MPI processes)
  bool b_localAssignedWork = false;
  ::std::vector<MPI_Request*> sendRequests;

  for (auto& frontierState : winFrontier)
  {
    if (losses.find(frontierState.b) == losses.end() 
        && wins.find(frontierState.b) == wins.end())
    {      
      // account for the fact that successors may have not been calculated
      if (boardMap[frontierState.b].C < 0)
      {
        auto succs = succFn(frontierState.b);
        boardMap[frontierState.b].C += succs.size();
      }
      short remainingPaths = boardMap[frontierState.b].C;
      if (remainingPaths == 0)
      {
        auto preds = predFn(frontierState.b);
        losses.insert(frontierState.b);
        boardMap[frontierState.b].T = boardMap[frontierState.b].M;

        for (auto&& pred : preds)
        {
          auto targetId = p(pred);
          if (targetId == id)
          {
            auto [_, b_insert] = loseFrontier.insert({false, boardMap[frontierState.b].T, ::std::move(pred)});
            if (b_insert)
              b_localAssignedWork = true;
          }
          else
          {
            predStore.emplace_back(false, boardMap[frontierState.b].T, ::std::move(pred));
            b_localAssignedWork = true;
            MPI_Request* r = new MPI_Request();
            sendRequests.push_back(r);
            MPI_Isend(&(predStore.back()), 1, MPI_NodeCommData, targetId, 0, 
              MPI_COMM_WORLD, r);
          }
        }
      }
    }
  }
  // communicate finished message
  for (int i = 0; i < numProcs; ++i)
  {
    if (i != id)
    {
      MPI_Request* r = new MPI_Request();
      sendRequests.push_back(r);
      
      // if no board states, insert null board
      if (winFrontier.empty())
        winFrontier.insert({false, 0, { false, {}, {} }});
      
      if (b_localAssignedWork)
      { 
        MPI_Isend(&(*winFrontier.begin()), 1, MPI_NodeCommData, i, 1, // need to send a 1 if it is the last msg
          MPI_COMM_WORLD, r);
      }
      else
      {
        MPI_Isend(&(*winFrontier.begin()), 1, MPI_NodeCommData, i, 2, // need to send a 2 if it is the last msg
          MPI_COMM_WORLD, r);
      }
    }
  }
  return ::std::make_tuple(b_localAssignedWork, sendRequests, ::std::move(boardMap), 
    ::std::move(loseFrontier), ::std::move(losses));
}

// TODO: Consider more efficient communication scheme with One-sided Communication 
/*
 * The following function is the required synchronization routine performed at the end of
 * each major and minor iteration 
 */
template<bool fromWinIteration, typename BoardMap, typename BoardSet, typename Frontier> 
auto do_syncAndFree(int numNodes, short v,
    ::std::vector<MPI_Request*> sendRequests,
    const BoardSet& wins,
    const BoardSet& losses,
    BoardMap&& boardMap, Frontier&& frontier)
{
  // initially, we only know that the current node is done with computation
  int finishedNodes = 1;
  bool b_otherAssignedWork = false;

  typename ::std::remove_reference<decltype(frontier)>::type::value_type recvBuf;
  // 1. Process all receives for the current node
  do 
  {
    MPI_Status status;
    MPI_Recv(&recvBuf, 1, MPI_NodeCommData, MPI_ANY_SOURCE,
        MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    
    auto tag = status.MPI_TAG;

    if (wins.find(recvBuf.b) == wins.end() 
        && losses.find(recvBuf.b) == losses.end() && tag == 0)
    {
      frontier.insert(recvBuf);

      if constexpr (fromWinIteration)
      {
        // We initialize remaining moves to -1 and handle later. It is too 
        // expensive to handle calculating successors here.
        if (boardMap.find(recvBuf.b) == boardMap.end())
        {
          boardMap[recvBuf.b] = 
            { 0, recvBuf.G, -1 }; 
        }
        // Decrement loss counter and determine current longest path to a loss
        else
        {
          auto& estimateNodeData = boardMap[recvBuf.b];
          --(estimateNodeData.C);
          estimateNodeData.M = ::std::max(estimateNodeData.M, recvBuf.G);
        }
      }
      else // from lose iteration
      {
        if (boardMap.find(recvBuf.b) == boardMap.end())
          boardMap[recvBuf.b] = { static_cast<short>(v + 1), {}, {} }; // last two fields are only relevant to potentially lost states.  
        else
          boardMap[recvBuf.b].T = static_cast<short>(v + 1); 
      }
    }
    if (tag == 1) // consumed all work from sending node
    {
      ++finishedNodes;
      b_otherAssignedWork = true;
    }
    else if (tag == 2)
    {
      ++finishedNodes;
    }
  } while(finishedNodes != numNodes);
  
  // 2. Wait for requests and free memory. All messages should be
  // freed at this point. Cleanup resources.
  for (::std::size_t i = 0; i < sendRequests.size(); ++i)
  {
    MPI_Status status;
    MPI_Wait(sendRequests[i], &status);
    delete sendRequests[i];
  }
  // All nodes must synchronize here prior to ensure all messages have 
  // been consumed
  MPI_Barrier(MPI_COMM_WORLD);
  return ::std::make_tuple(::std::move(boardMap), ::std::move(frontier), b_otherAssignedWork);
}

/*
* Technique for ensuring that a type extends a separate type in the below function templating
* inspired by the following answer by 'AndyG' last updated June 7th, 2015
* author: https://stackoverflow.com/users/27678/andyg
* answer: https://stackoverflow.com/a/30687399 
*
* Implementation inspired by following thesis paper: 
* Makhnychev Vladimir Sergeevich, ???Parallelization of retroanalysis algorithms for solving enumeration problems in computing 
* systems without shared memory,??? Moscow State University, Russia, 2012.
*
* This below function is the internal implementation for the multi-node retrograde analysis implementation. Invoking
* this function assumes an MPI installation on the system 
*/
template<::std::size_t FlattenedSz, typename NonPlacementDataType, ::std::size_t N, 
  ::std::size_t rowSz, ::std::size_t colSz,
  typename MoveGenerator, typename ReverseMoveGenerator, typename HorizontalSymFn=false_fn, 
  typename VerticalSymFn=false_fn, typename IsValidBoardFn=null_type, 
  typename ::std::enable_if<::std::is_base_of<GenerateForwardMoves<FlattenedSz, NonPlacementDataType>, 
    MoveGenerator>::value>::type* = nullptr,
  typename ::std::enable_if<::std::is_base_of<GenerateReverseMoves<FlattenedSz, NonPlacementDataType>, 
    ReverseMoveGenerator>::value>::type* = nullptr>
auto retrogradeAnalysisClusterImpl(const KStateSpacePartition<FlattenedSz, BoardState<FlattenedSz, NonPlacementDataType>>& partitioner, int id, 
    int numProcs, ::std::unordered_set<BoardState<FlattenedSz, NonPlacementDataType>, 
      BoardStateHasher<FlattenedSz, NonPlacementDataType>>&& checkmates,
    MoveGenerator generateSuccessors,
    ReverseMoveGenerator generatePredecessors,
    HorizontalSymFn hzSymFn={}, VerticalSymFn vSymFn={}, 
    IsValidBoardFn isValidBoardFn={})
{
  using board_set_t = ::std::unordered_set<BoardState<FlattenedSz, NonPlacementDataType>, 
    BoardStateHasher<FlattenedSz, NonPlacementDataType>>;
  using frontier_t = ::std::unordered_set<NodeCommData<FlattenedSz, NonPlacementDataType>, 
    NodeCommHasher<FlattenedSz, NonPlacementDataType>>;
  using pred_list_t = ::std::list<NodeCommData<FlattenedSz, NonPlacementDataType>>;
  
  // Estimate data during search - more expensive than omp implementation 
  using board_map_t = 
    ::std::unordered_map<BoardState<FlattenedSz, NonPlacementDataType>, 
    NodeEstimateData,
    BoardStateHasher<FlattenedSz, NonPlacementDataType>>;

  board_set_t wins;
  board_set_t losses = ::std::move(checkmates);
  
  frontier_t winFrontier;
  frontier_t loseFrontier;
  
  board_map_t estimateData;
  ::std::vector<MPI_Request*> sendRequests;
  pred_list_t predList;
  
  // 1. initialization 
  for (const auto& l : losses)
    winFrontier.insert({false, 0, l });
  
  // 2. perform modified minor iteration for init sends
  for (const auto& f : winFrontier)
  {
    auto preds = generatePredecessors(f.b);
    for (auto&& pred : preds)
    {
      auto targetId = partitioner(pred);
      if (targetId != id) // different node processes this
      {
        predList.emplace_back(true, 0, ::std::move(pred));
        MPI_Request* r = new MPI_Request();
        MPI_Isend(&(predList.back()), 1, MPI_NodeCommData, targetId, 0, 
              MPI_COMM_WORLD, r);
      }
      else
      {
        loseFrontier.insert({false, 0, pred });
      }
    }
  }
  for (int i = 0; i < numProcs; ++i)
  {
    if (i != id)
    {
      MPI_Request* r = new MPI_Request();
      sendRequests.push_back(r);
      if (loseFrontier.empty())
        loseFrontier.insert({false, 0, { false, {}, {} }});

      MPI_Isend(&(*loseFrontier.begin()), 1, MPI_NodeCommData, i, 1, // need to send a 1 if it is the last msg
        MPI_COMM_WORLD, r);
    }
  }

  bool b_otherAssignedWork{};
  bool b_localAssignedWork{};
  ::std::tie(estimateData, loseFrontier, b_otherAssignedWork) = do_syncAndFree<false>(numProcs, 0, sendRequests,
      wins, losses, ::std::move(estimateData), ::std::move(loseFrontier)); 

  winFrontier.clear();
  sendRequests.clear();
  predList.clear();

  for (short v = 1; v > 0; ++v)
  {
    // 1. Invoke major iteration
    ::std::tie(b_localAssignedWork, sendRequests, estimateData, winFrontier, wins) = do_majorIteration(id, v, numProcs,
        partitioner, predList, ::std::move(estimateData), loseFrontier, ::std::move(winFrontier), losses,
        ::std::move(wins), generatePredecessors);
    
    ::std::tie(estimateData, winFrontier, b_otherAssignedWork) = do_syncAndFree<true>(numProcs, v, sendRequests,
        wins, losses, ::std::move(estimateData), ::std::move(winFrontier));
    
    sendRequests.clear();
    loseFrontier.clear();
    predList.clear();

    if (!b_otherAssignedWork && !b_localAssignedWork)
      break;

    // 2. Invoke minor iteration
    ::std::tie(b_localAssignedWork, sendRequests, estimateData, loseFrontier, losses) = do_minorIteration(id, v, numProcs,
      partitioner, predList, ::std::move(estimateData), ::std::move(loseFrontier), winFrontier,
      ::std::move(losses), wins, generatePredecessors, generateSuccessors);
    
    ::std::tie(estimateData, loseFrontier, b_otherAssignedWork) = do_syncAndFree<false>(numProcs, v, sendRequests, 
        wins, losses, ::std::move(estimateData), ::std::move(loseFrontier));

    sendRequests.clear();
    winFrontier.clear();
    predList.clear();

    if (!b_otherAssignedWork && !b_localAssignedWork)
      break; 
  }
  return ::std::make_tuple(wins, losses, estimateData);
}

#endif
