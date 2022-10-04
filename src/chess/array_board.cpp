#include "array_board.h"

#include "constants.hpp"
#include "chess_utils.h"
#include "move.h"

#include <cstdint>
#include <stack>
#include <algorithm>
#include <vector>
#include <iostream>
#include <ctype.h>
#include <limits.h>
#include <tuple>
#include <queue>
#include <map>
#include <assert.h>

// TODO: this file is unmanageably big, need to split it up more. E.g. PieceList is an inner class, etc.

// arrayboard modulus
unsigned int ABModulus;

// This function is only meant to be used in init right now, unless it is cleaned up into something more general.
void ArrayBoard::updateExtrema(const ABModCoords& _new) {
    // FIXME: ugly hack to get min to update. Maybe this is clue to how to actually get min to update later on
    if (_new.file == m_minCoords.file-1) {
        m_minCoords.file = _new.file;
    }
    if (_new.rank == m_minCoords.rank-1) {
        m_minCoords.rank = _new.rank;
    }

    m_maxCoords.file = std::max(m_maxCoords.file, _new.file, 
    [&](const ABModInt& _a, const ABModInt& _b) {
        return _a.lessThan(_b, m_minCoords.file);
    });
    m_maxCoords.rank = std::max(m_maxCoords.rank, _new.rank, 
    [&](const ABModInt& _a, const ABModInt& _b) {
        return _a.lessThan(_b, m_minCoords.rank);
    });
}

void ArrayBoard::init(const std::string _sfen) {
    resetPL();
    int i; // which character of _sfen we are on

/* ----------------- loop through and count number of tiles ----------------- */
    m_numTiles = 0;
    parseSfenPos(
        _sfen,
        [&](SquareEnum _piece) {
            m_numTiles++;
        },
        [&](int _numVoid) {
            // no op
        },
        [&](int _numEmpty) {
            m_numTiles += _numEmpty;
        },
        [&]() {
            // no op
        }
    );
    m_grid_size = m_numTiles + 1; // grid size = init number of tiles + 1. See documentation on wrap-around for explanation of why
    // check for multiplication overflow
    if (m_grid_size > SIZE_MAX / m_grid_size) {
        std::cerr << "Board is too large to be stored as ArrayBoard!" << std::endl;
        throw "Board is too large!";
    }
    // delete old m_grid
    if (m_grid != nullptr) {
        delete[] m_grid;
    }
    m_grid = new SquareEnum[m_grid_size*m_grid_size];
    // initialize all squares
    for (size_t i = 0; i < m_grid_size*m_grid_size; i++) {
        m_grid[i] = VOID;
    }

    // set the class' modulus to match the wrap-around of our array.
    ABModulus = m_grid_size;

/* ---------------- loop through again to initialize the grid --------------- */

    // some arbitrary starting point
    const ABModCoords STARTING_CORNER (0, 0);
    m_minCoords = m_maxCoords = STARTING_CORNER; // TODO: remove STARTING CORNER, and just use m_minCoords?

    ABModCoords currentFR = STARTING_CORNER;
    parseSfenPos(
        _sfen,
        [&](SquareEnum _piece) {
            m_grid[toIndex(currentFR)] = _piece;
            addPieceToPL(_piece, currentFR);
            updateExtrema(currentFR);
            ++currentFR.file;
        },
        [&](int _numVoid) {
            currentFR.file += _numVoid;
        },
        [&](int _numEmpty) {
            for (int k = 0; k < _numEmpty; k++) {
                m_grid[toIndex(currentFR)] = EMPTY;
                updateExtrema(currentFR);
                ++currentFR.file;
            }
        },
        [&]() {
            currentFR.file = STARTING_CORNER.file;
            --currentFR.rank;
        }
    );
    // Parse remaining fields
    // TODO: implement
    // FIXME: add dModZero to sfen format
    // dlog("min coords are ", m_minCoords.file, ", ", m_minCoords.rank, " and max are ", m_maxCoords.file, ", ", m_maxCoords.rank);
}

bool ArrayBoard::updatePieceInPL(SquareEnum _piece, ABModCoords _oldLocation, ABModCoords _newLocation) {
    for (int i = 0; i < m_pieceLocations[_piece].size(); i++) { // loop for all pieces of type _piece
        if (m_pieceLocations[_piece][i] == _oldLocation) { // find the match
            m_pieceLocations[_piece][i] = _newLocation;
            return true;
        }
    }
    return false;
}

bool ArrayBoard::removePieceFromPL(SquareEnum _piece, ABModCoords _location) {
    for (int i = 0; i < m_pieceLocations[_piece].size(); i++) { // loop for all pieces of type _piece
        if (m_pieceLocations[_piece][i] == _location) { // find the match
            m_pieceLocations[_piece].erase(m_pieceLocations[_piece].begin() + i);
            return true;
        }
    }
    return false;
}

