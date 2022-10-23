#ifndef PERMUTATION_GENERATOR_HPP_ 
#define PERMUTATION_GENERATOR_HPP_

#include <vector>
#include <array>
#include <type_traits>
#include <algorithm>
#include <numeric>

#include <omp.h>

#include "state_transition.hpp"

// fallback for optional function templating - takes 1 byte of space
struct null_type {};

// false function default for symmetry analysis
struct false_fn
{
  bool operator()(const std::vector<unsigned char>& T) { return false; }
};

// permutation generator functor. This exploits symmetry if present 
template <::std::size_t FlattenedSz, typename NonPlacementDataType, ::std::size_t m_rowSz, ::std::size_t m_colSz, typename EvalFn, 
         typename HorizontalSymFn = false_fn, typename VerticalSymFn = false_fn, typename IsValidBoardFn = null_type>
class PermutationEvaluator
{
   HorizontalSymFn m_hSymFn;
   VerticalSymFn m_vSymFn;
   IsValidBoardFn m_isValidBoardFn;
  
   bool m_isHSym;
   bool m_isVSym;

public:
  constexpr PermutationEvaluator(HorizontalSymFn hSymFn = {}, VerticalSymFn vSymFn = {}, 
    IsValidBoardFn isValidBoardFn = {})
    : m_hSymFn(hSymFn),
      m_vSymFn(vSymFn),
      m_isValidBoardFn(isValidBoardFn),
      m_isHSym(false),
      m_isVSym(false)
  {
    // if the given functors / lambdas are not the defaults, then symmetry is present
    if constexpr (!::std::is_same<false_fn, HorizontalSymFn>::value)
      m_isHSym = true;
    if constexpr (!::std::is_same<false_fn, VerticalSymFn>::value)
      m_isVSym = true;
  }
  
  // thread safe function for generating permutations
  void inline generatePermutations(::std::unordered_set<BoardState<FlattenedSz, NonPlacementDataType>, BoardStateHasher<FlattenedSz, NonPlacementDataType>>& whiteWins,
    ::std::unordered_set<BoardState<FlattenedSz, NonPlacementDataType>, BoardStateHasher<FlattenedSz, NonPlacementDataType>>& whiteLosses,
    const ::std::vector<piece_label_t>& pieceSet,
    EvalFn checkmateEval,
    IsValidBoardFn boardValidityEval = {})
  {
    ::std::array<::std::size_t, FlattenedSz> indexPermutations;
    ::std::iota(indexPermutations.begin(), indexPermutations.end(), 0);

    ::std::size_t kPermute = pieceSet.size();
    
    // generates kPermute new checkmate positions.
    // TODO: consider how to store smaller checkmates. Do we keep in storage somewhere?
    do 
    {
      BoardState<FlattenedSz, NonPlacementDataType> currentBoard;
      for (::std::size_t i = 0; i != kPermute; ++i)
        currentBoard.m_board[indexPermutations[i]] = pieceSet[i]; // scatter pieces

      if constexpr (!::std::is_same<null_type, IsValidBoardFn>::value)
        if (!IsValidBoardFn(currentBoard))
          continue;
      
      // checking if black loses (white wins) 
      if (checkmateEval(currentBoard))
      {
#pragma omp critical 
        {
          whiteWins.insert(currentBoard);
        }
      }
      
      currentBoard.m_player.set();
      
      // checking if white loses (black wins)
      if (checkmateEval(currentBoard))
      {
#pragma omp critical
        {
          whiteLosses.insert(currentBoard);
        }
      }

      ::std::reverse(indexPermutations.begin() + kPermute, indexPermutations.end());
    } while (::std::next_permutation(indexPermutations.begin(), indexPermutations.end()));
  }
  
