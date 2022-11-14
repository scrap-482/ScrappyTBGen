// this is currently a benchmark used by matt to see how fast we 
// are permuting through the states of a chess game

#include <iostream>
#include <random>

#include "../../src/retrograde_analysis/state_transition.hpp"
#include "../../src/retrograde_analysis/checkmate_generation.hpp"

// Non-placement data
struct StandardChessNPD {
    int enpassantRights;
    // We do not consider castling rights because it has negligable effect on end game.
};

template<::std::size_t FlattenedSz, typename NonPlacementDataType>
struct cm_function : 
  public CheckmateEvaluator<FlattenedSz, NonPlacementDataType>
{
  ::std::random_device& m_rd;
  ::std::uniform_int_distribution<int>& m_distribution; 
  ::std::mt19937& m_engine;
  int sum;
public:
  cm_function(std::random_device& rd, std::uniform_int_distribution<int>& distribution,
      std::mt19937& engine)
    : m_rd(rd),
      m_distribution(distribution),
      m_engine(engine)
  {
  }

  bool operator()(const BoardState<FlattenedSz, NonPlacementDataType>& b)
  { // test case. odds of checkmate 1 / 100
#if 1
    int value = m_distribution(m_engine);
    if (value < 2)
      return true;

    return false;
#else
    return true;
#endif
  }
};

int main()
{
  std::vector<piece_label_t> noRoyaltyPieceset = { 'r', 'r', 'b', 'b', 'n', 'n', 'q', 'p', 'p', 'p', 'p', 'p', 'p', 'p', 'p', 'q', 'q', 'n', 'r', 'b',
                                                   'R', 'R', 'B', 'B', 'N', 'N', 'Q', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'Q', 'Q', 'N', 'R', 'B'};
  std::vector<piece_label_t> royaltyPieceset = { 'k', 'K' };
  
  auto symFn = [](const std::vector<piece_label_t>& v)
  {
    for (const auto& c : v)
    {
      if (c == 'p' || c =='P')
        return false;
    }
    return true;
  };
  
  auto rd = std::random_device{};
  auto m_distribution = std::uniform_int_distribution<int>(1, 100);
  auto m_engine = std::mt19937(rd()); // Mersenne twister MT19937

  
  auto c = cm_function<64, StandardChessNPD>(rd, m_distribution, m_engine);

#if 0
  auto losses = generateAllCheckmates<64, StandardChessNPD, 4, 8, 8, decltype(c),
       decltype(symFn), decltype(symFn)>(noRoyaltyPieceset, royaltyPieceset, c, symFn, symFn);
#else
  auto losses = generateAllCheckmates<64, StandardChessNPD, 3, 8, 8, decltype(c)>(noRoyaltyPieceset, royaltyPieceset, c);
#endif
  
  int total_cms = 0;
  for (const auto& [config, cm_set] : losses)
  {
    total_cms += cm_set.size(); 
  }
  std::cout << total_cms << std::endl;
}