bool ArrayBoard::apply(const Move& _move) {
    // dlog("apply ", _move.algebraic(), " called");
    switch (_move.m_type) {
    case PIECE_MOVE:
        return apply(static_cast<const PieceMove&>(_move));
    case TILE_MOVE:
        return apply(static_cast<const TileMove&>(_move));
    case TILE_DELETION:
        return apply(static_cast<const TileDeletion&>(_move));
    default:
        dlog("Unknown Move Type [", _move.m_type, "]\n", WHERE);
        return false;
    }
}
bool ArrayBoard::undo(const Move& _move) {
    // dlog(getAsciiBoard(), std::endl;
    // dlog("undo called", std::endl;
    switch (_move.m_type) {
    case PIECE_MOVE:
        return undo(static_cast<const PieceMove&>(_move));
    case TILE_MOVE:
        return undo(static_cast<const TileMove&>(_move));
    case TILE_DELETION:
        return undo(static_cast<const TileDeletion&>(_move));
    default:
        dlog("Unknown Move Type [" , _move.m_type , "]\n" , WHERE);
        return false;
    }
}

bool ArrayBoard::apply(const PieceMove& _move) {
    // dlog("applying PieceMove ", _move.algebraic(), std::endl;
    // dlog(getAsciiBoard(), std::endl;;
    ABModCoords startCoords = SAtoAB(DMtoSA(_move.m_startPos));
    ABModCoords endCoords = SAtoAB(DMtoSA(_move.m_endPos));

    // Execute the move
    // Update PieceList
    if (isPiece(m_grid[toIndex(endCoords)])) { // if a piece in endCoords, capture it
        removePieceFromPL(m_grid[toIndex(endCoords)], endCoords);
    }
    updatePieceInPL(m_grid[toIndex(startCoords)], startCoords, endCoords);

    // Update array
    m_grid[toIndex(endCoords)] = m_grid[toIndex(startCoords)];
    m_grid[toIndex(startCoords)] = EMPTY;
    return true;
};

bool ArrayBoard::undo(const PieceMove& _move) {
    // dlog("undoing move ", _move.algebraic(), std::endl;
    ABModCoords startCoords = SAtoAB(DMtoSA(_move.m_startPos));
    ABModCoords endCoords = SAtoAB(DMtoSA(_move.m_endPos));

    // Execute the undo
    // Update PieceList
    updatePieceInPL(m_grid[toIndex(endCoords)], endCoords, startCoords);
    if (isPiece(_move.m_capture)) { // if a piece was captured, add it back
        addPieceToPL(_move.m_capture, endCoords);
    }
    // Update array
    m_grid[toIndex(startCoords)] = m_grid[toIndex(endCoords)];
    m_grid[toIndex(endCoords)] = _move.m_capture; // Note this works since m_capture=EMPTY when no capture occurred
    return true;
};

