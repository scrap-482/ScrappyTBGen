#include "ruleset.h"
#include "chess_utils.h"

#include <sstream>
#include <string>
#include <fstream>
#include <map>

Ruleset::Ruleset(std::string _ruleFile) {
    // initialize pmo list
    m_pieceMoveOptionLists.resize(NUM_PIECE_TYPES_BY_COLOR + 1);
    init(_ruleFile);
}

void Ruleset::match(std::string _given, std::string _expected) {
    if (_given != _expected) {
        dlog(WHERE , "Error: Expected '" , _expected , "' in rules file, found '" , _given , "'");
    }
}

// Parses the properties of a MoveOption, from '{' to '}'.
void Ruleset::updateMoveOptionProperties(RulesetTokenizer& _tokenizer, MoveOptionProperties& _mop) {
    match(_tokenizer.next(), "{");
    for (;;) {
        std::string property = _tokenizer.next();
        if (property == "}") { // check if we've reached end of properties list
            return;
        }
        // otherwise, we expect a property assignment
        match(_tokenizer.next(), "="); // property name always followed by =, so may as well match it here.

        if (property == "forwardOnly") {
            _mop.m_forwardOnly = _tokenizer.nextBool();
        } else if (property == "captureMode") {
            _mop.m_captureMode = _tokenizer.nextInt();
        } else if (property == "flyOverGaps") {
            _mop.m_flyOverGaps = _tokenizer.nextBool();
        } else if (property == "flyOverPieces") {
            _mop.m_flyOverPieces = _tokenizer.nextBool();
        } else {
            dlog(WHERE , "Error: Unknown move option property '" , property , "' in rules file"); //TODO: would be nice if error reported line number
        }
    }
}

// Parses the pieceMoveOptions, from '{' to '}'.
void Ruleset::addPieceMoveOptions(RulesetTokenizer& _tokenizer, std::vector<std::vector<std::shared_ptr<MoveOption>>>& _allPmoLists) {
    // tdout << "parsing PMOs..." << std::endl;
    match(_tokenizer.next(), "{");
    MoveOptionProperties defaultMop;
    for (;;) {
        std::string targetPiece = _tokenizer.next();
        // tdout << "targetPiece = [" << targetPiece << "]" << std::endl;
        if (targetPiece == "defaults") {
            updateMoveOptionProperties(_tokenizer, defaultMop);

        // Check this is a valid piece name
        } else if (targetPiece.length() == 1 && isPiece(getSquareFromChar(targetPiece[0]))) {
            SquareEnum pieceEnum = getSquareFromChar(toupper(targetPiece[0]));
            // tdout << "got pieceEnum as [" << getCharFromSquare(pieceEnum) << "]" << std::endl;
            // A black pieceEnum is always 1 more than its white counterpart
            auto& whitePmoList = _allPmoLists.at(pieceEnum);
            // tdout << "please don't be off by one" << std::endl;
            auto& blackPmoList = _allPmoLists.at(pieceEnum+1);
            match(_tokenizer.next(), "add");
            std::string moveTypeString = _tokenizer.next();
            match(_tokenizer.next(), "(");

            // Handle all of the stuff specific to what type of move option this is
            std::unique_ptr<MoveOption> moPtr;
            if (moveTypeString == "slide") {
                // tdout << "This is a slide move" << std::endl;
                auto tempPtr = std::make_unique<SlideMoveOption>();
                tempPtr->m_maxDist = _tokenizer.nextInt();
                tempPtr->m_isDiagonal = _tokenizer.nextBool();
                moPtr = std::move(tempPtr); // convert this into pointer to base class
            } else if (moveTypeString == "leap") {
                // tdout << "This is a leap move" << std::endl;
                auto tempPtr = std::make_unique<LeapMoveOption>();
                tempPtr->m_forwardDist = _tokenizer.nextInt();
                tempPtr->m_sideDist = _tokenizer.nextInt();
                moPtr = std::move(tempPtr); // convert this into pointer to base class
            } else {
                dlog(WHERE , "Error: Unknown move option type '" , moveTypeString , "' in rules file");
            }
            match(_tokenizer.next(), ")");
            // tdout << "Matched closing )" << std::endl;

            // Check if has properties to parse
            if (_tokenizer.peek() == "{") {
                // tdout << "Found prop list" << std::endl;
                MoveOptionProperties mop = defaultMop; // copy
                updateMoveOptionProperties(_tokenizer, mop);
                moPtr->m_properties = mop;
                // tdout << "prop list updated" << std::endl;
            }

            // Finally, add our object to the lists
            whitePmoList.push_back(std::move(moPtr));
            // tdout << "added to whitePmoList" << std::endl;
            // auto hyelp = moPtr->clone(); // FIXME: clone isn't working at runtime and I don't know why
            // auto hyelpheylp = std::unique_ptr<MoveOption>(hyelp);
            // blackPmoList.push_back(std::move(hyelp));
            // blackPmoList.push_back(std::unique_ptr<MoveOption>(moPtr->clone()));
            // tdout << "added to blackPmoList" << std::endl;

        } else if (targetPiece == "}") { // we found the matching brace, so we are done
            break;
        } else {
            dlog(WHERE , "Error: Expected piece identifier, found '" , targetPiece , "' in rules file");
        }
    }
};

