#include "board.h"
#include <algorithm>

std::string Board::toSfen(bool _includeMinCorner) {
    // Convert into a standard array representation
    StandardArray sa = standardArray();

    if (sa.m_array.empty()) {
        return "[Empty board]"; // TODO: [Very Low Priority] There is probably a nicer way to display this. Probably should still to show the whole frame and everything.
    }

    std::string sfen = ""; // returned result

    unsigned int numOfEmpty = 0;
    unsigned int numOfVoid = 0;

    // function to improve code readability
    auto endEmptySequence = [&]() {
        if (numOfEmpty != 0) {
            sfen += std::to_string(numOfEmpty);
            numOfEmpty = 0;
        }
    };
    auto endVoidSequence = [&]() {
        if (numOfVoid != 0) {
            sfen += "(" + std::to_string(numOfVoid) + ")";
            numOfVoid = 0;
        }
    };

    // iterate over rows
    for (int row = 0; row < sa.m_dimensions.rank; row++) { // for each row
        // start of new line
        if (row != 0) { // check if this is not first iteration of loop, i.e. this is a line break
            endEmptySequence();
            numOfVoid = 0; // we can ignore trailing void squares
            sfen += "/";
        }

        // iterate over columns
        for (int col = 0; col < sa.m_dimensions.file; col++) { // for each element
            SquareEnum thisSquare = sa.m_array[col + sa.m_dimensions.file*row];
            if (thisSquare == VOID) {
                endEmptySequence();
                numOfVoid++;
            } else if (thisSquare == EMPTY) {
                endVoidSequence();
                numOfEmpty++;
            } else { // is a piece
                endEmptySequence();
                endVoidSequence();
                sfen += TILE_LETTERS[thisSquare];
            }
        }
    }
    endEmptySequence();
    if (_includeMinCorner) {
        sfen += " " + coordsToAlgebraic(m_displayCoordsZero);
    }
    return sfen;
};

/**
 * Note that comparing the hashes is MUCH faster that this, and should be used to compare boards
 * for most practical purposes. Really, this is just here to test if the hash function is working.
 */
bool Board::operator==(const Board& _other) const {
    // TODO: implement using StandardArray
    return false;
}

/** 
 * Print the current tiles and pieces in a nice ASCII format.
 */