bool ArrayBoard::apply(const TileMove& _move) {
    // dlog("applying tileMove ", _move.algebraic(), std::endl;
    // Store the absolute array position of our init selection
    ABModCoords abSelFirst = SAtoAB(DMtoSA(_move.m_selFirst));
    ABModCoords abSelSecond = SAtoAB(DMtoSA(_move.m_selSecond));

    // ----------- Cut out the selection ----------- //
    // Save old coords in case we have to undo the cut operation
    ABModCoords oldMin = m_minCoords, oldMax = m_maxCoords; 
    DModCoords oldDZero = m_displayCoordsZero;
    StandardArray cut = getSelection(abSelFirst, abSelSecond, true);
    // dlog("Cut out ", cut.dumpAsciiArray(), std::endl;
    // dlog("Now board is \n", getAsciiBoard(), std::endl;
    // dlog("min coords are ", m_minCoords.file, ", ", m_minCoords.rank, " and max are ", m_maxCoords.file, ", ", m_maxCoords.rank, std::endl;
    // dlog("and minDisplayCoords are ", m_displayCoordsZero, std::endl;

    // ----------- Paste in the selection ----------- //
    // Figure out if this move updates the minima
    // Assumes only 1-tiles moves in a large (#tiles < 1/2 DModuli)
    // FIXME: God is dead and this code has killed him. This probs should be redone from scratch to handle more than 1 tile moves.
    DModCoords destSecond = _move.m_destFirst + (_move.m_selSecond - _move.m_selFirst);
    // What our extrema are in DMod space.
    DModCoords displayCoordsMax = SAtoDM(ABtoSA(m_maxCoords));
    // dlog(
    // "_move.m_destFirst: ", _move.m_destFirst,
    // "\ndestSecond: ", destSecond,
    // "\n_move.m_selFirst: ", _move.m_selFirst,
    // "\n_move.m_selSecond: ", _move.m_selSecond,
    // "\nm_displayCoordsZero: ", m_displayCoordsZero, 
    // "\ndisplayCoordsMax: ", displayCoordsMax,
    // std::endl;

    // dlog(((_move.m_destFirst.file.heurLessThan(_move.m_selFirst.file))?"moving left":"not moving left"), std::endl;
    // dlog(((! _move.m_destFirst.file.isBetween(m_displayCoordsZero.file, displayCoordsMax.file))?"not in hor bounds":"in hor bounds"), std::endl;
    if ( //.file
        ( 
            (oldMin.file != m_minCoords.file && _move.m_destFirst.file.heurLessThanOrEqual(_move.m_selFirst.file)) || // THE minimum tile is being moved, and not to the right; or
            (_move.m_destFirst.file.heurLessThan(_move.m_selFirst.file)) // selection is moving left/down
        )
        &&
        (! _move.m_destFirst.file.isBetween(m_displayCoordsZero.file, displayCoordsMax.file)) // selection will be outside current bounds
       ) {
            // int difference = (m_displayCoordsZero.file - _move.m_destFirst.file).m_value;
            int difference = m_displayCoordsZero.file.getDistTo(_move.m_destFirst.file);
            m_minCoords.file += difference;
            m_displayCoordsZero.file += difference;
            // dlog("updated mins.file by ", difference, std::endl;
            assert(difference < 0);
    }
    // dlog(((_move.m_destFirst.rank.heurLessThan(_move.m_selFirst.rank))?"moving left":"not moving left"), std::endl;
    // dlog(((! _move.m_destFirst.rank.isBetween(m_displayCoordsZero.rank, displayCoordsMax.rank))?"not in vert bounds":"in vert bounds"), std::endl;
    if ( //.rank //TODO: combine these into one function somehow for readability.
        ( 
            (oldMin.rank != m_minCoords.rank && _move.m_destFirst.rank.heurLessThanOrEqual(_move.m_selFirst.rank)) || // THE minimum tile is being moved, and not to the right; or
            (_move.m_destFirst.rank.heurLessThan(_move.m_selFirst.rank)) // selection is moving left/down
        )
        &&
        (! _move.m_destFirst.rank.isBetween(m_displayCoordsZero.rank, displayCoordsMax.rank)) // selection will be outside current bounds
       ) {
            // int difference = (m_displayCoordsZero.rank - _move.m_destFirst.rank).m_value;
            int difference = m_displayCoordsZero.rank.getDistTo(_move.m_destFirst.rank);
            m_minCoords.rank += difference;
            m_displayCoordsZero.rank += difference;
            // dlog("updated mins.rank by ", difference, std::endl;
            assert(difference < 0);
    }

    // Now that our m_minCoords is updated, these operations are valid.
    ABModCoords abDestFirst = SAtoAB(DMtoSA(_move.m_destFirst));
    ABModCoords abDestSecond = SAtoAB(DMtoSA(destSecond));

    // Update m_maxCoords now. Same idea as updating the m_minCoords, but we can do this in ABCoord space now.
    // TODO: make max use same method as min, instead of the old continuity-dependent method.
    if ( //.file
        (m_maxCoords.file + 1)
        .isBetween(abDestFirst.file, abDestSecond.file)) {
            int difference = (abDestSecond.file - m_maxCoords.file).m_value;
            m_maxCoords.file += difference;
    }
    if ( //.rank
        (m_maxCoords.rank + 1)
        .isBetween(abDestFirst.rank, abDestSecond.rank)) {
            int difference = (abDestSecond.rank - m_maxCoords.rank).m_value;
            m_maxCoords.rank += difference;
    }

    // Check destination rectangle is empty
    for (ABModCoords i = abDestFirst; i.rank != abDestSecond.rank + 1; ++i.rank) { // iterate rows
        for (i.file = abDestFirst.file; i.file != abDestSecond.file + 1; ++i.file) { // iterate columns
            if (m_grid[toIndex(i)] != VOID) {
                dlog(WHERE , "FOUND TILE [" , getCharFromSquare(m_grid[toIndex(i)]) , "] in DESTINATION, at " , coordsToAlgebraic(SAtoDM(ABtoSA(i))));
                paste(cut, abSelFirst); // paste back in original position
                m_minCoords = oldMin;
                m_maxCoords = oldMax;
                m_displayCoordsZero = oldDZero;
                return false;
            }
        }
    }
    paste(cut, abDestFirst); // paste back in new position

    // Now, we have to check for continuity.
    if (!isContiguous()) {
        // dlog(WHERE, "Move not contiguous, reverting!", std::endl;
        clearSelection(abDestFirst, abDestSecond); // remove the paste
        paste(cut, abSelFirst); // paste back in original position
        m_minCoords = oldMin;
        m_maxCoords = oldMax;
        m_displayCoordsZero = oldDZero;
        // dlog(getAsciiBoard(), std::endl;
        return false;
    }

    // If we moved pieces, we need to update the pieceList.
    // TODO: this implementation only works for single-tile moves.
    if (isPiece(getPiece(_move.m_destFirst))) {
        if (!updatePieceInPL(getPiece(_move.m_destFirst), abSelFirst, abDestFirst)) {
            std::cerr << "Board pieceList is corrupted! " << WHERE << std::endl;
            exit(EXIT_FAILURE); // TODO: crash more gracefully
        }
    }
    return true;
};

bool ArrayBoard::undo(const TileMove& _move) {
    // Literally just move the selection back.
    DModCoords translationDist = (_move.m_selSecond - _move.m_selFirst);
    DModCoords destSecond = _move.m_destFirst + translationDist;
    auto reverseMove = std::make_unique<TileMove>(_move.m_destFirst, destSecond, _move.m_selFirst);
    return apply(*reverseMove);
}