  // thread safe function for generating permutations exploiting symmetry for 
  // horizontal, vertical, (and by extension diagonal) symmetries.
  void inline generateSymPermutations(::std::unordered_set<BoardState<FlattenedSz, NonPlacementDataType>, BoardStateHasher<FlattenedSz, NonPlacementDataType>>& whiteWins,
    ::std::unordered_set<BoardState<FlattenedSz, NonPlacementDataType>, BoardStateHasher<FlattenedSz, NonPlacementDataType>>& whiteLosses,
    const ::std::vector<piece_label_t>& pieceSet,
    EvalFn checkmateEval,
    IsValidBoardFn boardValidityEval = {})
  {
    ::std::vector<::std::size_t> indexPermutations(FlattenedSz);
    ::std::iota(indexPermutations.begin(), indexPermutations.end(), 0);
    ::std::size_t mid = indexPermutations[indexPermutations.size() / 2];

    ::std::size_t kPermute = pieceSet.size();
    
    do 
    {
      BoardState<FlattenedSz, NonPlacementDataType> currentBoard;
      for (::std::size_t i = 0; i != kPermute; ++i)
        currentBoard.m_board[indexPermutations[i]] = pieceSet[i]; // scatter pieces
      
      // TODO: generalize - this works for even row length boards only.
      // To prune out horizontally mirrored positions, we track the movement 
      // of a single piece as its position is permuted. Once, we permute this piece for
      // the upper half of the board, then all horizontal mirrors can be deduced
      if (m_isHSym && (mid == indexPermutations[0]))
        break;
      
      // TODO: generalize - this works for even column length boards only
      // To prune out vertically mirrored positions, we limit a single piece's positioning on the
      // left-hand side of the board. Once we permute for the entire left hand side, then 
      // the vertical mirros can be deduced.
      if (m_isVSym && ((indexPermutations[0] % m_colSz) > (m_colSz / 2)))
      {
        auto next_first_idx = indexPermutations[0] + (m_colSz - indexPermutations[0] % m_colSz);

        auto insert_new = ::std::find_if(indexPermutations.begin(),
            indexPermutations.end(),
            [&indexPermutations] (const auto& e) { return e > indexPermutations[0]; });
        indexPermutations.insert(insert_new, indexPermutations[0]);
        
        auto new_first = ::std::find(indexPermutations.begin(),
            indexPermutations.end(), next_first_idx);
        
        // If this occurs, then we are at the end of generation
        if (new_first == indexPermutations.end() 
            || insert_new == indexPermutations.end())
        { 
          break; 
        }
   
        indexPermutations[0] = *new_first;
        indexPermutations.erase(new_first);
      }

      if constexpr (!::std::is_same<null_type, IsValidBoardFn>::value)
        if (!IsValidBoardFn(currentBoard))
          continue;
      
      // checking if black loses (white wins)
      if (checkmateEval(currentBoard))
      {
#pragma omp critical 
        {
          whiteWins.insert(currentBoard);
        }
      }
      
      currentBoard.m_player.set();

      if (checkmateEval(currentBoard))
      {
#pragma omp critical
        {
          whiteLosses.insert(currentBoard);
        }
      }

      ::std::reverse(indexPermutations.begin() + kPermute, indexPermutations.end());
    } while (::std::next_permutation(indexPermutations.begin(), indexPermutations.end()));
  }
  
  // makes class a functor that evaluates the permutations 
  auto operator()(::std::unordered_set<BoardState<FlattenedSz, NonPlacementDataType>, BoardStateHasher<FlattenedSz, NonPlacementDataType>>& whiteWins,
    ::std::unordered_set<BoardState<FlattenedSz, NonPlacementDataType>, BoardStateHasher<FlattenedSz, NonPlacementDataType>>& whiteLosses, 
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
    
    // If piece set is vertically and/or horizontally symmetric
    if (pieceVSym || pieceHzSym)
    {
      generateSymPermutations(whiteWins, whiteLosses, pieceSet, eval,
        boardValidityEval);
    }
    else // generate all permutations 
    {
      generatePermutations(whiteWins, whiteLosses, pieceSet, eval,
        boardValidityEval);
    }
  }
};

#endif