std::string Board::getAsciiBoard() {
    // Convert into a standard array representation
    StandardArray sa = standardArray();

    if (sa.m_array.empty()) {
        return "[Empty board]"; // TODO: [Very Low Priority] There is probably a nicer way to display this. Probably should still to show the whole frame and everything.
    }

    std::vector<std::string> lines; // each element is line of output

    // DRAWING CONTROLS
    const std::string V_SEP = "|"; // sides of tile, repeated m_printSettings.m_height times
    const std::string CORNER = "+"; // corner where h_sep and V_SEP meet

    std::string h_sep = ""; // top/bottom of tiles. Must be odd since piece must be centered.
    std::string h_fill_out = ""; // filler used for outside of cell. Must be same size as h_sep
    std::string h_fill_in = ""; // filler used for outside of cell. Must be same size as h_sep
    std::string h_non_sep = ""; // Used like an h_sep but for when there is no adjacent cell
    for (int i = 0; i < m_printSettings.m_width; i++) {
        h_sep += "-";
        h_fill_out += " ";
        h_fill_in += m_printSettings.m_tileFillChar;
        h_non_sep += " ";
    }
    std::string h_pad_left = std::string((m_printSettings.m_width - 1) / 2, m_printSettings.m_tileFillChar); // Used between V_SEP and the piece character. Sizes must satisfy h_sep = h_pad + 1 + h_pad.
    std::string h_pad_right = std::string(m_printSettings.m_width / 2, m_printSettings.m_tileFillChar);

    // Margin drawing controls
    const unsigned int LEFT_MARGIN_SIZE = 6; // width of left margin
    const unsigned int LEFT_MARGIN_PAD = 3; // space between left margin and leftmost tiles
    const char MARGIN_V_SEP = '|'; // vertical boundary for margin
    const char MARGIN_H_SEP = '='; // horizontal boundary for margin
    const char MARGIN_H_LABEL_DIV = ' '; // What separates the labels on the x axis from eachother

    for (int i = 0; i < 2 + m_printSettings.m_height; i++) { // +2 for V_SEP on both sides
        lines.push_back("");
    }
    size_t activeLineNum = 0; // First line of output we are currently modifying
    bool trailingEdge = false; // Whether the last tile of this row already has printed the edge we share with it
    for (int row = 0; row < sa.m_dimensions.rank; row++) { // for each row
        if (row != 0) { // start the next line
            trailingEdge = false;
            for (int i = 0; i < 1 + m_printSettings.m_height; i++) { // +1 for V_SEP
                activeLineNum++;
                lines.push_back("");
            }
        }
        for (int col = 0; col < sa.m_dimensions.file; col++) { // for each element
            SquareEnum thisSquare = sa.m_array[col + sa.m_dimensions.file*row];
            size_t lowerLine = activeLineNum + m_printSettings.m_height + 1;

            if (thisSquare == VOID) { // print empty space where there is no tiles
                for (int i = 1; i <= m_printSettings.m_height; i++) {
                    lines[activeLineNum + i] += (trailingEdge ? "" : " ") + h_fill_out;
                }
                lines[lowerLine] += (trailingEdge ? "" : " ") + h_non_sep;
                // We did not print righthand edge in case our neighbor is a tile and needs to print its edge there
                trailingEdge = false;

            } else { // Tile is here, may have a piece or be empty.
                // Output our tile's bottom border and center.
                for (int i = 1; i <= m_printSettings.m_height; i++) {
                    if (i == m_printSettings.m_height / 2 + 1) { // Is this the row which contains the piece
                        lines[activeLineNum + i] += (trailingEdge ? "" : V_SEP) + h_pad_left + getCharFromSquare(thisSquare, m_printSettings.m_tileFillChar) + h_pad_right + V_SEP;
                    } else {
                        lines[activeLineNum + i] += (trailingEdge ? "" : V_SEP) + h_fill_in + V_SEP;
                    }
                }
                lines[lowerLine] += (trailingEdge ? "" : CORNER) + h_sep + CORNER;

                // Place upper line of our tile. This one is tricky bcz it is also the lower line of the previous row.
                // Fill in the line if needed so we can just run the same replacement and not worry about out-of-bounds
                for (int i = lines[activeLineNum + 1].size() - lines[activeLineNum].size(); i > 0; i--) {
                    lines[activeLineNum] += " ";
                }
                // Override upper border with an edge, whether it is empty or there is already a tile with a lower edge there.
                size_t starting = lines[activeLineNum + 1].size() - (m_printSettings.m_width + 2);
                lines[activeLineNum].replace(starting, (m_printSettings.m_width + 2), CORNER + h_sep + CORNER);

                // We printed the righthand edge
                trailingEdge = true;
            }
        }
    }

    std::string result = "";
    std::string dividerLine;

    // Add stuff to top of output;
    if (m_printSettings.m_showCoords) {
        // the cornerpiece
        result += MARGIN_V_SEP;
        result += std::string(LEFT_MARGIN_SIZE - 1, ' ');
        result += MARGIN_V_SEP;

        // pad before labels start
        result += std::string(LEFT_MARGIN_PAD, ' ');

        // labels
        result += MARGIN_H_LABEL_DIV;
        for (auto xLabel = 0; xLabel != sa.m_dimensions.file; xLabel++) {
            std::string labelString = intToLetters(m_displayCoordsZero.file + xLabel);
            while (labelString.size() < m_printSettings.m_width) {
                labelString += " "; // label filler
            }
            result += labelString + MARGIN_H_LABEL_DIV;
        }

        // long horizontal line
        dividerLine = std::string(result.size(), MARGIN_H_SEP);
        result = dividerLine + "\n" + result + "\n" + dividerLine + "\n";
    }
    // Add stuff to left side of output
    int currentY = sa.m_dimensions.rank - 1;
    for (int i = 0; i < lines.size(); i++) {
        if (m_printSettings.m_showCoords) {
            std::string leftMargin = std::string(1, MARGIN_V_SEP) + " ";
            if (i % (m_printSettings.m_height+1) == m_printSettings.m_height / 2 + 1) {
                leftMargin += std::to_string((m_displayCoordsZero.rank + currentY--).m_value);
            }
            while (leftMargin.size() < LEFT_MARGIN_SIZE) {
                leftMargin += " ";
            }
            leftMargin += MARGIN_V_SEP;
            leftMargin += std::string(LEFT_MARGIN_PAD, ' ');
            result += leftMargin;
        }
        result += lines[i] + "\n";
    }
    if (m_printSettings.m_showCoords) {
        result += dividerLine + "\n";
    }
    // tdout << "Length of printable string = " << result.length() << std::endl;
    return result;
}

uint64_t Board::getHash() const {
    // TODO: implement using StandardArray
    return 0u;
};

StandardArray::StandardArray() {
    m_dimensions = UnsignedCoords(0,0);
    m_array.clear();
}