bool ArrayBoard::apply(const TileDeletion& _move) {
    // dlog("applying TileDeletion move ", _move.algebraic(), std::endl;
    // Save values in case we have to revert
    auto oldMinCoords = m_minCoords;
    auto oldMaxCoords = m_maxCoords;
    auto oldDCZero = m_displayCoordsZero;

    for (const DModCoords& dModDeletion : _move.m_deleteCoords) {
        ABModCoords deletionCoords = SAtoAB(DMtoSA(dModDeletion));
        // if (m_grid[toIndex(deletionCoords)] == EMPTY) { // TODO: check if empty. Want to make sure we can undo it if part way we find it is illegal move.
        m_grid[toIndex(deletionCoords)] = VOID;
        --m_numTiles;
    }

    // it is possible we cut off the min/max, so we need to update it accordingly
    // dlog("minCoords=", m_minCoords, " maxCoords=", m_maxCoords, "before cut,", std::endl;
    ABModCoords updatedMin ( //TODO: this is repeated code from getSelection -- consider making this into a function, as well as its counterpart in apply, too
        nextTileByColOrder(ABModCoords(m_minCoords.file, 0), false).file, 
        nextTileByRowOrder(ABModCoords(0, m_minCoords.rank), false).rank
    ); // FIXME: something fishy going on with the literal zero's automatic type here
    ABModCoords updatedMax (
        nextTileByColOrder(ABModCoords(m_maxCoords.file, 0), true, true).file,
        nextTileByRowOrder(ABModCoords(0, m_maxCoords.rank), true, true).rank
    );
    ABModCoords minUpdate = updatedMin - m_minCoords; // since we are cutting tiles, updatedMin >= m_minCoords aka change in min is positive.
    m_displayCoordsZero += DModCoords(minUpdate.file.m_value, minUpdate.rank.m_value);
    m_minCoords = updatedMin;
    m_maxCoords = updatedMax;
    // dlog("now minCoords=", m_minCoords, " maxCoords=", m_maxCoords, "before after the cut", std::endl;

    // Now, we have to check for continuity.
    if (!isContiguous()) {
        // dout, WHERE, "Deletion not contiguous, reverting!", std::endl;
        m_minCoords = oldMinCoords;
        m_maxCoords = oldMaxCoords;
        m_displayCoordsZero = oldDCZero;
        for (const DModCoords& dModDeletion : _move.m_deleteCoords) {
            ABModCoords deletionCoords = SAtoAB(DMtoSA(dModDeletion));
            m_grid[toIndex(deletionCoords)] = EMPTY;
            ++m_numTiles;
        }
        // dlog(getAsciiBoard(), std::endl;
        // dlog("minDisplay: ", coordsToAlgebraic(m_displayCoordsZero), ", min: ", m_minCoords, ", max: ", m_maxCoords, std::endl;
        return false;
    }

    return true;
}

bool ArrayBoard::undo(const TileDeletion& _move) {
    // dlog("applying move ", _move.algebraic(), std::endl;
    // Note: this assumes that 2*[max number of tiles per deletion] + m_grid_size < 27*26 (the DModCoord space).
    // TODO: would be nice if there was a better way to figure out which side it corresponds to easier than checking which is closer so our assumption is not needed.
    for (const DModCoords& dModAddition : _move.m_deleteCoords) {
        // check if this addition would change m_minCoords
        DModCoords dModMin = SAtoDM(ABtoSA(m_minCoords)); //TODO: clean up this function's code
        DModCoords dModMax = SAtoDM(ABtoSA(m_maxCoords));
        // .file
        if (!dModAddition.file.isBetween(dModMin.file, dModMax.file)) { // check if this is outside of our current bounds
            unsigned int distToMin = (dModMin.file - dModAddition.file).m_value;
            unsigned int distToMax = (dModAddition.file - dModMax.file).m_value;
            if (distToMin < distToMax) {
                m_minCoords.file -= distToMin;
                m_displayCoordsZero.file -= distToMin;
            } else {
                m_maxCoords.file += distToMax;
            }
        }
        // .rank
        if (!dModAddition.rank.isBetween(dModMin.rank, dModMax.rank)) { // check if this is outside of our current bounds
            unsigned int distToMin = (dModMin.rank - dModAddition.rank).m_value;
            unsigned int distToMax = (dModAddition.rank - dModMax.rank).m_value;
            if (distToMin < distToMax) {
                m_minCoords.rank -= distToMin;
                m_displayCoordsZero.rank -= distToMin;
            } else {
                m_maxCoords.rank += distToMax;
            }
        }

        // Now that m_minCoords is updated, this conversion is safe.
        ABModCoords addedCoords = SAtoAB(DMtoSA(dModAddition));
        m_grid[toIndex(addedCoords)] = EMPTY;
        ++m_numTiles;
    }

    return true;
}

