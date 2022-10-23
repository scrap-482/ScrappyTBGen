#include "retrograde_analysis.hpp"
#include "state_transition.hpp"

template<::std::size_t FlattenedSz, typename NonPlacementDataType>
class TestForwardMove 
  : public GenerateForwardMoves<FlattenedSz, NonPlacementDataType> 
{
public:
  ::std::vector<BoardState<FlattenedSz, NonPlacementDataType>> 
  operator()(const BoardState<FlattenedSz, NonPlacementDataType>& b)
  {
    return {};
  }
};

template<::std::size_t FlattenedSz, typename NonPlacementDataType>
class TestReverseMove
  : public GenerateReverseMoves<FlattenedSz, NonPlacementDataType>
{
public:
  ::std::vector<BoardState<FlattenedSz, NonPlacementDataType>> 
  operator()(const BoardState<FlattenedSz, NonPlacementDataType>& b)
  {
    return {};
  }
};

template<::std::size_t FlattenedSz, typename NonPlacementDataType>
class TestCheckmateEvaluator
  : public CheckmateEvaluator<FlattenedSz, NonPlacementDataType>
{
  //const ::std::vector<piece_label_t>& m_royaltyPieceset;

  //TestCheckmateEvaluator(const ::std::vector<piece_label_t>& royaltyPieceset)
  //  : m_royaltyPieceset(royaltyPieceset)
  //{ }

public:
  bool operator()(const BoardState<FlattenedSz, NonPlacementDataType>& b)
  {
    return true;
  }
};

// sanity test case QKvk on 4x4 board
int main()
{
  constexpr ::std::size_t numRows = 4;
  constexpr ::std::size_t numCols = 4;
  
  std::vector<piece_label_t> noRoyaltyPieceset = { 'Q' };
  std::vector<piece_label_t> royaltyPieceset = { 'k', 'K' };
  TestForwardMove<16, null_type> forward;
  TestReverseMove<16, null_type> backward;
  TestCheckmateEvaluator<16, null_type> evaluator;

  auto results = retrogradeAnalysisBaseImpl<16, null_type, 3, 
    4, 4, decltype(evaluator), decltype(forward), decltype(backward)>(noRoyaltyPieceset, royaltyPieceset,
      forward, backward, evaluator);
}