StandardArray::StandardArray(UnsignedCoords _size) {
    m_dimensions = _size;
    m_array.resize(m_dimensions.file * m_dimensions.rank, VOID);
}

StandardArray::StandardArray(std::string _sfen) {
    // tdout << "created SA from Sfen" << std::endl;
    _sfen = _sfen.substr(0, _sfen.find(" ")); // remove game info, which we do not need

    // get dimensions by counting max length
    m_dimensions.file = 0;
    m_dimensions.rank = 1; // start at 1, add 1 every newline.
    unsigned int squareCount = 0; // how many squares this line has
    parseSfenPos(
        _sfen,
        [&](SquareEnum _piece) {
            squareCount++;
        },
        [&](int _numVoid) {
            squareCount += _numVoid;
        },
        [&](int _numEmpty) {
            squareCount += _numEmpty;
        },
        [&]() {
            m_dimensions.rank++;
            m_dimensions.file = std::max(m_dimensions.file, squareCount);
            squareCount = 0; // reset
        }
    );
    // Since we don't end with '/' but need to update after row read
    m_dimensions.file = std::max(m_dimensions.file, squareCount);

    dlog("Found dimension of " , m_dimensions.file , ", " , m_dimensions.rank);
    // allocate the array
    m_array.resize(m_dimensions.file * m_dimensions.rank, VOID);

    int currentRow = m_dimensions.rank - 1;
    int currentColumn = 0;
    parseSfenPos(
        _sfen,
        [&](SquareEnum _piece) {
            m_array[currentColumn + currentRow*m_dimensions.file] = _piece;
            currentColumn++;
        },
        [&](int _numVoid) {
            currentColumn += _numVoid;
        },
        [&](int _numEmpty) {
            for (int etci = 0; etci < _numEmpty; etci++) {
                m_array[currentColumn + currentRow*m_dimensions.file] = EMPTY;
                currentColumn++;
            }
        },
        [&]() {
            currentRow--;
            currentColumn = 0;
        }
    );
}

std::string StandardArray::dumpAsciiArray() {
    std::string result = "[";
    for (int row = 0; row < m_dimensions.rank; row++) {
        result += "\n";
        for (int col = 0; col < m_dimensions.file; col++) {
            result += getCharFromSquare(m_array[col + m_dimensions.file*row], '=', '.');
        }
    }
    result += "\n]";
    return result;
}

DModCoords Board::SAtoDM(UnsignedCoords _standard) const { // TODO: surely we can simplify these 4 conversion functions by just inlining with the += op?
    DModCoords dMod (_standard.file, _standard.rank); // TODO: conversions between coords?
    dMod.file += m_displayCoordsZero.file;
    dMod.rank += m_displayCoordsZero.rank;
    return dMod;
}
UnsignedCoords Board::DMtoSA(DModCoords _dMod) const {
    _dMod.file -= m_displayCoordsZero.file;
    _dMod.rank -= m_displayCoordsZero.rank;
    return UnsignedCoords(_dMod.file.m_value, _dMod.rank.m_value);
}

std::vector<std::unique_ptr<PieceMove>> Board::getMovesFromMO(DModCoords _pieceCoords, const MoveOption& _mo) {
    switch (_mo.m_type) {
    case LEAP_MO_TYPE:
        return getMovesFromMO(_pieceCoords, dynamic_cast<const LeapMoveOption&>(_mo));
    case SLIDE_MO_TYPE:
        return getMovesFromMO(_pieceCoords, dynamic_cast<const SlideMoveOption&>(_mo));
    default:
        dlog("Unknown MoveOption Type [" , _mo.m_type , "]\n" , WHERE);
        return std::vector<std::unique_ptr<PieceMove>>();
    }
}

