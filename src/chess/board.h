#ifndef BOARD_H
#define BOARD_H

#include "constants.hpp"
#include "chess_utils.h"
#include "move.h"
#include "ruleset.h"

#include <vector>

class BoardPrintSettings {
    public:
    bool m_showCoords = false;
    size_t m_width = 3;
    size_t m_height = 1;
    char m_tileFillChar = '`';

    BoardPrintSettings(bool _showCoords = true, size_t _width = 3, size_t _height = 1, char _tileFillChar = '`') :  
    m_showCoords(_showCoords), m_width(_width), m_height(_height), m_tileFillChar(_tileFillChar) {}
};

// TODO: I feel like this object can be designed better...
// TODO: implement functionality around this object
class StandardArray {
    public:
    // Bounding size of array. 
    // m_dimensions.file describes number of columns, 
    // m_dimensions.rank describes number of rows.
    UnsignedCoords m_dimensions;

    std::vector<SquareEnum> m_array;

    StandardArray();
    // Creates a new empty array of size
    StandardArray(UnsignedCoords _size);
    // Creates new array from sfen
    StandardArray(std::string _sfen);

    // Get the square at given coords
    inline SquareEnum at(unsigned int _f, unsigned int _r) { // TODO: used this in code that did it manually before
        return m_array[m_dimensions.file*_r + _f];
    }

    // Just print the entire contents of the array as-is.
    // For debugging purposes only.
    std::string dumpAsciiArray();
};

class Board {
    public:
        BoardPrintSettings m_printSettings;

        Board(Ruleset& _ruleset, DModCoords _minCorner=DModCoords(0,0)) : m_displayCoordsZero(_minCorner), m_rules(_ruleset) { }

        // Resets board from SFEN.
        virtual void init(const std::string _sfen) = 0;

        // Which displayCoords map to standardArray's (0, 0)
        // Usually, should update whenever minCoords update.
        DModCoords m_displayCoordsZero;

        int m_material; // changed material score to just be material for both
        uint64_t m_hashCode;

        const Ruleset& m_rules;

        // Returns the placement component of the SFEN of the current state of the board.
        // Will also include minCorner if specified.
        virtual std::string toSfen(bool _includeMinCorner=true);

        /**
         * Boards are equal if all independent fields except moveHistory are equal. 
         * Note that comparing the hashes is MUCH faster that this, and should be used to compare boards
         * for most practical purposes. Really, this is just here to test if the hash function is working.
         */
        virtual bool operator==(const Board& _other) const;

        DModCoords SAtoDM(UnsignedCoords _standard) const;
        UnsignedCoords DMtoSA(DModCoords _dMod) const;

        /** 
         * Print the current tiles and pieces in a nice ASCII format.
         */
        virtual std::string getAsciiBoard();

        /**
         * Gets the size of the minimum rectangle needed to surround this board in its current configuration.
         */
        virtual UnsignedCoords getDimensions() const = 0;

        /**
         * Returns the type of piece/tile at these _coords.
         */
        virtual SquareEnum getPiece(DModCoords _coords) const = 0;

        // /**
        //  * Attempt to move selection of tiles (_selectMinR, _selectMinF)-(_selectMaxR, _selectMaxF) to the new coords (_goalMinR, _goalMinF).
        //  *                                        lower left corner           upper right corner                         lower left corner
        //  * Returns false if the move is illegal.
        //  */
        // virtual bool moveSelection(UnsignedCoords _select1, UnsignedCoords _select2, UnsignedCoords _goal1) = 0;

        // /**
        //  * Attempt to move piece at (_startR, _startF) to the new UnsignedCoords (_goalR, _goalF).
        //  * Returns false if the move is illegal.
        //  */
        // virtual bool movePiece(UnsignedCoords _start, UnsignedCoords _goal) = 0;

        virtual bool apply(const Move& _move) = 0;

        /**
         * Undoes the last move(s) made on this board.
         */
        virtual bool undo(const Move& _move) = 0;

        // Checks legality by generating all moves on this position and checking if this
        // is one of them.
        // FIXME: requires color
        virtual bool isLegal(const Move& _move, PieceColor _turn);

        /**
         * Gets the hash of this configuration.
         */
        virtual uint64_t getHash() const;

        virtual int staticEvaluation() = 0;

        // For debugging purposes.
        virtual std::string printPieces() {
            return "[Lol I'm just the parent class, I can't do this]";
        }

        // List of all legal moves that _color can play on this position.\
        // WARNING: is slow and should probably not be used when efficiency is 
        // needed, especially not in the game tree.
        virtual std::vector<std::unique_ptr<Move>> getMoves(PieceColor _color) = 0;
        // List of all legal moves plus some illegal moves that _color can play 
        // on this position. Designed for use in the game tree, or in similar
        // situations where we are applying the move anyway.
        virtual std::vector<std::unique_ptr<Move>> getMaybeMoves(PieceColor _color) = 0;

        // Returns list of moves that the piece at _pieceCoords can make using this MoveOption on the current _board.
        virtual std::vector<std::unique_ptr<PieceMove>> getMovesFromMO(DModCoords _pieceCoords, const MoveOption& _mo);
        virtual std::vector<std::unique_ptr<PieceMove>> getMovesFromMO(DModCoords _pieceCoords, const LeapMoveOption& _mo);
        virtual std::vector<std::unique_ptr<PieceMove>> getMovesFromMO(DModCoords _pieceCoords, const SlideMoveOption& _mo);

        // Gets the standard array of this board. This is the smallest sized array (namely, size getDimensions())
        // that contains all pieces. Indexed by (file + rank*getDimensions().file). Used for printing, hashing, etc.
        // TODO: once the standardArrays are better working, then require this as a function
        virtual StandardArray standardArray() = 0;

        //TODO: unvirtual getAscii and toSfen using new standardArray
};

#endif