bool Ruleset::init(std::string _ruleFile) {
    // reset pmo lists
    for (auto& pmoList : m_pieceMoveOptionLists) {
        pmoList.clear();
    }

    std::ifstream infile(_ruleFile);
    RulesetTokenizer tokenizer(infile);
    for(;;) {
        std::string ruleName = tokenizer.next();
        // tdout << "got next rule as " << ruleName << std::endl;
        if (ruleName == "numDeletionsPerTurn") {
            match(tokenizer.next(), "=");
            m_numDeletionsPerTurn = tokenizer.nextInt();
        } else if (ruleName == "allowTileMoves") {
            match(tokenizer.next(), "=");
            m_allowTileMoves = tokenizer.nextBool();
        } else if (ruleName == "allowRotations") {
            match(tokenizer.next(), "=");
            m_allowRotations = tokenizer.nextBool();
        } else if (ruleName == "allowReflections") {
            match(tokenizer.next(), "=");
            m_allowReflections = tokenizer.nextBool();
        } else if (ruleName == "pieceMoveOptions") {
            match(tokenizer.next(), "=");
            addPieceMoveOptions(tokenizer, m_pieceMoveOptionLists);
        } else if (ruleName == "") { // We've reach EOF
            return true;
        } else {
            dlog(WHERE , "Error: Unknown rule '" , ruleName , "' in rules file");
        }
    }
}

inline bool RulesetTokenizer::isNumberCharacter(char _c) {
    return (isdigit(_c) || _c == '-');
}

inline void RulesetTokenizer::eatWhitespace(std::istream& _stream) {
    while(iswspace(_stream.peek())) {
        _stream.ignore();
    }
}

std::string RulesetTokenizer::matchWordToken(std::istream& _stream) {
    std::string token = "";
    for(;;) {
        char lookahead = _stream.peek();
        if (!isalpha(lookahead)) {
            return token;
        }
        token += lookahead;
        _stream.ignore(); // pop character
    }
}

std::string RulesetTokenizer::matchDigitToken(std::istream& _stream) {
    std::string token = "";
    for(;;) {
        char lookahead = _stream.peek();
        if (!isNumberCharacter(lookahead)) {
            return token;
        }
        token += lookahead;
        _stream.ignore(); // pop character
    }
}

std::string RulesetTokenizer::next() {
    if (m_hasPeeked) {
        m_hasPeeked = false;
        return m_peeked;
    }

    // Eat all comments until the next actual lexeme
    eatWhitespace(m_stream);
    while (m_stream.peek() == '#') {
        while (m_stream.peek() != '\n') { // ignore until end of line
            m_stream.ignore();
        }
        eatWhitespace(m_stream);
    }

    char lookahead = m_stream.peek();

    if (isalpha(lookahead)) { 
        return matchWordToken(m_stream);

    } else if (isNumberCharacter(lookahead)) { 
        return matchDigitToken(m_stream);

    } else if (std::string("={}()").find(lookahead) != std::string::npos) { // other permissible single-character lexemes
        m_stream.ignore(); // pop character
        return std::string(1, lookahead);

    } else if (lookahead == EOF) {
        return ""; // Return empty string for EOF

    } else {
        dlog(WHERE , "Error: Unknown character in rules file '" , lookahead , "'");
        return "[ERROR!]";
    }
}
int RulesetTokenizer::nextInt() {
    std::string token = next();
    return std::stoi(token);
}

bool RulesetTokenizer::nextBool() {
    const static std::map<std::string, bool> boolLexemesMap = {
        {"orthogonal", false},
        {"diagonal", true},
        {"false", false},
        {"true", true}
    };

    std::string token = next();
    if (boolLexemesMap.count(token) > 0) {
        return boolLexemesMap.at(token);
    } else {
        dlog(WHERE , "Error: Expected a bool in rules file, got '" , token , "'");
        return false; //TODO: error handling should be better. For starts, a line number of file would be nice.
    }

}

