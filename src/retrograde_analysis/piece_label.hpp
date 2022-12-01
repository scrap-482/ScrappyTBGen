#ifndef PIECE_LABEL_HPP_
#define PIECE_LABEL_HPP_

#include <cctype> // For tolower() and toupper()
#include <algorithm> // std::find
#include "../core/ignore_macros.hpp"

using piece_label_t = unsigned char;

const std::vector<piece_label_t> NON_ROYAL_PIECES = NO_ROYALTY_PIECESET;
const std::vector<piece_label_t> ROYAL_PIECES = ROYALTY_PIECESET;

// TODO: redo these to handle shogi-style promotion data

inline piece_label_t toBlack(piece_label_t letter) {
  return tolower(letter);
}
inline piece_label_t toWhite(piece_label_t letter) {
  return toupper(letter);
}
inline bool isEmpty(piece_label_t letter) {
  return letter == '\0';
}
inline bool isWhite(piece_label_t letter) {
  return isupper(letter);
}
inline bool isRoyal(piece_label_t letter) {
  return (std::find(ROYAL_PIECES.begin(), ROYAL_PIECES.end(), letter) != ROYAL_PIECES.end());
}

#endif