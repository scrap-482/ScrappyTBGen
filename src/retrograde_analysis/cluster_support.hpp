#ifndef CLUSTER_SUPPORT_H_
#define CLUSTER_SUPPORT_H_

#include <tuple>
#include <bitset>
#include <cmath>
#include <cassert>
#include <iostream>

template <::std::size_t FlattenedSz>
class KStateSpacePartition
{
  // inclusive range
  ::std::bitset<FlattenedSz> m_lowerRange;
  ::std::bitset<FlattenedSz> m_upperRange;

  ::std::size_t m_id;
  ::std::size_t m_k;
  ::std::size_t m_partitionSzLog;
  
  // https://www.geeksforgeeks.org/arithmetic-operations-with-stdbitset-in-c/
  bool fullAdder(bool b1, bool b2, bool& carry)
  {
      bool sum = (b1 ^ b2) ^ carry;
      carry = (b1 && b2) || (b1 && carry) || (b2 && carry);
      return sum;
  }

  // https://www.geeksforgeeks.org/arithmetic-operations-with-stdbitset-in-c/
  // Function to add two bitsets
  std::bitset<FlattenedSz> bitsetAdd(std::bitset<FlattenedSz>& x, std::bitset<FlattenedSz>& y)
  {
      bool carry = false;
      // bitset to store the sum of the two bitsets
      std::bitset<FlattenedSz> ans;
      for (int i = 0; i < FlattenedSz + 1; i++) {
          ans[i] = fullAdder(x[i], y[i], carry);
      }
      return ans;
  }

  void initRange(void)
  {
    ::std::size_t base2Log = ::std::log2(m_k);
    assert(base2Log > 0);

    m_partitionSzLog = FlattenedSz - base2Log;
    ::std::bitset<FlattenedSz> bitAdd;
    bitAdd.set(m_partitionSzLog);

    m_lowerRange = 0b0;
    for (::std::size_t i = 0; i < m_id; ++i)
    {
      m_lowerRange = bitsetAdd(m_lowerRange, bitAdd); 
    }

    m_upperRange = bitsetAdd(m_lowerRange, bitAdd).flip();
  }

public:
  KStateSpacePartition(void) = default;
  KStateSpacePartition(const ::std::size_t& id, 
    const ::std::size_t& k)
    : m_id(id),
      m_k(k)
  {
    initRange();
  }

  auto getRange(void)
  { return ::std::make_tuple(m_lowerRange, m_upperRange); }
};

#endif
