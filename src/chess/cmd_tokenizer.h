#ifndef CMD_TOKENIZER_H
#define CMD_TOKENIZER_H

#include "../utils/tokenizer.h"
#include "constants.hpp"

// Tokenizer for parsing commands sent to GUI from engine
class CmdTokenizer : public StringTokenizer {
    public:
        CmdTokenizer (std::string _string) : StringTokenizer(_string) {};
        // Returns the next token and increments
        std::string next();
        // Assuming next token should be a SFEN string, return that.
        // This is because xboard does not quote SFEN strings, and a lot of gross stuff might happen with all the spaces in it.
        std::string nextSfen();

    private:
        inline bool isWhitespaceSansNewline(char _c);
        inline void eatWhitespace(std::istream& _stream);
        bool isReservedLexemeChar(char _c);
        std::string matchQuotedToken(std::istream& _stream, char _delim, bool _includeDelim=true);
        std::string matchWordToken(std::istream& _stream);
};

// TODO: probably provide another tokenizer for commands from engine to GUI here. 
// It'll share much of the same code, so we can probably just put it in this file.

#endif