// bool ArrayBoard::isLegal(const Move& _move) {
//     switch (_move.m_type) {
//     case PIECE_MOVE:
//         return isLegal(std::static_pointer_cast<PieceMove>(_move));
//     case TILE_MOVE:
//         return isLegal(std::static_pointer_cast<TileMove>(_move));
//     case TILE_DELETION:
//         return isLegal(std::static_pointer_cast<TileDeletion>(_move));
//     default:
//         dlog("Unknown Move Type [" , _move.m_type , "]\n" , WHERE);
//         return false;
//     }
// }
// bool ArrayBoard::isLegal(const PieceMove& _move) {
//     SquareEnum pieceType = getPiece(_move.m_startPos);
//     if (!isPiece(pieceType)) return false;
//     for (auto moveOption : m_rules.m_pieceMoveOptionLists[pieceType]) {
//         if ()
//     }
//     //TODO: implement
//     return true;
// }
// bool ArrayBoard::isLegal(const TileMove& _move) {
//     // check selection rectangle is not oversized //TODO:
//     // check squares we are copying to is actually void. Handle the coords conversions. //TODO:
//     // check that connectedness is maintained //TODO: I think "Maintenance of a minimum spanning forest in a dynamic plane graph" by Eppstein et al. may give optimal results, although its probably easier to just run A* or smth
//     return true;
// }
// bool ArrayBoard::isLegal(const TileDeletion& _move) {
//     // check number of deletions OK
//     if (_move.m_deleteCoords.empty()) {
//         return false;
//     }
//     if (_move.m_deleteCoords.size() > m_rules.m_numDeletionsPerTurn) {
//         return false;
//     }
//     // check there are actually EMPTY tiles to delete at given coords //TODO
//     // for (DModCoords& delCoords : _move.m_deleteCoords) {
//     //     if ()
//     // }
//     // check deletions do not break connectedness //TODO

//     return true;
// }

// Convert external coords to internal, e.g. (0,0) will be converted to m_minCoords
ABModCoords ArrayBoard::SAtoAB(UnsignedCoords _extern) const {
    ABModCoords intern(_extern.file, _extern.rank); //KLUDGE: weird conversion
    intern.file += m_minCoords.file;
    intern.rank += m_minCoords.rank;
    return intern;
}
// Convert internal coords to external, e.g. m_minCoords will be converted to (0,0)
UnsignedCoords ArrayBoard::ABtoSA(ABModCoords _intern) const {
    _intern.file -= m_minCoords.file;
    _intern.rank -= m_minCoords.rank;
    UnsignedCoords external(_intern.file.m_value, _intern.rank.m_value);
    return external;
}

size_t ArrayBoard::toIndex(ABModCoords _coords) const {
    return _coords.file.m_value + m_grid_size * _coords.rank.m_value;
}

StandardArray ArrayBoard::standardArray() {
    UnsignedCoords dimensions = getDimensions();
    // standard array we will return
    StandardArray sa(dimensions);

    size_t i = 0; // current index of sa
    ABModCoords coords (m_minCoords.file, m_maxCoords.rank);
    // iterate over rows
    for (; coords.rank != m_minCoords.rank-1; --coords.rank) {
        coords.file = m_minCoords.file; // reset each loop to start at beginning of the row
        // iterate over columns
        for (; coords.file != m_maxCoords.file+1; ++coords.file) {
            sa.m_array[i++] = m_grid[toIndex(coords)];
        }
    }
    return sa;
}

std::string ArrayBoard::dumpAsciiArray() const {
    std::string result = "[";
    for (int row = 0; row < m_grid_size; ++row) {
        result += "\n";
        for (int col = 0; col < m_grid_size; ++col) {
            result += getCharFromSquare(m_grid[toIndex(ABModCoords(col, row))], '=', '.');
        }
    }
    result += "\n]";
    return result;
}

int ArrayBoard::staticEvaluation() {
    //TODO: this is just a dumb implementation to test if minmax works. Find a better implementation in the future.
    int staticValue = 0;
    for (int i = 1; i < NUM_PIECE_TYPES*2+1; i++) {
        staticValue += PIECE_VALUES[i] * m_pieceLocations[i].size();
        for (ABModCoords t : m_pieceLocations[i]) {
            // add 20 centipawns for being on 'even' coordinates
            if ((t.file + t.rank).m_value % 2 == 0) {
                staticValue += 20 * (isWhite(i) ? 1 : -1); // negate if black
            }
        }
    }
    return staticValue;
}

bool ArrayBoard::moveIsFromMO(const Move& _move, const MoveOption& _mo) {
    switch (_mo.m_type) {
    case LEAP_MO_TYPE:
        return moveIsFromMO(_move, dynamic_cast<const LeapMoveOption&>(_mo)); // TODO: why is this dynamic_cast, but other is static pointer cast?
    case SLIDE_MO_TYPE:
        return moveIsFromMO(_move, dynamic_cast<const SlideMoveOption&>(_mo));
    default:
        dlog("Unknown MoveOption Type [" , _mo.m_type , "]\n" , WHERE);
        return false;
    }
}

bool ArrayBoard::moveIsFromMO(const Move& _move, const LeapMoveOption& _mo) {
    // TODO: implement
    return false;
}

// Checks if the move could have been generated from current MoveOption
bool ArrayBoard::moveIsFromMO(const Move& _move, const SlideMoveOption& _mo) {
    // TODO: implement
    // TODO: If we don't care about efficiency, this can just call the general getMovesFromMO and we don't even need this function then
    return false;
}

