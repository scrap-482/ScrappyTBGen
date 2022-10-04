#ifndef ARRAY_BOARD_H
#define ARRAY_BOARD_H

#include "constants.hpp"
#include "board.h"
#include "move.h"
#include "../utils/modular_int.hpp"
#include "../utils/coords.hpp"

#include <cstdint>
#include <stack>
#include <vector>
#include <string>

extern unsigned int ABModulus;
typedef ModularInt<&ABModulus> ABModInt;
typedef Coords<ABModInt, ABModInt> ABModCoords; //FIXME

class ArrayBoard : public Board {
    public:
        SquareEnum* m_grid = nullptr;
        // max number of tiles the grid can contain, and also the length and height of the grid.
        size_t m_grid_size; 

        // How many tiles (either empty or with pieces) there are.
        unsigned int m_numTiles;

        // minimum x and y of this board. Because of wrap-around, the literal min integer value is not 
        // guaranteed to be the furtherest "left" or "down".
        ABModCoords m_minCoords;
        ABModCoords m_maxCoords;
        
        /** For looking up where pieces are at by their type and color. 
         * Organized in same order as SquareEnum: 
         *      since SquareEnum starts pieces on 1, the zero element of this is a placeholder.
        */
        std::vector<ABModCoords> m_pieceLocations[2 * NUM_PIECE_TYPES+1];

        explicit ArrayBoard(Ruleset& _ruleset, DModCoords _minCorner=DModCoords(0,0)) : Board(_ruleset, _minCorner) { };
        void init(const std::string _sfen);
        ArrayBoard(Ruleset& _ruleset, const std::string _sfen, DModCoords _minCorner=DModCoords(0,0)) : ArrayBoard(_ruleset, _minCorner) {
            init(_sfen);
        }

        StandardArray standardArray();

        /** 
         * Clears entire piece list
         */
        void resetPL() {
            for (SquareEnum _piece = 0; _piece < 2 * NUM_PIECE_TYPES + 1; _piece++) { // loop for all lists
                m_pieceLocations[_piece].clear();
            }
        }

        /** 
         * Update the position oldLocation to be newLocation for type piece.
         * Returns false if it does not find such a piece to update.
         */
        bool updatePieceInPL(SquareEnum _piece, ABModCoords _oldLocation, ABModCoords _newLocation);

        /** 
         * Remove the piece at location for type piece.
         * Returns false if it does not find such a piece to remove.
         */
        bool removePieceFromPL(SquareEnum _piece, ABModCoords _location);
        /** 
         * Adds the piece at location for type piece.
         */
        void addPieceToPL(SquareEnum _piece, ABModCoords _location) {
            m_pieceLocations[_piece].push_back(_location);
        }

        // public function for getting the piece // TODO: rename to getSquare
        SquareEnum getPiece(DModCoords _coords) const {
            return m_grid[toIndex(SAtoAB(DMtoSA(_coords)))];
        };

        bool apply(const Move& _move);
        bool apply(const PieceMove& _move);
        bool apply(const TileMove& _move);
        bool apply(const TileDeletion& _move);

        bool undo(const Move& _move);
        bool undo(const PieceMove& _move);
        bool undo(const TileMove& _move);
        bool undo(const TileDeletion& _move);

        // bool isLegal(const Move& _move);
        // bool isLegal(const PieceMove& _move);
        // bool isLegal(const TileMove& _move);
        // bool isLegal(const TileDeletion& _move);

        UnsignedCoords getDimensions() const {
            // Just taking the m_value here is OK because this distance is not affected by modulus, so it is guaranteed result to be a normal positive number.
            return UnsignedCoords((m_maxCoords.file - m_minCoords.file + 1).m_value, (m_maxCoords.rank - m_minCoords.rank + 1).m_value);
        };

        // For debugging purposes. Prints all pieces in the m_pieceLocations list
        std::string printPieces() {
            std::string result = "[";
            for (int i = 1; i < NUM_PIECE_TYPES*2+1; i++) {
                result += TILE_LETTERS[i];
                result += "=" + std::to_string(m_pieceLocations[i].size()) + "{";
                for (ABModCoords& t : m_pieceLocations[i]) {
                    result += TILE_LETTERS[m_grid[toIndex(t)]];
                    result += " ";
                }
                result += "} ";
            }
            result += "]";
            return result;
        }

