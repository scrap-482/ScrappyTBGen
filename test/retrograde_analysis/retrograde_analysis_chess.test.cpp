#include "../../src/retrograde_analysis/state_transition.hpp"
#include "../../src/retrograde_analysis/checkmate_generation.hpp"
#include "../../src/retrograde_analysis/retrograde_analysis.hpp"

#include "../../src/rules/chess/interface.h"

#include <cassert>
#include <iostream>

struct BucketStats {
    size_t occupied = 0;
    size_t total_collisions = 0;
    size_t max_collisions = 0;

    template<class... Args>
    BucketStats(std::unordered_set<Args...> const& c)
    {
        for(auto bucket = c.bucket_count(); bucket--;) {
            auto bucket_size = c.bucket_size(bucket);
            occupied += bucket_size > 0;
            if(bucket_size > 1) {
                auto collisions = bucket_size - 1;
                total_collisions += collisions;
                max_collisions = std::max(max_collisions, collisions);
            }
        }
    }

    double avg_collisions() const {
        return occupied ? static_cast<double>(total_collisions) / occupied : 0;
    }

    friend std::ostream& operator<<(std::ostream& s, BucketStats const& b) {
        return s
            << "used buckets: " << b.occupied
            << "; total collisions: " << b.total_collisions
            << "; max collisions in a bucket: " << b.max_collisions
            << "; avg collisions per bucket: " << b.avg_collisions()
            ;
    }
};
template <class Map> 
double unordered_map_badness(Map const& map)
{
    auto const lambda = map.size() / double(map.bucket_count());

    auto cost = 0.;
    for (auto const& [k, _] : map)
        cost += map.bucket_size(map.bucket(k));
    cost /= map.size();

    return std::max(0., cost / (1 + lambda) - 1);
}

template <class Set> 
double unordered_set_badness(Set const& set)
{
    auto const lambda = set.size() / double(set.bucket_count());
    auto cost = 0.;
    for (auto const& k : set)
        cost += set.bucket_size(set.bucket(k));
    cost /= set.size();

    return std::max(0., cost / (1 + lambda) - 1);
}

template<typename BoardType, typename MapType, typename SuccFn>
auto probe(const BoardType& b, const MapType& m, SuccFn succFn, bool isWinIteration)
{
  BoardType g = b;
  int depthToEnd = 0;
  auto v = m.at(g);
  auto print = ChessBoardPrinter();
  std::vector<BoardType> pathwayToEnd = {g};
  for (;;)
  {
    auto succs = succFn(g);
    if (succs.size() == 0)
      break;
    bool pathFound = false;
    std::cout << print(g) << std::endl;
    std::cout << "v=" << v << " succ v= ";
    for (const auto& succ : succs)
    {
      // exploring a draw state
      if (m.find(succ) == m.end())
        continue;
      auto lvlToEnd = m.at(succ);
      std::cout << lvlToEnd << " ";
      // levels are doubled up per move
      if (isWinIteration && lvlToEnd == v - 1) 
      {
        //std::cout << lvlToEnd << std::endl;
        g = succ;
        pathwayToEnd.push_back(g);
        ++depthToEnd;
        v = lvlToEnd;
        std::cout << lvlToEnd << " " << depthToEnd << std::endl;
        pathFound = true;
        break;
      }
      else if (!isWinIteration && lvlToEnd == v)
      {
        //std::cout << lvlToEnd << std::endl;
        g = succ;
        pathwayToEnd.push_back(g);
        ++depthToEnd;
        std::cout << lvlToEnd << " " << depthToEnd << std::endl;
        v = lvlToEnd;
        pathFound = true;
        break;
      }
    }
    std::cout << std::endl;
    assert(pathFound);
    if (v == 0)
      break;
    isWinIteration = !isWinIteration;
  }
  return std::make_tuple(depthToEnd, pathwayToEnd);
}

