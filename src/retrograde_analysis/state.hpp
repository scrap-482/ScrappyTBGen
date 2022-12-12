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
 * The generic board representation with the NonPlacementDataType is provided in this
 * file
 */

#ifndef STATE_HPP_
#define STATE_HPP_

#include <array>
#include <bitset>
#include <tuple>
#include <cassert>
#include <functional>
#include <string>

#include "piece_label.hpp"

#if 1
// The flattened size is the 1d size of the board. Ex: 8x8 chess has flattened size of 64
// The NonPlacementDataType is any domain-specific type inserted by the user
template<::std::size_t FlattenedSz, typename NonPlacementDataType>
struct BoardState
{
  // 1 for white move. 0 for black move
  //::std::bitset<1> m_player;
  bool m_player;
  ::std::array<piece_label_t, FlattenedSz> m_board{};
  NonPlacementDataType nonPlacementData;
};

// Forward declare this; user will specify
template<typename NonPlacementDataType>
std::string NPDToString(const NonPlacementDataType& npd);

// TODO: hash the NonPlacementType? 
template<::std::size_t FlattenedSz, typename NonPlacementDataType>
struct BoardStateHasher
{
  auto operator()(const BoardState<FlattenedSz, NonPlacementDataType>& b) const
  {
    ::std::string stringifiedBoard(b.m_board.begin(), b.m_board.end());
    stringifiedBoard += static_cast<char>(b.m_player);
    
    return ::std::hash<::std::string>{}(stringifiedBoard); 
  }
};

template<::std::size_t FlattenedSz, typename NonPlacementDataType>
bool operator==(const BoardState<FlattenedSz, NonPlacementDataType>& x, const BoardState<FlattenedSz, NonPlacementDataType>& y){
	return x.m_board == y.m_board;
}
#else // TODO: Potential future bitboard optimization 
template<::std::size_t FlattenedSz, typename NonPlacementDataType, ::std::size_t NumUniquePieces>
struct bb_state
{
  using bitboard_t = ::std::bitset<FlattenedSz>;

  ::std::array<bitboard_t, NumUniquePieces> m_piece_bb;

  bitboard_t m_white_bit_board;
  bitboard_t m_black_bit_board;
  NonPlacementDataType nonPlacementData;
};
#endif

// TODO: better scheme as future work.
// contingent on the location of a single piece on the board. each 
// process is assigned all positions dependent on position of one piece
template <::std::size_t FlattenedSz, typename BoardType>
class KStateSpacePartition
{
  piece_label_t m_toTrack;
  int m_segLength;

public:
  KStateSpacePartition(const piece_label_t& toTrack, int K)
    : m_toTrack(toTrack),
      m_segLength(FlattenedSz / K)
  {
    // with this partitioning scheme, cannot have more nodes than max board size.
    assert(FlattenedSz >= K);
  }
  
  // Contingent on tracked piece location 
  int operator()(const BoardType& b) const
  {
    int idx = 0;
    
    for (const auto& c : b.m_board)
    {
      if (c == m_toTrack)
        break;

      ++idx;
    }
    return idx / m_segLength;
  }

  auto getRange(int k) const
  {
    return ::std::make_tuple(k * m_segLength, 
        ::std::min((k+1) * m_segLength, static_cast<int>(FlattenedSz)));
  }
  
  inline bool checkInRange(const auto& startBoard, 
    const auto& currentBoard) const
  {
    return (currentBoard[0] - startBoard[0]) < m_segLength; 
  }
};

#endif
