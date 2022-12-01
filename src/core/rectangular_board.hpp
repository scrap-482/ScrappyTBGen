#ifndef RECTANGULAR_BOARD_HPP
#define RECTANGULAR_BOARD_HPP
#include "coords_grid.hpp"
#include "ignore_macros.hpp"

const size_t BOARD_WIDTH = ROW_SZ;
const size_t BOARD_HEIGHT = COL_SZ;

using Coords = CoordsGrid<int, int, BOARD_WIDTH>;

#define inBounds(coords) ((coords.file >= 0 && coords.file < BOARD_WIDTH && coords.rank >= 0 && coords.rank < BOARD_HEIGHT))

template<::std::size_t FS, typename NPDT>
std::string printBoard(const BoardState<FS, NPDT>& b) {
    std::string ret = "";
    /* -------------------------- Print placement data -------------------------- */
    // go reverse in rank since the y-axis is mirrored (lower rank is further down on output)
    for (size_t rank = BOARD_HEIGHT-1; rank != (size_t)-1; --rank) {
        for (size_t file = 0; file < BOARD_WIDTH; ++file) {
            auto piece = b.m_board.at(file + rank*BOARD_WIDTH);
            ret += (piece == '\0')? '.' : piece;
        }
        ret += "\n";
    }
    /* --------------------------- Print player's turn -------------------------- */
    ret += (b.m_player? "White to move\n" : "Black to move\n");
    /* -------------------------------- Print NPD ------------------------------- */
    ret += NPDToString<NPDT>(b.nonPlacementData);
    return ret;
}

#endif