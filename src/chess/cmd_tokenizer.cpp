#include "cmd_tokenizer.h"
#include "constants.hpp"
#include "chess_utils.h"

// Note: since CmdTokenizer is only being called on strings created from std::getline(), 
// then anything here relating to '\n' is unecessary complexity.
// TODO: if this newline complexity is not used in the future, 
// we should probably remove it.

inline bool CmdTokenizer::isWhitespaceSansNewline(char _c) {
    return (iswspace(_c) && _c != '\n');
}

inline void CmdTokenizer::eatWhitespace(std::istream& _stream) {
  while(isWhitespaceSansNewline(_stream.peek())){ 
    _stream.ignore();
  }
}

// xboard word lexemes cannot contain these characters
bool CmdTokenizer::isReservedLexemeChar(char _c) {
    const std::string RESERVED = "(){}\"=#\n";
    return std::string::npos != RESERVED.find(_c);
}

std::string CmdTokenizer::matchQuotedToken(std::istream& _stream, char _delim, bool _includeDelim) {
    char firstDelim = _stream.get();
    std::string token(1, firstDelim);
    for(;;) {
        char lookahead = _stream.peek();

        if (!_includeDelim && lookahead == _delim) {
            return token;
        }
        if (lookahead == '\n' || lookahead == EOF) {
            dlog("Error with command: unmatched " , _delim);
            return "[ERROR]"; // TODO:
        }
        token += lookahead;
        _stream.ignore(); // pop character

        if (lookahead == _delim) {
            return token;
        }
    }
}

std::string CmdTokenizer::matchWordToken(std::istream& _stream) {
    std::string token = "";
    for(;;) {
        char lookahead = _stream.peek();
        if (iswspace(lookahead) || isReservedLexemeChar(lookahead) || lookahead == EOF) {
            return token;
        }
        token += lookahead;
        _stream.ignore(); // pop character
    }
}

std::string CmdTokenizer::next() {
    if (m_hasPeeked) {
        m_hasPeeked = false;
        return m_peeked;
    }
    eatWhitespace(m_stream); // eat leading whitespace

    char lookahead = m_stream.peek();

    // one-character lexemes
    if (lookahead == EOF || lookahead == ':' || lookahead == '\n' || lookahead == '=') {
        m_stream.ignore(); // pop character
        return std::string(1, lookahead);

    // quoted lexemes
    } else if (lookahead == '"') {
        return matchQuotedToken(m_stream, '"');
    } else if (lookahead == '(') {
        return matchQuotedToken(m_stream, ')');
    } else if (lookahead == '{') {
        return matchQuotedToken(m_stream, '}');
    } else if (lookahead == '#') {
        return matchQuotedToken(m_stream, EOF, false); //TODO: assumes \n never appears in string. This is ok assumption for now, but might be dangerous later. See Note above.

    // word lexemes aka miscellaneous
    } else {
        return matchWordToken(m_stream);
    }
}

std::string CmdTokenizer::nextSfen() {
    // TODO: we can do something similar to xboard by taking a piece-to-char table and maybe variant info? Probably better to just use the rules file tbf
    // First part of SFEN is always the board position
    std::string fullSfen = next();
    // get remaining fields. //TODO: handle how many fields we need. Maybe even consider specifying Sfen fields in the .rules file?
    const int NUM_SFEN_FIELDS = 3;
    for (int i = 0; i < NUM_SFEN_FIELDS; i++) {
        std::string nextField = next();
        if (nextField == "") {
            // TODO: handle partial info given? Or just reject somehow?
            return fullSfen;
        }
        fullSfen += " " + nextField;
    }
    // tdout << "Got Sfen token as '" << fullSfen << std::endl;
    return fullSfen;
}