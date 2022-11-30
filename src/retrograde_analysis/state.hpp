#ifndef STATE_HPP_
#define STATE_HPP_

#include <array>
#include <bitset>
#include <tuple>
#include <cassert>
#include <functional>

#include "piece_label.hpp"

#if 1
template<::std::size_t FlattenedSz, typename NonPlacementDataType>
struct BoardState
{
  // 1 for white move. 0 for black move
  //::std::bitset<1> m_player;
  bool m_player;
  ::std::array<piece_label_t, FlattenedSz> m_board{};
  NonPlacementDataType nonPlacementData;
};

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
