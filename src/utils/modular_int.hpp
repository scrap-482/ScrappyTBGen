/*
* Copyright 2022 SCRAP
*
* This file is part of Scrappy Tablebase Generator.
* 
* Scrappy Tablebase Generator is free software: you can redistribute it and/or modify it under the terms 
* of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, 
* or (at your option) any later version.
* 
* Scrappy Tablebase Generator is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of 
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along with Scrappy Tablebase Generator. If not, see <https://www.gnu.org/licenses/>.
*/


#ifndef MODULAR_INT_HPP
#define MODULAR_INT_HPP

#include <iostream>
#include "utils.h"

// Arithmetic operations on objects of this class are done with a modulo, and always return type ModularInt.
// m_value will always be a value in {0, 1, ..., modulus-1}.
// To get back to normal arithmetic, get the m_value.
// Templated by a pointer to the modulus. This means each class created from template has a modulus shared by all its objects, 
// and that modulus can be modified.
// The behavior for a ModularInt object is not defined when modulus changes during its existence.
template <unsigned int* modulus>
class ModularInt {
    public:
        // Value stored by a specific object
        int m_value; // Even though this only contains nonnegative values, we keep it signed so it doesn't overflow on negative numbers.

        ModularInt(int _x);
        ModularInt() : m_value(0) { } // TODO: consider renaming to value instead of m_value?

        ModularInt& operator+=(int _x) {
            m_value += _x;
            // gcc complains if I don't put this-> in front of these for some obscure reason 
            //https://stackoverflow.com/questions/9941987/there-are-no-arguments-that-depend-on-a-template-parameter
            m_value = positiveModulo(m_value, *modulus); 
            return *this;
        }
        ModularInt& operator-=(int _x) {
            m_value -= _x;
            m_value = positiveModulo(m_value, *modulus);
            return *this;
        }
        ModularInt& operator+=(const ModularInt _x) {
            m_value += _x.m_value;
            m_value = positiveModulo(m_value, *modulus);
            return *this;
        }
        ModularInt& operator-=(const ModularInt _x) {
            m_value -= _x.m_value;
            m_value = positiveModulo(m_value, *modulus);
            return *this;
        }
        ModularInt& operator++() {
            m_value = positiveModulo(m_value + 1, *modulus);
            return *this;
        }
        ModularInt& operator--() {
            m_value = positiveModulo(m_value - 1, *modulus);
            return *this;
        }
        // TODO: maybe add postfix operators?

        bool operator==(const ModularInt& _other) const {
            return m_value == _other.m_value;
        }
        bool operator!=(const ModularInt& _other) const {
            return m_value != _other.m_value;
        }

        // Return true if this is between _lower and _upper, inclusive;
        // if _excludeUpper=true, then returns true if this is between [_lower, _upper) instead.
        // i.e. starting at _lower and traveling positive, you will hit this before you hit _upper.
        // Edge cases: 
        //      x.isBetween(x,x) = true       x.isBetween(x,x, true) = true 
        //      y.isBetween(x,x) = false      y.isBetween(x,x, true) = true
        //      y.isBetween(x, y) = true      y.isBetween(x, y, true) = false
        bool isBetween(ModularInt _lower, ModularInt _upper, bool _excludeUpper=false) const;
        // Compares which int is more to the left or down from another, based on where the wrap-around happens (namely, at _relZero)
        // returns if this < _other.
        // NOTE: Is the same as isBetween(_relZero, _other, true), except for the poorly defined half-closed interval [x, x),
        // so try to use isBetween unless you need to handle that edge.
        // Edge cases: 
        //      x.lessThan(x,x) = false
        //      y.lessThan(x,x) = false
        //      y.lessThan(y,x) = false
        bool lessThan(ModularInt _other, ModularInt _relZero) const;

        // Heuristic Less Than. Maybe better name would be 'shortSideLessThan' or 'noWrapLessThan'.
        // when _lhs and _rhs are close together, we can ignore the wrap-around and perform comparisons, i.e. less than.
        // Note that the behavior of this function is undefined when the difference between _lhs and _rhs is close to ±1/2 the modulus
        bool heurLessThan(ModularInt _rhs) const;

        bool heurLessThanOrEqual(ModularInt _rhs) const;

