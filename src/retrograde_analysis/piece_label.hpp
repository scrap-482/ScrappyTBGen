#ifndef PIECE_LABEL_HPP_
#define PIECE_LABEL_HPP_

#include <cctype> // For tolower() and toupper()

using piece_label_t = unsigned char;

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

#endif