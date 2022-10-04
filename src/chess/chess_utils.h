#ifndef CHESS_UTILITYMETHODS_H
#define CHESS_UTILITYMETHODS_H

#include "constants.hpp"
#include "../utils/utils.h"

#include <string>
#include <iostream>
#include <sstream>
#include <ios>
#include <stdlib.h>
#include <vector>
#include <regex>

// TODO: might be good to wrap some of these functions in a namespace like "SCUtils::" or smth

// ----------- SquareEnum functions ----------- //
/**
 * Gets letter respresentation of square type enum
 * Optionally set empty to desired character representation of EMPTY and VOID
 */
char getCharFromSquare(SquareEnum _enumValue, char _empty=TILE_LETTERS[EMPTY], char _invalid=TILE_LETTERS[VOID]);
/**
 * Gets square enum from letter
 * Optionally set empty to desired character representation of EMPTY
 * Returns VOID for unknown characters.
 */
SquareEnum getSquareFromChar(char _char, char _empty=TILE_LETTERS[EMPTY]);

/**
 * Gets unicode chess piece respresentation of square type enum
 * OPTIONALLY set empty to desired character representation of EMPTY, 
 */
std::string getUnicodeCharFromSquare(SquareEnum _enumValue, std::string _empty=" ");

/** Is this a piece enum? */
inline bool isPiece(SquareEnum _squareEnum) {
    return _squareEnum != EMPTY && _squareEnum != VOID; 
}
/** Assuming a valid piece enum, is the piece white or black? */
inline bool isWhite(SquareEnum _piece) {
    return _piece & 1u; // Gets the LSB
}
/** Assuming a valid nonempty piece enum, is the piece white or black? */
inline bool isBlack(SquareEnum _piece) {
    return !isWhite(_piece); // Assumes black pieces are even enums
}

// ----------- DirectionEnum functions ----------- //

/**
 * Gets letter respresentation of direction
 */
char getCharFromDirection(DirectionEnum _dir);

/**
 * LEFT to RIGHT, etc.
 */
DirectionEnum flipDirection(DirectionEnum _dir);

//----------- Here are some function useful for coordinate wrapping ----------//

// Compares which coords is more to the left or down from another coord, based on where the wrap-around happens (namely, at _relZero)
// returns if _lhs < _rhs
// TODO: consider merging with modularInt's methods
// It seems this is only used by DLLBoard, so i'd consider this to be deprecated
inline bool coordLessThan(unsigned int _lhs, unsigned int _rhs, unsigned int _relZero) {
    return (_lhs - _relZero) < (_rhs - _relZero);
}
inline bool coordGreaterThan(unsigned int _lhs, unsigned int _rhs, unsigned int _relZero) {
    return (_lhs - _relZero) > (_rhs - _relZero);
}

// Gets the distance from one coord to another, without crossing the wrap around point _relZero
inline unsigned int coordDistance(unsigned int _1, unsigned int _2, unsigned int _relZero) {
    if (coordLessThan(_1, _2, _relZero)) {
        return _2 - _1;
    } else {
        return _1 - _2;
    }
}

// Reads the _sfen, executing the lambda functions _forX whenever X is encountered.
// Those lambdas expect the following parameters:
//      _forPiece(SquareEnum _piece)
//      _forVoid(int _numVoid)
//      _forEmpty(int _numEmpty)
//      _forNewline()
template<typename T1, typename T2, typename T3, typename T4>
void parseSfenPos(
    std::string _sfen,
    T1 _forPiece,
    T2 _forVoid,
    T3 _forEmpty,
    T4 _forNewline
) {
    for (int i = 0; i < _sfen.length() && _sfen[i] != ' '; i++) {
        char c = _sfen[i];
        if (c == '/') { // Next row
            _forNewline();
            continue;
        }
        if (c == '(') { // Void square(s)
            int j;
            // set j to be the next non-numeric character
            for (j = i+1; isdigit(_sfen[j]); j++);
            // If it is the edge case where there is no numbers, i.e. "()", we can skip this part
            if (j != i+1) {
                // Get the next characters as integer
                int numVoidTiles = std::stoi(_sfen.substr(i+1, j-(i+1))); //i+1 to ignore '('
                _forVoid(numVoidTiles);
            }
            // update i to to account for the number of additional characters we read in
            i = j;
            continue;
        }
        if (isdigit(c)) { // Empty tile(s)
            int j;
            // set j to be the next non-numeric character
            for (j = i+1; isdigit(_sfen[j]); j++);
            // Get the next characters as integer
            int numEmptyTiles = std::stoi(_sfen.substr(i, j-i));
            // update i to to account for the number of additional characters we read in
            i = j-1;
            _forEmpty(numEmptyTiles);
            continue;
        }

        SquareEnum thisTile = getSquareFromChar(c, ' '); // We look for empty as ' ' to ensure we never find empty this way, just in case.
        if (thisTile == VOID) {
            //FIXME: error handling
            dlog("Error parsing sfen position: unknown character '", c, "'");
            continue;
        }
        _forPiece(thisTile);
    }
}


#endif
