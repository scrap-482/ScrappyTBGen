// Defines a template for grid coordinates. The row and file types must have equality, addition, subtraction, and 
// negation with itself defined. Optionally, scalar multiplication can be implemented.
#ifndef COORDS_GRID_HPP
#define COORDS_GRID_HPP

#include "../utils/utils.h"
#include <iostream>

// Any coords class must define a bijective map to size_t.
// You MUST implement a constructor with signature AbstractCoorCoordsGrid<T,U>ds(size_t flattened)
class AbstractCoords {
public: 
    virtual size_t flatten() const = 0;
};

//Shorthand:
// BW = BoardWidth

template <typename T, typename U, ::std::size_t BW>
class CoordsGrid : AbstractCoords {
    public:
        // First param. Defines which column it is in, aka what position in the row it is.
        T file;
        // Second param. Defines which row it is on, aka what position in the column it is.
        U rank;

        CoordsGrid(T _file, U _rank) : file(_file), rank(_rank) { }
        // Unflattening ctor
        CoordsGrid(size_t flattened) : file(flattened % BW), rank(flattened / BW) { }
        // WARNING: default constructor does NOT guarantee zeroed values
        CoordsGrid() {};

        size_t flatten() const override {
            // ASSUMPTION: T and U can be safely cast to size_t and this 
            return file + rank * BW;
        }

        inline bool operator==(const CoordsGrid<T,U,BW>& _rhs) const {
            return (file == _rhs.file && rank == _rhs.rank);
        }
        inline bool operator!=(const CoordsGrid<T,U,BW>& _rhs) const {
            return !(*this == _rhs);
        }

        // Define element-wise arithmetic
        // WARNING: when adding CoordsGrid together, returned type is that of LHS.
        // TODO: nailed down what does/should happen when adding modInts with different mods.
        template <typename X, typename Y, ::std::size_t Z>
        CoordsGrid<T,U,BW>& operator+=(const CoordsGrid<X,Y,Z>& _rhs) {
            this->file += _rhs.file;
            this->rank += _rhs.rank;
            return *this;
        }

        template <typename X, typename Y, ::std::size_t Z>
        CoordsGrid<T,U,BW> operator+(const CoordsGrid<X,Y,Z>& _rhs) const {
            return CoordsGrid<T,U,BW>(*this) += _rhs;
        }
        template <typename X, typename Y, ::std::size_t Z>
        CoordsGrid<T,U,BW>& operator-=(const CoordsGrid<X,Y,Z>& _rhs) {
            this->file -= _rhs.file;
            this->rank -= _rhs.rank;
            return *this;
        }
        template <typename X, typename Y, ::std::size_t Z>
        CoordsGrid<T,U,BW> operator-(const CoordsGrid<X,Y,Z>& _rhs) const {
            return CoordsGrid<T,U,BW>(*this) -= _rhs;
        }

        // Define negation
        CoordsGrid<T,U,BW> operator-() const {
            CoordsGrid<T,U,BW> ret = CoordsGrid<T,U,BW>(*this);
            ret.file = -this->file;
            ret.rank = -this->rank;
            return ret;
        }
};

template <typename T, typename U, ::std::size_t BW>
inline std::ostream& operator<<(std::ostream& _stream, const CoordsGrid<T,U,BW>& _coords) {
    _stream << "(" << _coords.file << ", " << _coords.rank << ")";
    return _stream;
}

// Scalar multiplication on pairs
template <typename T, typename U, ::std::size_t BW, typename S>
CoordsGrid<T,U,BW>& operator*=(CoordsGrid<T,U,BW>& _pair, const S& _scalar) {
    _pair.file *= _scalar;
    _pair.rank *= _scalar;
    return _pair;
}
template <typename T, typename U, ::std::size_t BW, typename S>
CoordsGrid<T,U,BW> operator*(CoordsGrid<T,U,BW> _pair, const S& _scalar) {
    return _pair *= _scalar;
}
template <typename T, typename U, ::std::size_t BW, typename S>
CoordsGrid<T,U,BW> operator*(const S& _scalar, CoordsGrid<T,U,BW> _pair) {
    return _pair *= _scalar;
}

#endif