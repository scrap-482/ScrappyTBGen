#include <iostream>
#include "state_transition.hpp"
#include "checkmate_generation.hpp"

template<::std::size_t FlattenedSz>
struct cm_function : public CheckmateEvaluator<FlattenedSz>
{
  bool operator()(const BoardState<FlattenedSz>& b)
  { return true; }
};

int main()
{
  std::vector<piece_label_t> noRoyaltyPieceset = { 'r', 'r', 'b', 'b', 'n', 'n', 'q', 'p', 'p', 'p', 'p', 'p', 'p', 'p', 'p', 'q', 'q', 'n', 'r', 'b',
                                                   'R', 'R', 'B', 'B', 'N', 'N', 'Q', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'Q', 'Q', 'N', 'R', 'B'};
  std::vector<piece_label_t> royaltyPieceset = { 'k', 'K' };
  
  // need to fix
  auto symFn = [](const std::vector<piece_label_t>& v)
  {
    for (const auto& c : v)
    {
      if (c == 'p' || c =='P')
        return false;
    }
    return true;
  };
  
  auto c = cm_function<64>{};
  auto [white_wins, white_losses] = generateAllCheckmates<64, 3, 8, 8, decltype(c),
       decltype(symFn), decltype(symFn)>(noRoyaltyPieceset, royaltyPieceset, c, symFn, symFn);
  std::cout << white_wins.size() + white_losses.size() << std::endl;
}