void test (const Move& _move) {
    dlog(_move.algebraic());
}

std::vector<std::unique_ptr<Move>> ArrayBoard::getMoves(PieceColor _color) {
    std::vector<std::unique_ptr<Move>> legalMoves;
    auto maybeMoves = getMaybeMoves(_color);
    for (auto& maybeMove : maybeMoves) {
        if(!apply(*maybeMove)) {
            // Turns out this move wasn't legal
            // dlog("skipping move", maybeMove->algebraic(), " because it wasn't actually legal.");
            continue;
        }
        // else, this is legal
        if (!undo(*maybeMove)) {
            std::cerr << "Board is corrupted, exiting!" << WHERE << std::endl;
            exit(EXIT_FAILURE);
        }
        legalMoves.push_back(std::move(maybeMove)); // Do this last bcz std::move
    }
    return legalMoves;
}

std::vector<std::unique_ptr<PieceMove>> ArrayBoard::getPieceMoves(PieceColor _color) {
    std::vector<std::unique_ptr<PieceMove>> legalMoves;
    // ----------- PieceMoves ----------- //
    // dlog("Getting PieceMoves", std::endl;
    for (int pieceType = (_color==WHITE ? W_PAWN : B_PAWN); pieceType < NUM_PIECE_TYPES*2+1; pieceType+=2) { // iterate over pieces of _color in piece list
        // dlog("  getting moves for pieces of type [", getCharFromSquare(pieceType), "]", std::endl;
        auto& moveOptions = m_rules.m_pieceMoveOptionLists.at(pieceType);
        for (ABModCoords pieceCoords : m_pieceLocations[pieceType]) { // for all coords of pieces of this type
            // dlog("    checking MoveOptions for piece on coords ", SAtoDM(ABtoSA(pieceCoords)), std::endl;
            for (auto& mo : moveOptions) {
                // dlog("      checking a MoveOption for piece on coords ", SAtoDM(ABtoSA(pieceCoords)), std::endl;
                std::vector<std::unique_ptr<PieceMove>> newMoves = getMovesFromMO( SAtoDM(ABtoSA(pieceCoords)), *(mo.get()));
                // Moves new moves into our vector
                legalMoves.insert(legalMoves.end(), std::make_move_iterator(newMoves.begin()), std::make_move_iterator(newMoves.end()));
            }
        }
    }
    return legalMoves;
}
std::vector<std::unique_ptr<TileMove>> ArrayBoard::getMaybeTileMoves(PieceColor _color) {
    std::vector<std::unique_ptr<TileMove>> maybeMoves;
    // ----------- TileMoves ----------- //
    // TODO: assumes only one tile can be moved, implement for more tiles?
    // dlog("Getting tileMoves", std::endl;
    // First, iterate over all possible tiles we can move.
    // TODO: gross iteration over the entire space, would be nice if I only iterated over tiles.
    if (m_rules.m_allowTileMoves) {
        ABModCoords startCoords = m_minCoords;
        // iterate over rows
        for (; startCoords.rank != m_maxCoords.rank+1; ++startCoords.rank) {
            startCoords.file = m_minCoords.file; // reset each loop to start at beginning of the row
            // iterate over columns
            for (; startCoords.file != m_maxCoords.file+1; ++startCoords.file) {
                if (m_grid[toIndex(startCoords)] != VOID) { // ignore non-tile squares

                    DModCoords dmStartCoords = SAtoDM(ABtoSA(startCoords));
                    // Now, iterate over possible squares that this tile can be moved to
                    // We do this in DModCoords because it can go smaller than m_minCoords.
                    DModCoords dmMinCoords = SAtoDM(ABtoSA(m_minCoords));
                    DModCoords dmMaxCoords = SAtoDM(ABtoSA(m_maxCoords));
                    DModCoords endCoordsMin = dmMinCoords - DModCoords(1,1);
                    DModCoords endCoordsMax = dmMaxCoords + DModCoords(1,1);
                    DModCoords endCoords = endCoordsMin;
                    for (; endCoords.rank != endCoordsMax.rank+1; ++endCoords.rank) {
                        endCoords.file = endCoordsMin.file; // reset each loop to start at beginning of the row
                        // iterate over columns
                        for (; endCoords.file != endCoordsMax.file+1; ++endCoords.file) {
                            if (
                                // If endCoords is out of bounds, this is a valid move, but it is not safe to do conversions since m_minCoords affects conversion.
                                (
                                    !endCoords.file.isBetween(dmMinCoords.file, dmMaxCoords.file) ||
                                    !endCoords.rank.isBetween(dmMinCoords.rank, dmMaxCoords.rank)
                                )
                                || // else endCoords is within bounds. This means we can safely convert it to ABCoords, but that we also have to check the array to see if there is a tile there blocking it.
                                (
                                    m_grid[toIndex(SAtoAB(DMtoSA(endCoords)))] == VOID && 
                                    dmStartCoords != endCoords // just in case
                                )
                            ) {
                                std::unique_ptr<TileMove> newMove (new TileMove(dmStartCoords, dmStartCoords, endCoords));
                                maybeMoves.push_back(std::move(newMove));
                            }
                        }
                    }
                }
            }
        }
    }

    return maybeMoves;

}
std::vector<std::unique_ptr<TileDeletion>> ArrayBoard::getMaybeTileDeletions(PieceColor _color) {
    std::vector<std::unique_ptr<TileDeletion>> maybeMoves;
    // ----------- TileDeletions ----------- //
        // TODO: gross iteration over the entire space, would be nice if I only iterated over tiles.
    if (m_rules.m_numDeletionsPerTurn > 0) { // TODO: assumes only one tile can be deleted, implement for more tiles?
        ABModCoords coords (m_minCoords.file, m_maxCoords.rank);
        // iterate over rows
        for (; coords.rank != m_minCoords.rank-1; --coords.rank) {
            coords.file = m_minCoords.file; // reset each loop to start at beginning of the row
            // iterate over columns
            for (; coords.file != m_maxCoords.file+1; ++coords.file) {
                if (m_grid[toIndex(coords)] == EMPTY) {
                    std::unique_ptr<TileDeletion> newMove (new TileDeletion(SAtoDM(ABtoSA(coords))));
                    maybeMoves.push_back(std::move(newMove));
                }
            }
        }
    }
    return maybeMoves;
}

