// Defines a template for grid coordinates. The row and file types must have equality, addition, subtraction, and 
// negation with itself defined. Optionally, scalar multiplication can be implemented.
#ifndef COORDS_GRID_HPP
#define COORDS_GRID_HPP

#include "../utils/utils.h"
#include <iostream>

template <typename T, typename U>
class CoordsGrid {
    public:
        // First param. Defines which column it is in, aka what position in the row it is.
        T file;
        // Second param. Defines which row it is on, aka what position in the column it is.
        U rank;

        CoordsGrid(T _file, U _rank) : file(_file), rank(_rank) { }
        // WARNING: default constructor does NOT guarantee zeroed values
        CoordsGrid() {};

        inline bool operator==(const CoordsGrid<T,U>& _rhs) const {
            return (file == _rhs.file && rank == _rhs.rank);
        }
        inline bool operator!=(const CoordsGrid<T,U>& _rhs) const {
            return !(*this == _rhs);
        }

        // Define element-wise arithmetic
        // WARNING: when adding CoordsGrid together, returned type is that of LHS.
        // TODO: nailed down what does/should happen when adding modInts with different mods.
        template <typename X, typename Y>
        CoordsGrid<T,U>& operator+=(const CoordsGrid<X,Y>& _rhs) {
            this->file += _rhs.file;
            this->rank += _rhs.rank;
            return *this;
        }

        template <typename X, typename Y>
        CoordsGrid<T,U> operator+(const CoordsGrid<X,Y>& _rhs) const {
            return CoordsGrid<T,U>(*this) += _rhs;
        }
        template <typename X, typename Y>
        CoordsGrid<T,U>& operator-=(const CoordsGrid<X,Y>& _rhs) {
            this->file -= _rhs.file;
            this->rank -= _rhs.rank;
            return *this;
        }
        template <typename X, typename Y>
        CoordsGrid<T,U> operator-(const CoordsGrid<X,Y>& _rhs) const {
            return CoordsGrid<T,U>(*this) -= _rhs;
        }

        // Define negation
        CoordsGrid<T,U> operator-() const {
            CoordsGrid<T,U> ret = CoordsGrid<T,U>(*this);
            ret.file = -this->file;
            ret.rank = -this->rank;
            return ret;
        }
};

template <typename T, typename U>
inline std::ostream& operator<<(std::ostream& _stream, const CoordsGrid<T,U>& _coords) {
    _stream << "(" << _coords.file << ", " << _coords.rank << ")";
    return _stream;
}

// Scalar multiplication on pairs
template <typename T, typename U, typename S>
CoordsGrid<T,U>& operator*=(CoordsGrid<T,U>& _pair, const S& _scalar) {
    _pair.file *= _scalar;
    _pair.rank *= _scalar;
    return _pair;
}
template <typename T, typename U, typename S>
CoordsGrid<T,U> operator*(CoordsGrid<T,U> _pair, const S& _scalar) {
    return _pair *= _scalar;
}
template <typename T, typename U, typename S>
CoordsGrid<T,U> operator*(const S& _scalar, CoordsGrid<T,U> _pair) {
    return _pair *= _scalar;
}

#endif