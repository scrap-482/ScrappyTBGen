#ifndef CHESS_CONSTANTS_HPP
#define CHESS_CONSTANTS_HPP

#include <string>
#include <vector>
#include "../utils/coords.hpp"

// TODO: the contents of this file feel very misc-y, should probably restructure a little.

// typedef std::pair<unsigned int, unsigned int> UnsignedCoords;
typedef Coords<unsigned int, unsigned int> UnsignedCoords;
typedef Coords<int, int> SignedCoords;

enum class CustomException {ENGINE_DIED=101};

// For search algorithms
enum class SearchState {QUEUED, EXPLORED};

typedef signed char PieceColor;
// White is 1, black is -1. 
enum : PieceColor {BLACK=-1, WHITE=1};

// TODO: currently unused, consider removing
enum class PieceType {PAWN, ROOK, KNIGHT, BISHOP, QUEEN, KING};
const int NUM_PIECE_TYPES = 6;

/**
 * White is odd, black is even, excluding EMPTY=0 and VOID.
*/
typedef unsigned char SquareEnum; // TODO: consider re-writing these enums to not use typedefs.
enum : SquareEnum {EMPTY=0, W_PAWN=1, B_PAWN, W_ROOK, B_ROOK, W_KNIGHT, B_KNIGHT, W_BISHOP, B_BISHOP, W_QUEEN, B_QUEEN, W_KING, B_KING, VOID};
// How many types of pieces there are, counting white and black as different
static const int NUM_PIECE_TYPES_BY_COLOR = 2*NUM_PIECE_TYPES;

// How we represent tiles with characters. Parallel to SquareEnum
const std::string TILE_LETTERS = ".PpRrNnBbQqKk?";

static const SquareEnum BLACK_PIECES[NUM_PIECE_TYPES] = {B_PAWN, B_ROOK, B_KNIGHT, B_BISHOP, B_QUEEN, B_KING};
static const SquareEnum WHITE_PIECES[NUM_PIECE_TYPES] = {W_PAWN, W_ROOK, W_KNIGHT, W_BISHOP, W_QUEEN, W_KING};

// piece value list: //TODO: this is very limited way to look at piece value
// used as follows: PIECE_VALUES[W_PAWN], for example
const short PIECE_VALUES[] = {0, 100, -100, 500, -500, 300, -300, 300, -300, 900, -900, 10000, -10000};

/**
 * Paired up so opposites are next to eachother and can be easily flipped.
 * Sometimes we only use the first 4 as indices, which is why they are first.
*/
typedef unsigned char DirectionEnum;
enum : DirectionEnum {LEFT, RIGHT, UP, DOWN, DOWN_LEFT, UP_RIGHT, DOWN_RIGHT, UP_LEFT};

// Provided these for syntactic sugar of for looping through directions
const std::vector<DirectionEnum> ORTHO_DIRECTIONS {LEFT, RIGHT, UP, DOWN};
const std::vector<DirectionEnum> DIAG_DIRECTIONS {DOWN_LEFT, UP_RIGHT, DOWN_RIGHT, UP_LEFT};
const std::vector<DirectionEnum> ALL_8_DIRECTIONS {LEFT, RIGHT, UP, DOWN, DOWN_LEFT, UP_RIGHT, DOWN_RIGHT, UP_LEFT};

static const SignedCoords DIRECTION_SIGNS[] = {
    SignedCoords(-1,0), SignedCoords(1,0),
    SignedCoords(0,1), SignedCoords(0,-1),
    SignedCoords(-1,-1), SignedCoords(1,1),
    SignedCoords(1,-1), SignedCoords(-1,1)
};

static const char* ASCII_LOGO =
"      ______   ______   __                       \n"
"     /\\  ___\\ /\\  ___\\ /\\ \\                      \n"
"     \\ \\___  \\\\ \\  __\\ \\ \\ \\____                 \n"
"      \\/\\_____\\\\ \\_____\\\\ \\_____\\                \n"
"       \\/_____/ \\/_____/ \\/_____/                \n"
"   ______   __  __   ______   ______   ______    \n"
"  /\\  ___\\ /\\ \\_\\ \\ /\\  ___\\ /\\  ___\\ /\\  ___\\   \n"
"  \\ \\ \\____\\ \\  __ \\\\ \\  __\\ \\ \\___  \\\\ \\___  \\  \n"
"   \\ \\_____\\\\ \\_\\ \\_\\\\ \\_____\\\\/\\_____\\\\/\\_____\\ \n"
"    \\/_____/ \\/_/\\/_/ \\/_____/ \\/_____/ \\/_____/ \n"
"   ================================================\n";
#endif