std::vector<std::unique_ptr<Move>> ArrayBoard::getMaybeMoves(PieceColor _color) {
    std::vector<std::unique_ptr<Move>> maybeMoves;
    // just a combination of each of the functions' returns.
    auto pieceMoves = getPieceMoves(_color);
    for (auto& ptr : pieceMoves) maybeMoves.push_back(std::move(ptr));
    auto tileMoves = getMaybeTileMoves(_color);
    for (auto& ptr : tileMoves) maybeMoves.push_back(std::move(ptr));
    auto tileDeletions = getMaybeTileDeletions(_color);
    for (auto& ptr : tileDeletions) maybeMoves.push_back(std::move(ptr));

    return maybeMoves;
}

StandardArray ArrayBoard::getSelection(const ABModCoords& _bl, const ABModCoords& _tr, bool _cut) { // TODO: consider merging with standardArray() ?
    // dout, "getting selection ", std::endl;
    UnsignedCoords dimensions ((_tr - _bl).file.m_value+1, (_tr - _bl).rank.m_value+1);
    // standard array we will return
    StandardArray sa(dimensions);

    // dout, "filling stdArray", std::endl;
    size_t i = 0; // current index of sa
    ABModCoords coords (_bl.file, _tr.rank);
    // iterate over rows
    for (; coords.rank != _bl.rank-1; --coords.rank) {
        coords.file = _bl.file; // reset each loop to start at beginning of the row
        // iterate over columns
        for (; coords.file != _tr.file+1; ++coords.file) {
            sa.m_array[i++] = m_grid[toIndex(coords)];
            if (_cut) {
                m_grid[toIndex(coords)] = VOID;
            }
        }
    }
    if (_cut) {
        // dlog("minCoords=", m_minCoords, " maxCoords=", m_maxCoords, "before cut,", std::endl;
        // it is possible we cut off the min/max, so we need to update it accordingly
        ABModCoords updatedMin (
            nextTileByColOrder(ABModCoords(m_minCoords.file, 0), false).file, 
            nextTileByRowOrder(ABModCoords(0, m_minCoords.rank), false).rank
        );
        ABModCoords updatedMax (
            nextTileByColOrder(ABModCoords(m_maxCoords.file, 0), true, true).file,
            nextTileByRowOrder(ABModCoords(0, m_maxCoords.rank), true, true).rank
        );
        ABModCoords minUpdate = updatedMin - m_minCoords; // since we are cutting tiles, updatedMin >= m_minCoords aka change in min is positive.
        m_displayCoordsZero += DModCoords(minUpdate.file.m_value, minUpdate.rank.m_value);
        m_minCoords = updatedMin;
        m_maxCoords = updatedMax;
        // dlog("now minCoords=", m_minCoords, " maxCoords=", m_maxCoords, "before after the cut", std::endl;
    }

    // dout, "got selection ", std::endl;
    return sa;
}

void ArrayBoard::paste(const StandardArray& _sa, const ABModCoords& _bl) {
    ABModCoords tr = _bl + _sa.m_dimensions - SignedCoords(1,1);
    size_t i = 0; // current index of sa

    ABModCoords coords (_bl.file, tr.rank);
    // iterate over rows
    // dlog("pasted: ";
    for (; coords.rank != _bl.rank-1; --coords.rank) {
        coords.file = _bl.file; // reset each loop to start at beginning of the row
        // iterate over columns
        for (; coords.file != tr.file+1; ++coords.file) {
            m_grid[toIndex(coords)] = _sa.m_array[i++];
            // dlog(coordsToAlgebraic(SAtoDM(ABtoSA(coords))), ", ";
        }
    }
    // dlog("\b\b  ", std::endl;
}

