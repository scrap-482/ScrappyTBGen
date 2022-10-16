// TODO: Matt reduce code duplication in symmetry functions

#ifndef PERMUTATION_GENERATOR_HPP_ 
#define PERMUTATION_GENERATOR_HPP_

#include "state.hpp"

// fallback for optional function templating
struct null_type {};

struct false_fn
{
  bool operator()(const std::vector<unsigned char>& T) { return false; }
};

// permutation generator functor. This accounts for symmetry of the board
template <::std::size_t FlattenedSz, typename EvalFn, typename IsValidBoardFn = null_type,
         typename HorizontalSymFn = false_fn, typename VerticalSymFn = false_fn>
class PermutationEvaluator
{
   HorizontalSymFn m_hSymFn;
   VerticalSymFn m_vSymFn;

   ::std::size_t m_rowSz;
   ::std::size_t m_colSz;

   bool m_isHSym;
   bool m_isVSym;

public:
  constexpr PermutationEvaluator(HorizontalSymFn hSymFn = false_fn{}, 
      VerticalSymFn vSymFn = false_fn{})
    : m_hSymFn(hSymFn),
      m_vSymFn(vSymFn),
      m_isHSym(false),
      m_isVSym(true)
  {
    if constexpr (!::std::is_same<false_fn, HorizontalSymFn>::value)
      m_isHSym = true;

    if constexpr (!::std::is_same<false_fn, VerticalSymFn>::value)
      m_isVSym = true;
  }

  void inline generatePermutations(::std::vector<BoardState<FlattenedSz>>& checkmates,
    const ::std::vector<piece_label_t>& pieceSet,
    EvalFn checkmateEval,
    IsValidBoardFn boardValidityEval = {})
  {
    ::std::array<::std::size_t, FlattenedSz> indexPermutations;
    ::std::iota(indexPermutations.begin(), indexPermutations.end(), 0);
    
    // currently generates checkmates from 2 to n
    for (::std::size_t k_permute = 2; k_permute != pieceSet.size() + 1; ++k_permute)
    {
      do 
      {
        BoardState<FlattenedSz> currentBoard;
        for (::std::size_t i = 0; i != k_permute; ++i)
          currentBoard.m_board[indexPermutations[i]] = pieceSet[i]; // scatter pieces

        if constexpr (!::std::is_same<null_type, IsValidBoardFn>::value)
          if (!IsValidBoardFn(currentBoard))
            continue;

        if (checkmateEval(currentBoard))
        {
#pragma omp critical 
          {
            checkmates.push_back(currentBoard);
          }
        }
        
        currentBoard.m_player.set();

        if (checkmateEval(currentBoard))
        {
#pragma omp critical
          {
            checkmates.push_back(currentBoard);
          }
        }

        ::std::reverse(indexPermutations.begin() + k_permute, indexPermutations.end());
      } while (::std::next_permutation(indexPermutations.begin(), indexPermutations.end()));
    }
  }

  void inline generateSymPermutations(::std::vector<BoardState<FlattenedSz>>& checkmates,
    const ::std::vector<piece_label_t>& pieceSet,
    EvalFn checkmateEval,
    IsValidBoardFn boardValidityEval = {})
  {
    ::std::array<::std::size_t, FlattenedSz> indexPermutations;
    ::std::iota(indexPermutations.begin(), indexPermutations.end(), 0);
    ::std::size_t mid = indexPermutations[indexPermutations.size() / 2];
    
    // currently generates checkmates from 2 to n
    for (::std::size_t k_permute = 2; k_permute != pieceSet.size() + 1; ++k_permute)
    {
      do 
      {
        BoardState<FlattenedSz> currentBoard;
        for (::std::size_t i = 0; i != k_permute; ++i)
          currentBoard.m_board[indexPermutations[i]] = pieceSet[i]; // scatter pieces
        
        // TODO: this works for even row length boards only.
        if (m_isHSym && mid == indexPermutations[0])
          break;
        
        // TODO: this works for odd column length boards only
        if (m_isVSym && ((indexPermutations[0] % m_colSz) > (m_colSz / 2)))
        {
          // reset 
        }

        if constexpr (!::std::is_same<null_type, IsValidBoardFn>::value)
          if (!IsValidBoardFn(currentBoard))
            continue;

        if (checkmateEval(currentBoard))
        {
#pragma omp critical 
          {
            checkmates.push_back(currentBoard);
          }
        }
        
        currentBoard.m_player.set();

        if (checkmateEval(currentBoard))
        {
#pragma omp critical
          {
            checkmates.push_back(currentBoard);
          }
        }

        ::std::reverse(indexPermutations.begin() + k_permute, indexPermutations.end());
      } while (::std::next_permutation(indexPermutations.begin(), indexPermutations.end()));
    }
  }
  
  auto operator()(::std::vector<BoardState<FlattenedSz>>& checkmates,
    const ::std::vector<piece_label_t>& pieceSet,
    EvalFn eval,
    IsValidBoardFn boardValidityEval = {})
  {
    bool pieceHzSym = false;
    bool pieceVSym = false;
    
    // evaluate horizontal symmetry condition of the piece set
    if (m_isHSym)
    {
      if (m_hSymFn(pieceSet))
        pieceHzSym = true;
    }
    
    // evaluate vertical symmetry condition of the piece set  
    if (m_isVSym)
    {
      if (m_vSymFn(pieceSet))
        pieceVSym = true;
    }
    
    // If piece set is both vertically and horizontally symmetric, 
    // then it is diagonally symmetric.
    if (pieceVSym || pieceHzSym)
    {
      generateSymPermutations(checkmates, pieceSet, eval,
        boardValidityEval);
    }
    else // generate for all 4 quadrants 
    {
      generatePermutations(checkmates, pieceSet, eval,
        boardValidityEval);
    }
  }
};

#endif
