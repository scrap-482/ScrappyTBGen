// NOTE: when using some build flags and not the hardcoded
// checkmates, this program segfaults (due to 
// poor implementation of the callbacks). However, this hacky test case
// will soon be replaced

// Matt's hardcoded validation before rulesets are implemented. 
// Mess around with this file to observe retrograde analysis' behavior 
// requires the hardcoding of checkmates.
// this will be obsolete once we have an actual ruleset
#include "../../src/retrograde_analysis/retrograde_analysis.hpp"
#include "../../src/retrograde_analysis/state_transition.hpp"
#include <iostream>

template<::std::size_t FlattenedSz, typename NonPlacementDataType>
class TestForwardMove 
  : public GenerateForwardMoves<FlattenedSz, NonPlacementDataType> 
{
public:
  ::std::vector<BoardState<FlattenedSz, NonPlacementDataType>> 
  operator()(const BoardState<FlattenedSz, NonPlacementDataType>& b)
  {
    ::std::vector<BoardState<FlattenedSz, NonPlacementDataType>> possible_rev_moves;
    if (b.m_player == 1)
    {
      for (int i = 0; i != b.m_board.size(); ++i)
      {
        if (b.m_board[i] == 'K')
        {
          ::std::array<int, 8> possible_moves = 
            { i + 4, i - 4, i + 3, i - 3, i + 5, i - 5, i + 1, i - 1 };
          
          for (auto pos : possible_moves)
          {
            if ((i >= 0 && i <= 15) && b.m_board[pos] != 'R' && b.m_board[pos] != 'k')
            {
              BoardState<FlattenedSz, NonPlacementDataType> next_move = b;
              next_move.m_player = 0;
              next_move.m_board[i] = {};
              next_move.m_board[pos] = 'K';

              possible_rev_moves.push_back(next_move);
            }
          }
        }
        if (b.m_board[i] == 'R')
        {
          int row = i / 4;
          int col = i % 4;
          
          for (int j = 0; j < 4; ++j)
          {
            if (b.m_board[row*4 + j] != 'K' && b.m_board[row*4 + j] != 'R' && b.m_board[row*4 + j] != 'k') 
            {
              BoardState<FlattenedSz, NonPlacementDataType> next_move = b;
              next_move.m_player = 0;
              next_move.m_board[row*4 + j] = 'R';
              next_move.m_board[i] = {};

              possible_rev_moves.push_back(next_move);
            }
            if (b.m_board[4*j + col] != 'K' && b.m_board[4*j + col] != 'R' && b.m_board[4*j + col] != 'k')
            {
              BoardState<FlattenedSz, NonPlacementDataType> next_move = b;
              next_move.m_player = 0;
              next_move.m_board[col + j*4] = 'R';
              next_move.m_board[i] = {};

              possible_rev_moves.push_back(next_move);
            }
          }
        }

      }
    }
    else
    {
      for (int i = 0; i != b.m_board.size(); ++i)
      {
        if (b.m_board[i] == 'k')
        {
          ::std::array<int, 1> possible_moves = 
            { 12 };
          
          for (auto pos : possible_moves)
          {
            if (i >= 0 && i <= 15)
            {
              BoardState<FlattenedSz, NonPlacementDataType> next_move = b;
              next_move.m_player = 1;
              next_move.m_board[i] = {};
              next_move.m_board[pos] = 'k';

              possible_rev_moves.push_back(next_move);
            }
          }
        }
      }
    }
    return possible_rev_moves;
  }
};

