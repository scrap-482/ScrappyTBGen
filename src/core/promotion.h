#ifndef PROMOTION_H_
#define PROMOTION_H_

#include "../utils/utils.h"
#include "../core/coords_grid.hpp"
#include "../retrograde_analysis/state.hpp"

#include <map>

// This is essentially just a bidirectional map.
// Every piece type must be in this map, or you might get outOfBound error
// ASSUMPTION: all pieces must be black by convention
class PromotionScheme {
private:
    // from unpromoted to possible promotions
    std::map<piece_label_t, std::vector<piece_label_t>> promotionList;
    // from promoted to possible unpromotions
    std::map<piece_label_t, std::vector<piece_label_t>> unpromotionList;

public:
    // Builds unpromotionList using given promotionList.
    PromotionScheme(std::map<piece_label_t, std::vector<piece_label_t>> _promotionList);

    // For Shogi-style promotions, builds promotionList and unpromotionList by specifying which unflipped pieces can 
    // and cannot be promoted.
    // Do not put already promoted piece types in nonPromotablePieces, they are automatically unpromotable.
    // TODO: implement
    // PromotionScheme(std::vector<piece_label_t> promotablePieces, std::vector<piece_label_t> nonPromotablePieces);

    inline const std::vector<piece_label_t>& getPromotions(piece_label_t piece) {
        return promotionList.at(piece);
    }

    inline const std::vector<piece_label_t>& getUnpromotions(piece_label_t piece) {
        return unpromotionList.at(piece);
    }
};

// Announce that the user will define this global variable.
extern PromotionScheme promotionScheme;

#endif