#include "move.h"
#include "constants.hpp"
#include "chess_utils.h"

#include <math.h>
#include <regex>

// Note: all of the error checks to make sure the format is correct can be ommited by a chess engine when it communicates with the gui, if it wants to.

// Since the the sequence a,b,..,z,aa,ab,...,zz has 26*27 elements
unsigned int DAModulus = 26*27;
// 1000 is nice number, and is big enough.
unsigned int DDModulus = 1000;

// ----------- Conversions for the letter lexeme of algebraic notation ----------- //
// a=0, ..., z=25, aa=26+0, ..., az=26+25, ba=2*26+0, ..., zz=26*26+25
// Note that upper case letters are prohibited
DAModInt lettersToInt(std::string _letters) {
    if (_letters.length() > 2) {
        dlog("ERROR: coords too big. " , WHERE);
        return -1;
    }
    unsigned int value = 0;

    if (_letters.length() == 2) {
        value += (_letters.at(0)-'a'+1) * 26;
        value += _letters.at(1)-'a';
    } else { // length=1
        value += _letters.at(0)-'a';
    }
    return value;
}
std::string intToLetters(DAModInt _int) {
    std::string letters = "";
    if (_int.m_value >= 26) {
        letters += (_int.m_value/26)+'a'-1;
    }
    letters += (_int.m_value%26)+'a';
    return letters;
}

std::string PieceMove::algebraic() const {
    return coordsToAlgebraic(m_startPos) + coordsToAlgebraic(m_endPos);
}
std::string TileMove::algebraic() const {
    std::string result = "S" + coordsToAlgebraic(m_selFirst) + coordsToAlgebraic(m_selSecond) + coordsToAlgebraic(m_destFirst);
    if (abs(m_symmetry) < 4) {
        result += "R" + std::to_string(abs(m_symmetry));
    }
    if (m_symmetry < 0) {
        result += "F";
    }
    return result;
}
std::string TileDeletion::algebraic() const {
    std::string result = "D";
    for (DModCoords c : m_deleteCoords) {
        result += coordsToAlgebraic(c);
    }
    return result;
}

bool isAlgebraic(std::string _algrebra) {
    // TODO:
    // Regex components:
    // match coords: [:alpha:]{1,3}[:d:]{1,3}
    // 
    const std::regex MOVE_ALGEBRA_REGEX("[SD]?");
    return false;
}

std::unique_ptr<Move> readAlgebraic(std::string _algebra) {
    // tdout << "readAlgebraic(" << _algebra << ")" << std::endl;
    AlgebraicTokenizer tokenizer(_algebra);

    if (isalpha(tokenizer.peek()[0]) && isupper(tokenizer.peek()[0])) { // got a capital prefix

        if (tokenizer.peek() == "S") { // this is a tile selection move
            tokenizer.next(); // eat the S
            DModCoords f = tokenizer.nextCoords(); DModCoords s = tokenizer.nextCoords(); DModCoords d = tokenizer.nextCoords();
            std::unique_ptr<TileMove> move = std::make_unique<TileMove>(f, s, d);
            if (tokenizer.hasNext()) { // Using symmetry modifier(s)
                if (tokenizer.peek() == "R") { // Rotation
                    // tdout << "has rotation" << std::endl;
                    tokenizer.next(); // eat the R
                    int numRotations = std::stoi(tokenizer.next());
                    numRotations %= 4;
                    if (numRotations == 0) numRotations = 4;
                    move->m_symmetry = numRotations;
                }
                if (tokenizer.hasNext()) {
                    if (tokenizer.peek() == "F") { // reFlection/Flip. Note we don't care if we eat the F here since it is last character.
                        // tdout << "has flip" << std::endl;
                        move->m_symmetry *= -1;
                    } else {
                        dlog("UNKNOWN SUFFIX '" , tokenizer.peek() , "'" , WHERE);
                    }
                }
            }
            return move;

        } else if (tokenizer.peek() == "D") { // this is a tile deletion move
            tokenizer.next(); // eat the D
            std::vector<DModCoords> deletions;
            while (tokenizer.hasNext()) {
                deletions.push_back(tokenizer.nextCoords());
            }
            return std::make_unique<TileDeletion>(deletions);

        } else {
            dlog("UNKNOWN PREFIX '" , tokenizer.peek() , "' " , WHERE);
        }
    }
    // If no prefix, then this is a piece move
    DModCoords f = tokenizer.nextCoords(); DModCoords s = tokenizer.nextCoords(); // We declare them before passing as params to make sure order of operations is OK
    return std::make_unique<PieceMove>(f, s);
}

// FIXME: now that these functions are exposed to user-input, it is critical that they failsafe.
std::string coordsToAlgebraic(DModCoords _coords, DModCoords _offset) {
    //TODO: implement _offset if its needed
    return intToLetters(_coords.file) + std::to_string(_coords.rank.m_value);
}
std::string signedCoordsToAlgebraic(SignedCoords _coords) {
    //TODO: implement _offset if its needed
    return (_coords.file  >= 0? "+" : "") + std::to_string(_coords.file) + 
           (_coords.rank >= 0? "+" : "") + std::to_string(_coords.rank);
}

DModCoords algebraicToCoords(std::string _algebra, DModCoords _offset) {
    //TODO: implement _offset if its needed
    //TODO: handle errors
    AlgebraicTokenizer tokenizer(_algebra);
    std::string letterPart = tokenizer.next();
    std::string numberPart = tokenizer.next();
    return DModCoords(
        lettersToInt(letterPart),
        std::stoi(numberPart)
    );
}

std::string matchHomogenous(std::istream& _stream, int (*_homoFunc)(int)) {
    std::string token = "";
    for(;;) {
        char lookahead = _stream.peek();

        if (!_homoFunc(lookahead)) {
            return token;
        }
        token += lookahead;
        _stream.ignore(); // pop character
    }
}

std::string AlgebraicTokenizer::next() {
    if (m_hasPeeked) {
        m_hasPeeked = false;
        return m_peeked;
    }

    char lookahead = m_stream.peek();
    if (isalpha(lookahead)) { 
        if (islower(lookahead)) { // lowercase letter is a file coord
            return matchHomogenous(m_stream, isalpha);
        }
        m_stream.ignore(); // pop character
        return std::string(1, lookahead);

    } else if (isdigit(lookahead)) { // digit is a rank coord
        return matchHomogenous(m_stream, isdigit);

    } else if (std::string("-+").find(lookahead) != std::string::npos) { // other permissible single-character lexemes
        m_stream.ignore(); // pop character
        return std::string(1, lookahead);

    } else {
        dlog(WHERE , "Error: Unknown character in algebraic string '" , lookahead , "'");
        return "[ERROR!]";
    }
}
DModCoords AlgebraicTokenizer::nextCoords() {
    //TODO: handle errors
    std::string letters = next();
    std::string numbers = next();
    return DModCoords(lettersToInt(letters), (unsigned int) std::stoi(numbers));
}
SignedCoords AlgebraicTokenizer::nextSignedCoords() {
    //TODO: handle errors
    std::string coordStr = next();
    coordStr += next();
    int firstCoords = std::stoi(coordStr);
    coordStr = next();
    coordStr += next();
    int secondCoords = std::stoi(coordStr);
    return SignedCoords(firstCoords, secondCoords);
}

