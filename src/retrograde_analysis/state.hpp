#ifndef STATE_HPP_
#define STATE_HPP_

#include <array>
#include <bitset>
#include <functional>

using piece_label_t = unsigned char;

#if 1
template<::std::size_t FlattenedSz, typename NonPlacementDataType>
struct BoardState
{
  // 1 for white move. 0 for black move
  ::std::bitset<1> m_player;
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
    stringifiedBoard += static_cast<char>(b.m_player.to_ulong());
    
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

#endif