int main()
{
  auto fwdMoveGenerator = ChessGenerateForwardMoves();
  auto revMoveGenerator = ChessGenerateReverseMoves();
  auto winCondEvaluator = ChessCheckmateEvaluator();
  auto boardPrinter = ChessBoardPrinter();

  constexpr ::std::size_t N      = 3;
  constexpr ::std::size_t ROW_SZ = 8;
  constexpr ::std::size_t COL_SZ = 8;
  
  BoardState<64, ChessNPD> a 
  { 
    true,
    {
      '\0','\0','\0', 'K','\0','\0','\0','\0',
      '\0','\0','\0','\0','\0','\0','\0','\0',
      '\0','\0','\0','\0','\0','\0','\0','\0',
      '\0', 'k','\0','\0', 'q','\0','\0','\0',
      '\0','\0','\0','\0','\0','\0','\0','\0',
      '\0','\0','\0','\0','\0','\0','\0','\0',
      '\0','\0','\0','\0','\0','\0','\0','\0',
      '\0','\0','\0','\0','\0','\0','\0','\0',
    },
    {-1}
  };

  BoardState<64, ChessNPD> b 
  { 
    true,
    {
      '\0','\0','K','\0', 'q','\0','\0','\0',
      '\0','\0','\0','\0','\0','\0','\0','\0',
      '\0','\0','k','\0','\0','\0','\0','\0',
      '\0','\0','\0','\0','\0','\0','\0','\0',
      '\0','\0','\0','\0','\0','\0','\0','\0',
      '\0','\0','\0','\0','\0','\0','\0','\0',
      '\0','\0','\0','\0','\0','\0','\0','\0',
      '\0','\0','\0','\0','\0','\0','\0','\0',
    },
    {-1}
  };
  
  auto asuccs = fwdMoveGenerator(a);

  // TODO: This is hardcoded for now. playout for correctness
  std::vector<piece_label_t> noRoyaltyPieceset = { 'q' };
  std::vector<piece_label_t> royaltyPieceset = { 'k', 'K' };

  std::vector<piece_label_t> fullPieceset = { 'K', 'k', 'q'};
  
  //auto checkmates = generateAllCheckmates<64, ChessNPD, N, ROW_SZ, COL_SZ,
  //  decltype(winCondEvaluator)>(noRoyaltyPieceset, royaltyPieceset, evaluator);
  
  auto checkmates = generateConfigCheckmates<64, ChessNPD, N, ROW_SZ, COL_SZ,
       decltype(winCondEvaluator)>(fullPieceset, winCondEvaluator);

  std::cout << checkmates.size() << std::endl;
  
  std::cout << "done with checkmates" << std::endl;

  auto [wins, losses, dtm] = retrogradeAnalysisBaseImpl<64, ChessNPD, N, ROW_SZ, 
    COL_SZ, decltype(fwdMoveGenerator), decltype(revMoveGenerator)>(::std::move(checkmates),
      fwdMoveGenerator, revMoveGenerator);

  std::cout << unordered_set_badness(wins) << std::endl;
  std::cout << unordered_set_badness(losses) << std::endl;

  //BoardState<64, ChessNPD>  longest3ManDTM 
  //{ 
  //  true,
  //  {
  //    '\0','\0','\0','\0','\0','\0','\0','\0',
  //    '\0','\0','\0','\0','\0','\0','\0','\0',
  //    'k','\0','\0','\0','\0','\0','\0','\0',
  //    '\0','\0','\0','\0','\0','\0','\0','\0',
  //    '\0','\0','\0','\0', 'K','\0','\0','\0',
  //    '\0','\0','\0','\0','\0','\0','\0','\0',
  //    '\0', 'q','\0','\0','\0','\0','\0','\0',
  //    '\0', '\0','\0','\0','\0','\0','\0','\0',
  //  },
  //  {-1}
  //};

  // Farthest state to end with optimal play for 3 man.
  // This is essentially the last state visited in the retrograde 
  // analysis implementation 
  //BoardState<64, ChessNPD> longest3ManDTM 
  //{ 
  //  false,
  //  {
  //    '\0','\0','\0','\0','\0','\0','\0','\0',
  //    '\0','\0','\0','\0','\0','\0','\0','\0',
  //    '\0','\0','\0','\0','\0','\0','\0','\0',
  //    '\0','\0','\0','\0','\0','\0','\0','\0',
  //    '\0','\0','\0','\0', 'K','\0','\0','\0',
  //    '\0','\0','\0','\0','\0','\0','\0','\0',
  //    '\0', 'q','\0','\0','\0','\0','\0','\0',
  //    'k', '\0','\0','\0','\0','\0','\0','\0',
  //  },
  //  {-1}
  //};

  BoardState<64, ChessNPD> longest3ManDTM 
  { 
    true,
    {
      '\0','\0','\0','\0','\0','\0','\0','\0',
      '\0','\0','\0', 'K','\0','\0','\0','\0',
      '\0','\0','\0','\0','\0','\0','\0','\0',
      '\0', 'k','\0','\0', 'q','\0','\0','\0',
      '\0','\0','\0','\0','\0','\0','\0','\0',
      '\0','\0','\0','\0','\0','\0','\0','\0',
      '\0','\0','\0','\0','\0','\0','\0','\0',
      '\0','\0','\0','\0','\0','\0','\0','\0',
    },
    {-1}
  };
  
  std::cout << wins.size() << " " << losses.size() << std::endl;
  std::cout << BucketStats(wins) << std::endl;
  std::cout << BucketStats(losses) << std::endl;

  bool isWin = wins.find(longest3ManDTM) != wins.end();
  bool isLoss = losses.find(longest3ManDTM) != losses.end();
  if (isWin)
    std::cout << "is a win in " << dtm[longest3ManDTM] << " pairs of moves" << std::endl;
  else if (isLoss)
    std::cout << "is a loss in " << dtm[longest3ManDTM] << " pairs of moves" << std::endl;
  else
    std::cout << "is a draw" << std::endl;

  auto [depthToEnd, pathwayToEnd] = probe(asuccs[0], dtm, fwdMoveGenerator, true);
  auto [depthToEnd2, pathwayToEnd2] = probe(asuccs[1], dtm, fwdMoveGenerator, true);
  
  for (const auto& g : pathwayToEnd2)
    std::cout << boardPrinter(g) << std::endl;

  std::cout << "depth to end: " << depthToEnd << std::endl;

  for (auto a : asuccs)
    std::cout << dtm[a] << std::endl;

  assert(depthToEnd == 20);
  
  return 0;
}
