/* -------------------------------------------------------------------------- */
/* --------------------------- Design Description: -------------------------- */
// Coords is the object used throughout this program to describe a spacial 
// position. This class gives abstract ways to represent Coords, and as such may
// not be all used. Coords is defined at the bottom of this file.
/* -------------------------------------------------------------------------- */

#ifndef COORDS_HPP
#define COORDS_HPP

#include <iostream>
#include "utils.h"

template <typename T, typename U>
class Coords2D {
    public:
        // Seems weird to call these m_..., considering we will commonly be 
        // calling smth like 'start.file' instead of 'start.m_file'

        // First param. Defines which column it is in, aka what position in the row it is.
        T file;
        // Second param. Defines which row it is on, aka what position in the column it is.
        U rank;
        // We could theoretically implement an applyEach here, but its not going to be pretty.

        Coords2D(T _file, U _rank) : file(_file), rank(_rank) { }
        // WARNING: default constructor does NOT guarantee zeroed values
        Coords2D() {};

        inline bool operator==(const Coords2D<T,U>& _rhs) const {
            return (file == _rhs.file && rank == _rhs.rank);
        }
        inline bool operator!=(const Coords2D<T,U>& _rhs) const {
            return !(*this == _rhs);
        }

        // Define element-wise arithmetic
        // WARNING: when adding Coords2D together, returned type is that of LHS.
        // TODO: nailed down what does/should happen when adding modInts with different mods.
        template <typename X, typename Y>
        Coords2D<T,U>& operator+=(const Coords2D<X,Y>& _rhs) {
            this->file += _rhs.file;
            this->rank += _rhs.rank;
            return *this;
        }

        template <typename X, typename Y>
        Coords2D<T,U> operator+(const Coords2D<X,Y>& _rhs) const {
            return Coords2D<T,U>(*this) += _rhs;
        }
        template <typename X, typename Y>
        Coords2D<T,U>& operator-=(const Coords2D<X,Y>& _rhs) {
            this->file -= _rhs.file;
            this->rank -= _rhs.rank;
            return *this;
        }
        template <typename X, typename Y>
        Coords2D<T,U> operator-(const Coords2D<X,Y>& _rhs) const {
            return Coords2D<T,U>(*this) -= _rhs;
        }

        // Define negation
        Coords2D<T,U> operator-() const {
            Coords2D<T,U> ret = Coords2D<T,U>(*this);
            ret.file = -this->file;
            ret.rank = -this->rank;
            return ret;
        }

        // TODO: getting this element-wise functions to work is probably not worth it,
        // but leaving the stuff commented out here in case I have to use it later.
        // void applyEach(void(*_function)(T&)) {
        //     _function(this->file);
        //     _function(this->rank);
        // }
        // Index, especially useful if we want to apply function to both file and rank via loop
        
};

template <typename T, typename U>
inline std::ostream& operator<<(std::ostream& _stream, const Coords2D<T,U>& _coords) {
    _stream << "(" << _coords.file << ", " << _coords.rank << ")";
    return _stream;
}

// Scalar multiplication on pairs
template <typename T, typename U, typename S>
Coords2D<T,U>& operator*=(Coords2D<T,U>& _pair, const S& _scalar) {
    _pair.file *= _scalar;
    _pair.rank *= _scalar;
    return _pair;
}
template <typename T, typename U, typename S>
Coords2D<T,U> operator*(Coords2D<T,U> _pair, const S& _scalar) {
    return _pair *= _scalar;
}
template <typename T, typename U, typename S>
Coords2D<T,U> operator*(const S& _scalar, Coords2D<T,U> _pair) {
    return _pair *= _scalar;
}

// ModInt methods that are nice to do pairwise
// FIXME: this gross syntax emphasizes why we need a custom pair class
template <typename T, typename U>
Coords2D<T,U> getDistTo(const Coords2D<T,U>& _lhs, const Coords2D<T,U>& _rhs) {
    return std::make_pair(_lhs.file.getDistTo(_rhs.file), _lhs.rank.getDistTo(_rhs.rank));
}

/* ----------------------- Actual Coord implementation ---------------------- */
typedef Coords2D<int, int> Coords;

#endif