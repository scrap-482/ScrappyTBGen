#ifndef UTILITYMETHODS_H
#define UTILITYMETHODS_H

#include <string>
#include <iostream>
#include <sstream>
#include <ios>
#include <stdlib.h>
#include <vector>
#include <regex> //TODO: remove unnecessary includes

// TODO: might be good to wrap some of these functions in a namespace like "SCUtils::" or smth

/* ---------------------------- Debugging Macros ---------------------------- */
// Control where dlog is sent to
#ifdef DEBUG
    // Forward declarations required
    template <typename... Targs> void dlogStart(Targs... _args);
    template <typename T> void dlogMid(T _val);
    template <typename T, typename... Targs> void dlogMid(T _val, Targs... _args);
    template <typename... Targs> void dlogEnd(Targs... _args);

    extern std::stringstream dlogStream;
    // Engine's can't just write anything to cout, so we need to comment it first.

    // Debug log. Takes anynumber of arguments, and outputs them to the 
    // appropriate place with the appropriate format. Assumes it is being
    // called on the start of a new line. Outputs newline at the end automatically.
    template <typename... Targs> 
    void dlog(Targs... _args) {
        std::cout << "# ";
        dlogMid(_args...);
        std::cout << std::endl;
    }

/* - If you have to output on the same line with different calls, use these - */

    // Alternative dlog that does not output newline at end. Make sure to follow 
    // this up with a dlogMid or dlogEnd to ensure newline is added.
    template <typename... Targs> 
    void dlogStart(Targs... _args) {
        std::cout << "# ";
        dlogMid(_args...);
    }
    // Alternative dlog that does not output # at start. Make sure this is between
    // A dlogStart/dlogMid and a dlogEnd/dlogMid.
    template <typename T> 
    void dlogMid(T _val) {
        dlogStream << _val;
        std::cout << std::regex_replace(dlogStream.str() , std::regex("\n"), "\n# ") << std::flush;
        dlogStream.str(""); // clear the stream
    }
    // Alternative dlog that does not output # at start. Make sure this is between
    // A dlogStart/dlogMid and a dlogEnd/dlogMid.
    template <typename T, typename... Targs> 
    void dlogMid(T _val, Targs... _args) {
        dlogStream << _val;
        dlogMid(_args...);
    }
    // Alternative dlog that does not output # at start. Make sure this is preceeded
    // by a dlogStart or dlogMid to ensure # is added.
    template <typename... Targs> 
    void dlogEnd(Targs... _args) {
        dlogMid(_args...);
        std::cout << std::endl;
    }
/* -------------------------------------------------------------------------- */
#else
    // Set these to noops
    #define dlog(...) (void*)0
    #define dlogStart(...) (void*)0
    #define dlogMid(...) (void*)0
    #define dlogEnd(...) (void*)0
#endif

// When you need a comma within a parameter of a preprocessor macro (that is not expanded again)
// e.g. std::pair<int COMMA int>
#define COMMA ,

// USAGE: FORMAT(foo << bar << ... << baz) returns a std::string
// This macro lets us make std::string analogous to
// std::cout << foo << bar << baz
// Credit to Mr.Ree https://stackoverflow.com/questions/303562/c-format-macro-inline-ostringstream
#define FORMAT(ITEMS) \
    ( ( dynamic_cast<std::ostringstream &> ( \
        std::ostringstream() . std::ostream::seekp( 0, std::ios_base::cur ) << ITEMS ) \
    ) . str() )

// USAGE: returns a string describing where in the source code this macro is.
#define WHERE (FORMAT("\tFile: " << __FILE__ << "\n\tLine: " << __LINE__ << std::endl))

/* ---------------------------------- Misc ---------------------------------- */

std::vector<std::string> split(const std::string str, const std::string regex_str);

// Guarantees positive result congruent to i % n, except in case of an overflow.
// See this post for more details: https://stackoverflow.com/questions/14997165/fastest-way-to-get-a-positive-modulo-in-c-c
inline int positiveModulo(int i, int n) {
    return (i % n + n) % n;
}

// Division of a / b that always rounds towards negative infinity, e.g. -12 / 10 = -2
// from https://stackoverflow.com/questions/39304681/division-with-negative-dividend-but-rounded-towards-negative-infinity
inline int divFloor(int a, int b) {
    int res = a / b;
    int rem = a % b;
    // Correct division result downwards if up-rounding happened,
    // (for non-zero remainder of sign different than the divisor).
    int corr = (rem != 0 && ((rem < 0) != (b < 0)));
    return res - corr;
}

template <typename T, typename U>
inline std::ostream& operator<<(std::ostream& _stream, const std::pair<T,U>& _coords) {
    _stream << "(" << _coords.first << ", " << _coords.second << ")";
    return _stream;
}

// // Define element-wise arithmetic on std::pairs. // TODO: deprecated since coords.hpp, but keeping around for a bit just in case
// template <typename T, typename U, typename X, typename Y>
// std::pair<T,U>& operator+=(std::pair<T,U>& _lhs, const std::pair<X,Y>& _rhs) {
//     _lhs.first += _rhs.first;
//     _lhs.second += _rhs.second;
//     return _lhs;
// }
// template <typename T, typename U, typename X, typename Y>
// std::pair<T,U> operator+(std::pair<T,U> _lhs, const std::pair<X,Y>& _rhs) {
//     return _lhs += _rhs;
// }
// template <typename T, typename U, typename X, typename Y>
// std::pair<T,U>& operator-=(std::pair<T,U>& _lhs, const std::pair<X,Y>& _rhs) {
//     _lhs.first -= _rhs.first;
//     _lhs.second -= _rhs.second;
//     return _lhs;
// }
// template <typename T, typename U, typename X, typename Y>
// std::pair<T,U> operator-(std::pair<T,U> _lhs, const std::pair<X,Y>& _rhs) {
//     return _lhs -= _rhs;
// }

#endif