void ArrayBoard::clearSelection(const ABModCoords& _bl, const ABModCoords& _tr) {
    ABModCoords coords (_bl.file, _tr.rank);
    for (; coords.rank != _bl.rank-1; --coords.rank) {
        coords.file = _bl.file; // reset each loop to start at beginning of the row
        // iterate over columns
        for (; coords.file != _tr.file+1; ++coords.file) {
            m_grid[toIndex(coords)] = VOID;
        }
    }
}

ABModCoords ArrayBoard::nextTileByRowOrder(const ABModCoords& _start, bool _reverse, bool _colReversed) const {
    ABModCoords current = _start;
    // dlog("nextTiles search:", current;
    int rankIncr = (_reverse? -1:1);
    int fileIncr = (_reverse? -1:1) * (_colReversed? -1:1);
    while (m_grid[toIndex(current)] == VOID) {
        // check if reached end of row. If positive fileIncr, end of row is m_grid_size-1, otherwise end of row is 0
        if (current.file.m_value == (fileIncr==1 ? m_grid_size-1 : 0)) { 
            current.rank += rankIncr;
        }
        current.file += fileIncr;
        // dlog(current;
    }
    return current;
}

ABModCoords ArrayBoard::nextTileByColOrder(const ABModCoords& _start, bool _reverse, bool _rowReversed) const {
    ABModCoords current = _start;
    // dlog("nextTiles search:", current;
    int fileIncr = (_reverse? -1:1);
    int rankIncr = (_reverse? -1:1) * (_rowReversed? -1:1);
    while (m_grid[toIndex(current)] == VOID) {
        // check if reached end of column. If positive rankIncr, end of column is m_grid_size-1, otherwise end of column is 0
        if (current.rank.m_value == (rankIncr==1 ? m_grid_size-1 : 0)) { 
            current.file += fileIncr;
        }
        current.rank += rankIncr;
        // dlog(current;
    }
    return current;
}

// Algorithm: start on a tile, do a BFS, and see if the number of tiles we found matches m_numTiles. If less found, then it must be noncontiguous.
bool ArrayBoard::isContiguous() const {
    // dlog("starting isContiguous", std::endl;
    if (m_numTiles == 0) {
        // dlog("found no tiles -> contiguous", std::endl;
        return true; // seems like no tiles should be considered a valid board, and a valid board must be contiguous.
    }
    ABModCoords bfsStart = m_minCoords; // semi-arbitrary starting point. There should be a tile in this row, so we only have to search 1 row at most.
    // ensure that we are starting on a tile
    bfsStart = nextTileByRowOrder(bfsStart);
    // dlog("starting on tile ", coordsToAlgebraic(SAtoDM(ABtoSA(bfsStart))), std::endl;

    std::map<ABModCoords, SearchState, compareModCoordsColOrder<ABModInt, ABModInt>> bfsMarkedTiles;
    std::queue<ABModCoords> bfsQueue;

    bfsQueue.push(bfsStart);
    bfsMarkedTiles.insert(std::make_pair(bfsStart, SearchState::QUEUED));

    while (!bfsQueue.empty()) {
        ABModCoords exploring = bfsQueue.front();
        bfsQueue.pop();
        // dlog("exploring tile ", coordsToAlgebraic(SAtoDM(ABtoSA(exploring))), std::endl;
        for (DirectionEnum dir : ORTHO_DIRECTIONS) {
            // make sure we don't consider ABCoord space wrap-arounds as adjacencies
            if (
                ((dir == UP) && (exploring.rank == m_minCoords.rank-1)) ||
                ((dir == DOWN) && (exploring.rank == m_minCoords.rank)) ||
                ((dir == RIGHT) && (exploring.file == m_minCoords.file-1)) ||
                ((dir == LEFT) && (exploring.file == m_minCoords.file))
            ) {
                // dlog("skipped direction ", getCharFromDirection(dir), std::endl;
                continue;
            }
            ABModCoords adjSquareCoords = exploring + DIRECTION_SIGNS[dir];
            SquareEnum adjSquare = m_grid[toIndex((adjSquareCoords))];
            if (adjSquare != VOID) {
                // dlog("found adjacent ", coordsToAlgebraic(SAtoDM(ABtoSA(adjSquareCoords)));
                auto it = bfsMarkedTiles.find(adjSquareCoords);
                if (it == bfsMarkedTiles.end()) {
                    // this means we've never found this adjSquareCoords as either QUEUED or EXPLORED
                    bfsQueue.push(adjSquareCoords);
                    bfsMarkedTiles.insert(std::make_pair(adjSquareCoords, SearchState::QUEUED));
                    // dlog(" and added it to queue", std::endl;
                } else {
                    // dlog(" but it was already found", std::endl;
                }
            }
        }
        bfsMarkedTiles[exploring] = SearchState::EXPLORED; // Note: I don't think there is a difference between EXPLORED or QUEUED marking, but it is easier to read probably.
    }
    // Number of explored tiles equals actual number of tiles iff tiles are contiguous.
    // dlog("Conclusion of isContiguous: ", bfsMarkedTiles.size(), " == ", m_numTiles, std::endl;
    return bfsMarkedTiles.size() == m_numTiles;
}
