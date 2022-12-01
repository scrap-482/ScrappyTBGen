#ifndef RECTANGULAR_BOARD_HPP
#define RECTANGULAR_BOARD_HPP
#include "coords_grid.hpp"
#include "ignore_macros.hpp"

const size_t BOARD_WIDTH = ROW_SZ;
const size_t BOARD_HEIGHT = COL_SZ;

using Coords = CoordsGrid<int, int, BOARD_WIDTH>;

#define inBounds(coords) ((coords.file >= 0 && coords.file < BOARD_WIDTH && coords.rank >= 0 && coords.rank < BOARD_HEIGHT))

#endif