std::vector<std::unique_ptr<PieceMove>> Board::getMovesFromMO(DModCoords _pieceCoords, const LeapMoveOption& _mo) {
    // dlog("getMovesFrom Leaping MO called", std::endl;
    DModCoords startCoords = _pieceCoords;
    std::vector<std::unique_ptr<PieceMove>> moves;


    if (_mo.m_properties.m_forwardOnly) {
        dlog("UNIMPLEMENTED FEATURE: forwardOnly" , WHERE);
    }
    if (!_mo.m_properties.m_flyOverGaps || !_mo.m_properties.m_flyOverPieces) {
        dlog("UNIMPLEMENTED FEATURE: flyOverX=false" , WHERE);
    }

    for (DirectionEnum direction : ORTHO_DIRECTIONS) { // iterate over 4 directions
        // dlog("checking in direction ", getCharFromDirection(direction), std::endl;
        DirectionEnum bwdLoopStartCond;
        DirectionEnum bwdLoopEndCond;
        // Get perpendicular directions
        if (direction == LEFT || direction == RIGHT) {
            bwdLoopStartCond = UP;
            bwdLoopEndCond = DOWN;
        } else {
            bwdLoopStartCond = LEFT;
            bwdLoopEndCond = RIGHT;
        }
        // KLUDGE: weird way of iterating the two `perDirections` perpendicular to `direction`.
        for (DirectionEnum perpDirection = bwdLoopStartCond; perpDirection <= bwdLoopEndCond; ++perpDirection) {
            SignedCoords leapAmount = (DIRECTION_SIGNS[direction] * _mo.m_forwardDist) + (DIRECTION_SIGNS[perpDirection] * _mo.m_sideDist);
            DModCoords endCoords = startCoords + leapAmount;

            // check if void
            if (getPiece(endCoords) == VOID) {
                // Do nothing

            // check if empty
            } else if (getPiece(endCoords) == EMPTY) {
                if (_mo.m_properties.m_captureMode != CAPTURE_ONLY) {
                    // dlog("found valid moved", std::endl;
                    std::unique_ptr<PieceMove> newMove (new PieceMove(startCoords, endCoords));
                    moves.push_back(std::move(newMove));
                } // TODO: this smells like repeated code...

            // check if capture
            } else if (isPiece(getPiece(endCoords))) { // Are we moving onto another piece
                if (isWhite(getPiece(startCoords)) != isWhite(getPiece(endCoords))) { // Is this an enemy piece
                    if (_mo.m_properties.m_captureMode != MOVE_ONLY) {
                        // dlog("found valid capture", std::endl;
                        std::unique_ptr<PieceMove> newMove (new PieceMove(startCoords, endCoords));
                        newMove->m_capture = getPiece(endCoords);
                        moves.push_back(std::move(newMove));
                    }
                }
            }
        }
    }
    return std::move(moves);
}

// Returns list of moves that the piece at _pieceCoords can make using this MoveOption on the current _board.
std::vector<std::unique_ptr<PieceMove>> Board::getMovesFromMO(DModCoords _pieceCoords, const SlideMoveOption& _mo) {
    // dlog("getMovesFrom Sliding MO called", std::endl;
    DModCoords startCoords = _pieceCoords;
    std::vector<std::unique_ptr<PieceMove>> moves;
    auto& loopDirections = (_mo.m_isDiagonal ? DIAG_DIRECTIONS : ORTHO_DIRECTIONS);

    if (_mo.m_properties.m_forwardOnly) {
        dlog("UNIMPLEMENTED FEATURE: forwardOnly" , WHERE);
    }

    for (DirectionEnum direction : loopDirections) { // iterate over 4 directions
        // dlog("checking in direction ", getCharFromDirection(direction), std::endl;
        DModCoords endCoords = startCoords;
        for (int slideRemaining = _mo.m_maxDist; slideRemaining != 0; --slideRemaining) { // Loop until we can't slide any further. Note != is used instead of > to ensure -1 loops forever
            endCoords += DIRECTION_SIGNS[direction];

            // check if void
            if (getPiece(endCoords) == VOID) {
                if (!_mo.m_properties.m_flyOverGaps) {
                    break; // stop searching in this direction
                }
            // check if empty
            } else if (getPiece(endCoords) == EMPTY) {
                if (_mo.m_properties.m_captureMode != CAPTURE_ONLY) {
                    // dlog("found valid moved", std::endl;
                    std::unique_ptr<PieceMove> newMove (new PieceMove(startCoords, endCoords));
                    moves.push_back(std::move(newMove));
                }

            // check if capture
            } else if (isPiece(getPiece(endCoords))) { // Are we moving onto another piece
                if (isWhite(getPiece(startCoords)) != isWhite(getPiece(endCoords))) { // Is this an enemy piece
                    if (_mo.m_properties.m_captureMode != MOVE_ONLY) {
                        // dlog("found valid capture", std::endl;
                        std::unique_ptr<PieceMove> newMove (new PieceMove(startCoords, endCoords));
                        newMove->m_capture = getPiece(endCoords);
                        moves.push_back(std::move(newMove));
                    }
                }
                if (!_mo.m_properties.m_flyOverPieces) {
                    break; // stop searching this direction
                }
            }
        }
    }
    return std::move(moves);
}

bool Board::isLegal(const Move& _move, PieceColor _turn) {
    auto possibleMoves = getMoves(_turn);
    for (std::unique_ptr<Move>& possibleMove : possibleMoves) {
        // std::shared_ptr<Move> possibleMove = std::move(m);
        if (_move == *possibleMove) {
            return true;
        }
    }
    return false;
}