// this is essentially hardcoded for correctness
template<::std::size_t FlattenedSz, typename NonPlacementDataType>
class TestReverseMove
  : public GenerateReverseMoves<FlattenedSz, NonPlacementDataType>
{
public:
  ::std::vector<BoardState<FlattenedSz, NonPlacementDataType>> 
  operator()(const BoardState<FlattenedSz, NonPlacementDataType>& b)
  {
    ::std::vector<BoardState<FlattenedSz, NonPlacementDataType>> possible_rev_moves;
    if (b.m_player == 0)
    {
      for (int i = 0; i != b.m_board.size(); ++i)
      {
        if (b.m_board[i] == 'K')
        {
          ::std::array<int, 8> possible_moves = 
            { i + 4, i - 4, i + 3, i - 3, i + 5, i - 5, i + 1, i - 1 };
          
          for (auto pos : possible_moves)
          {
            if ((pos >= 0 && pos <= 15) && b.m_board[pos] != 'R' && b.m_board[pos] != 'k')
            {
              BoardState<FlattenedSz, NonPlacementDataType> next_move = b;
              next_move.m_player = 1;
              next_move.m_board[i] = {};
              next_move.m_board[pos] = 'K';

              possible_rev_moves.push_back(next_move);
            }
          }
        }
        if (b.m_board[i] == 'R')
        {
          int row = i / 4;
          int col = i % 4;
          
          for (int j = 0; j < 4; ++j)
          {
            if (b.m_board[row*4 + j] != 'K' && b.m_board[row*4 + j] != 'R' && b.m_board[row*4 + j] != 'k') 
            {
              BoardState<FlattenedSz, NonPlacementDataType> next_move = b;
              next_move.m_player = 1;
              next_move.m_board[row*4 + j] = 'R';
              next_move.m_board[i] = {};

              possible_rev_moves.push_back(next_move);
            }
            if (b.m_board[4*j + col] != 'K' && b.m_board[4*j + col] != 'R' && b.m_board[4*j + col] != 'k')
            {
              BoardState<FlattenedSz, NonPlacementDataType> next_move = b;
              next_move.m_player = 1;
              next_move.m_board[col + j*4] = 'R';
              next_move.m_board[i] = {};

              possible_rev_moves.push_back(next_move);
            }
          }
        }
      }
    }
    else
    {
      for (int i = 0; i != b.m_board.size(); ++i)
      {
        if (b.m_board[i] == 'k')
        {
          ::std::array<int, 8> possible_moves = 
            { i + 4, i - 4, i + 3, i - 3, i + 5, i - 5, i + 1, i - 1 };
          
          for (auto pos : possible_moves)
          {
            if (pos >= 0 && pos <= 15)
            {
              BoardState<FlattenedSz, NonPlacementDataType> next_move = b;
              next_move.m_player = 0;
              next_move.m_board[i] = {};
              next_move.m_board[pos] = 'k';

              possible_rev_moves.push_back(next_move);
            }
          }
        }
      }
    }
    return possible_rev_moves;
  }
};

template<::std::size_t FlattenedSz, typename NonPlacementDataType>
class TestCheckmateEvaluator
  : public CheckmateEvaluator<FlattenedSz, NonPlacementDataType>
{
public:
  bool operator()(const BoardState<FlattenedSz, NonPlacementDataType>& b)
  {
    return true;
  }
};

// sanity test case QKvk on 4x4 board
// TODO: integrate mpi checkmate testing
int main()
{
  constexpr ::std::size_t numRows = 4;
  constexpr ::std::size_t numCols = 4;
  
  std::vector<piece_label_t> noRoyaltyPieceset = { 'q' };
  std::vector<piece_label_t> royaltyPieceset = { 'k', 'q', 'K' };
  TestForwardMove<16, null_type> forward;
  TestReverseMove<16, null_type> backward;
  TestCheckmateEvaluator<16, null_type> evaluator;

  auto checkmates = generateAllCheckmates<16, null_type, 3, 4, 4,
       decltype(evaluator)>(noRoyaltyPieceset, royaltyPieceset, evaluator);
  auto [config, checkmates_0] = checkmates[0];

  
#ifdef MULTI_NODE
  KStateSpacePartition<16, BoardState<16, null_type>> p('k', 4);
  auto [wins, losses] = retrogradeAnalysisClusterImpl<16, null_type, 3, 
    4, 4, decltype(forward), decltype(backward)>(p, 2, 16, ::std::move(checkmates_0),
      forward, backward);
#else
  auto [wins, losses, dtm] = retrogradeAnalysisBaseImpl<16, null_type, 3, 
    4, 4, decltype(forward), decltype(backward)>(::std::move(checkmates_0),
      forward, backward);
#endif
}