        // Gets distance from *this to _target as a signed number, ignoring wrap around (similar to heurLessThan).
        // E.g. if _target+5 == *this, and modulus ≫ 5, then -5 is returned.
        // Note that the behavior of this function is undefined when the difference between _start and _end is close to ±1/2 the modulus
        int getDistTo(const ModularInt& _target) const;

        friend std::ostream& operator<<(std::ostream& _os, const ModularInt<modulus>& _mi) {
            _os << _mi.m_value; // << " (mod " << *modulus << ")";
            return _os;
        }
};


//-------------- Implementations below ---------------//
template <unsigned int* modulus>
inline ModularInt<modulus>::ModularInt(int _x) {
    m_value = positiveModulo(_x, *modulus);
}

// Addition with ints
template <unsigned int* modulus>
inline ModularInt<modulus> operator+(ModularInt<modulus> _mi, int _x) {
    _mi += _x;
    return _mi;
}
template <unsigned int* modulus>
inline ModularInt<modulus> operator+(int _x, ModularInt<modulus> _mi) {
    return _mi + _x;
}
// Addition with other
template <unsigned int* modulus>
inline ModularInt<modulus> operator+(ModularInt<modulus> _mi1, ModularInt<modulus> _mi2) {
    _mi1 += _mi2;
    return _mi1;
}
// Nice shorthand for adding negative ints
template <unsigned int* modulus>
inline ModularInt<modulus> operator-(ModularInt<modulus> _mi, int _x) {
    _mi -= _x;
    return _mi;
}
// Negation
template <unsigned int* modulus>
inline ModularInt<modulus> operator-(ModularInt<modulus> _mi) {
    return ModularInt<modulus>(-_mi.m_value);
}
template <unsigned int* modulus>
inline ModularInt<modulus> operator-(int _x, ModularInt<modulus> _mi) {
    return _x + (-_mi);
}
// Subtraction with other
template <unsigned int* modulus>
inline ModularInt<modulus> operator-(ModularInt<modulus> _mi1, ModularInt<modulus> _mi2) {
    _mi1 -= _mi2;
    return _mi1;
}

template <unsigned int* modulus>
inline bool ModularInt<modulus>::isBetween(ModularInt<modulus> _lower, ModularInt<modulus> _upper, bool _excludeUpper) const {
    auto actualUpper = (_excludeUpper? _upper-1 : _upper); // since integers, [a,b) = [a, b-1]
    return (*this - _lower).m_value <= (actualUpper - _lower).m_value;
}
template <unsigned int* modulus>
inline bool ModularInt<modulus>::lessThan(ModularInt<modulus> _other, ModularInt<modulus> _relZero) const {
    return (*this - _relZero).m_value < (_other - _relZero).m_value;
}

template <unsigned int* modulus>
inline bool ModularInt<modulus>::heurLessThan(ModularInt _rhs) const {
    // *this - _rhs gives the distance you have to travel in the positive direction to start from _rhs and end on *this.
    // If the distance is greater than half of the space, that means traveling in the negative direction is shorter, so we assume it would wrap around if we took this long way.
    // Since we can't wrap around, this means traveling in the positive direction from _rhs won't end on *this, so _rhs is greater.
    // Therefore, if *this - _rhs > half of space, then *this < _rhs

    // Basically like how a negative number converted to an unsigned number is big.
    return (*this - _rhs).m_value > (*modulus) / 2;
}

template <unsigned int* modulus>
inline bool ModularInt<modulus>::heurLessThanOrEqual(ModularInt _rhs) const {
    // Basically like how a negative number converted to an unsigned number is big and a positive number stays small,
    // (_rhs - *this) is "big" (i.e. > half of space) when *this > _rhs.
    // The inverse of this is *this <= _rhs, which entails (_rhs - *this) is small (< half of space)
    // heur< and heur<= differ because checking if 'x-y is small' or 'y-x is large' are the same check when x!=y, x-x=0 is small always,
    // So whether we check x-y=small or y-x=large affects if 0 is included. // TODO: Make this documentation more concise
    return (_rhs - *this).m_value < (*modulus) / 2;
}

template <unsigned int* modulus>
inline int ModularInt<modulus>::getDistTo(const ModularInt& _target) const {
    if (_target.heurLessThan(*this)) {
        return -((*this - _target).m_value); // Avoids wrap-around by negating after converted to int.
    } else {
        return (_target - *this).m_value;
    }
}


#endif