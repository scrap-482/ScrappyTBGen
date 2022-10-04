#ifndef COORDS_HPP
#define COORDS_HPP

#include <iostream>
#include "utils.h"

template <typename T, typename U>
class Coords {
    public:
        // Seems weird to call these m_..., considering we will commonly be 
        // calling smth like 'start.file' instead of 'start.m_file'

        // First param. Defines which column it is in, aka what position in the row it is.
        T file;
        // Second param. Defines which row it is on, aka what position in the column it is.
        U rank;
        // We could theoretically implement an applyEach here, but its not going to be pretty.

        Coords(T _file, U _rank) : file(_file), rank(_rank) { }
        // WARNING: default constructor does NOT guarantee zeroed values
        Coords() {};

        inline bool operator==(const Coords<T,U>& _rhs) const {
            return (file == _rhs.file && rank == _rhs.rank);
        }
        inline bool operator!=(const Coords<T,U>& _rhs) const {
            return !(*this == _rhs);
        }

        // Define element-wise arithmetic
        // WARNING: when adding Coords together, returned type is that of LHS.
        // TODO: nailed down what does/should happen when adding modInts with different mods.
        template <typename X, typename Y>
        Coords<T,U>& operator+=(const Coords<X,Y>& _rhs) {
            this->file += _rhs.file;
            this->rank += _rhs.rank;
            return *this;
        }

        template <typename X, typename Y>
        Coords<T,U> operator+(const Coords<X,Y>& _rhs) const {
            return Coords<T,U>(*this) += _rhs;
        }
        template <typename X, typename Y>
        Coords<T,U>& operator-=(const Coords<X,Y>& _rhs) {
            this->file -= _rhs.file;
            this->rank -= _rhs.rank;
            return *this;
        }
        template <typename X, typename Y>
        Coords<T,U> operator-(const Coords<X,Y>& _rhs) const {
            return Coords<T,U>(*this) -= _rhs;
        }

        // Define negation
        Coords<T,U> operator-() const {
            Coords<T,U> ret = Coords<T,U>(*this);
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

// template <typename T>
// class HomoCoords : public Coords<T,T> {
//     public:
//         void applyEach(void(*_function)(T&)) {
//             _function(this->file);
//             _function(this->rank);
//         }
//         HomoCoords(T _file, T _rank) : Coords<T,T>(_file, _rank) { }
//         // WARNING: default constructor does NOT guarantee zeroed values
//         HomoCoords() {};
// };

// template <typename T, typename U>
// T& GetFile (Coords<T, U> _coords) {
//     return _coords.file;
// }
// template <typename T, typename U>
// U& GetRank (Coords<T, U> _coords) {
//     return _coords.rank;
// }

template <typename T, typename U>
inline std::ostream& operator<<(std::ostream& _stream, const Coords<T,U>& _coords) {
    _stream << "(" << _coords.file << ", " << _coords.rank << ")";
    return _stream;
}

// Scalar multiplication on pairs
template <typename T, typename U, typename S>
Coords<T,U>& operator*=(Coords<T,U>& _pair, const S& _scalar) {
    _pair.file *= _scalar;
    _pair.rank *= _scalar;
    return _pair;
}
template <typename T, typename U, typename S>
Coords<T,U> operator*(Coords<T,U> _pair, const S& _scalar) {
    return _pair *= _scalar;
}
template <typename T, typename U, typename S>
Coords<T,U> operator*(const S& _scalar, Coords<T,U> _pair) {
    return _pair *= _scalar;
}

// ModInt methods that are nice to do pairwise
// FIXME: this gross syntax emphasizes why we need a custom pair class
template <typename T, typename U>
Coords<T,U> getDistTo(const Coords<T,U>& _lhs, const Coords<T,U>& _rhs) {
    return std::make_pair(_lhs.file.getDistTo(_rhs.file), _lhs.rank.getDistTo(_rhs.rank));
}

// Comparators for pairs //TODO: remove if unneeded
// Row order implies f0 < a8
template <typename T, typename U>
struct compareCoordsRowOrder {
    bool operator()(const Coords<T,U>& _a, const Coords<T,U>& _b) const {
        if (_a.rank != _b.rank) {
            return _a.rank < _b.rank;
        } else {
            return _a.file < _b.file;
        }
    }
};
// Column Order implies a8 < f0
template <typename T, typename U>
struct compareCoordsColOrder {
    bool operator()(const Coords<T,U>& _a, const Coords<T,U>& _b) const {
        if (_a.file != _b.file) {
            return _a.file < _b.file;
        } else {
            return _a.rank < _b.rank;
        }
    }
};
// Comparators for pairs of modular ints -- this is needed for storing them in maps
// Row order implies f0 < a8
template <typename T, typename U>
struct compareModCoordsRowOrder {
    bool operator()(const Coords<T,U>& _a, const Coords<T,U>& _b) const {
        if (_a.rank != _b.rank) {
            return _a.rank.m_value < _b.rank.m_value;
        } else {
            return _a.file.m_value < _b.file.m_value;
        }
    }
};

// Column Order implies a8 < f0
template <typename T, typename U>
struct compareModCoordsColOrder { // TODO: this feels like it goes with ModInt?
    bool operator()(const Coords<T,U>& _a, const Coords<T,U>& _b) const {
        if (_a.file != _b.file) {
            return _a.file.m_value < _b.file.m_value;
        } else {
            return _a.rank.m_value < _b.rank.m_value;
        }
    }
};


#endif