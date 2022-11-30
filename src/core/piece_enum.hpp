#ifndef PIECE_ENUM_H
#define PIECE_ENUM_H

#include <map>
#include <vector>

#include "../retrograde_analysis/piece_label.hpp"
#include "../utils/utils.h"

// The type PIECE_TYPE_ENUM is
using piece_type_enum_t = piece_label_t;
// Forward declare enum for ruleset to implement
enum PIECE_TYPE_ENUM : piece_type_enum_t;

// extern const int NUM_PIECE_TYPES;
// extern const int NUM_FLIPPABLE_PIECES;

// Forward declare converters for ruleset to implement
PIECE_TYPE_ENUM getTypeEnumFromPieceLabel(piece_label_t letter);
piece_label_t getPieceLabelFromTypeEnum(PIECE_TYPE_ENUM type);

#endif