        std::vector<std::unique_ptr<PieceMove>> getPieceMoves(PieceColor _color);
        std::vector<std::unique_ptr<TileMove>> getMaybeTileMoves(PieceColor _color);
        std::vector<std::unique_ptr<TileDeletion>> getMaybeTileDeletions(PieceColor _color);

        std::vector<std::unique_ptr<Move>> getMoves(PieceColor _color);
        std::vector<std::unique_ptr<Move>> getMaybeMoves(PieceColor _color);

        int staticEvaluation();

        // Just print the entire contents of the array as-is.
        // For debugging purposes only.
        std::string dumpAsciiArray() const;

        // Gets the index of m_grid corresponding to _coords. Takes internal UnsignedCoords as input.
        size_t toIndex(ABModCoords _coords) const;

        // Convert external coords to internal, e.g. (0,0) will be converted to m_minCoords
        ABModCoords SAtoAB(UnsignedCoords _extern) const;
        // Convert internal coords to external, e.g. m_minCoords will be converted to (0,0)
        UnsignedCoords ABtoSA(ABModCoords _intern) const;

        bool isContiguous() const;

    protected: //TODO: sort some more stuff into protected?
        // check if adding a tile at _new ABModCoords will update m_minCoords or m_maxCoords
        void updateExtrema(const ABModCoords& _new);

        // Get the next coords at or after _start, sorted by row order. Assumes board has tiles
        ABModCoords nextTileByRowOrder(const ABModCoords& _start, bool _reverse=false, bool _colReversed=false) const;
        // Get the next coords at or after _start, sorted by column order. Assumes board has tiles
        ABModCoords nextTileByColOrder(const ABModCoords& _start, bool _reverse=false, bool _rowReversed=false) const;

        inline bool compareFileCoords(const ABModCoords& _a, const ABModCoords& _b) const {
            // effectively a < b
            return _a.file.isBetween(m_minCoords.file, _b.file, true);
        }
        inline bool compareRankCoords(const ABModCoords& _a, const ABModCoords& _b) const {
            // effectively a < b
            return _a.rank.isBetween(m_minCoords.rank, _b.rank, true);
        }

        // Copy the tiles from bottom left _bl to top right _tr into a standardarray. If _cut=true, remove the original copied section.
        StandardArray getSelection(const ABModCoords& _bl, const ABModCoords& _tr, bool _cut=false);

        // Deletes all tiles from bottom left _bl to top right _tr.
        void clearSelection(const ABModCoords& _bl, const ABModCoords& _tr);

        // Paste the tiles from _sa into our grid, matching bottom left _bl to bottom left of _sa.
        // WARNING: does not update m_minCoords, m_maxCoords, pieceList, or any other member besides m_grid!
        void paste(const StandardArray& _sa, const ABModCoords& _bl);

        // TODO: Do these MO functions generalize well enough to be moved into board.h?
        // Checks if the move could have been generated from current MoveOption
        bool moveIsFromMO(const Move& _move, const MoveOption& _mo);
        bool moveIsFromMO(const Move& _move, const LeapMoveOption& _mo);
        bool moveIsFromMO(const Move& _move, const SlideMoveOption& _mo);
};

inline ABModCoords& operator+=(ABModCoords& _mc1, const SignedCoords& _diff) {
    _mc1.file += _diff.file;
    _mc1.rank += _diff.rank;
    return _mc1;
}
inline ABModCoords& operator-=(ABModCoords& _mc1, const SignedCoords& _diff) {
    _mc1.file -= _diff.file;
    _mc1.rank -= _diff.rank;
    return _mc1;
}
inline ABModCoords operator+(ABModCoords _mc1, const SignedCoords& _diff) {
    return _mc1 += _diff;
}
inline ABModCoords operator+(const SignedCoords& _diff, ABModCoords _mc1) {
    return _mc1 += _diff;
}
inline ABModCoords operator-(ABModCoords _mc1, const SignedCoords& _diff) {
    return _mc1 -= _diff;
}

#endif