#ifndef STATE_HPP_
#define STATE_HPP_

#include <array>
#include <bitset>

using piece_label_t = unsigned char;

#if 1
template<::std::size_t FlattenedSz, typename NonPlacementDataType>
struct BoardState
{
  // 1 for white move. 0 for black move
  ::std::bitset<1> m_player;
  ::std::array<piece_label_t, FlattenedSz> m_board;
  NonPlacementDataType nonPlacementData;
};
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

// TODO: "unmove" operations 
template<::std::size_t FlattenedSz, typename NonPlacementDataType>
auto
generatePredecessors(const BoardState<FlattenedSz, NonPlacementDataType>& board);

#endif
