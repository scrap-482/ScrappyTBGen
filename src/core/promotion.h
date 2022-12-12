/*
* Copyright 2022 SCRAP
*
* This file is part of Scrappy Tablebase Generator.
* 
* Scrappy Tablebase Generator is free software: you can redistribute it and/or modify it under the terms 
* of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, 
* or (at your option) any later version.
* 
* Scrappy Tablebase Generator is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of 
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along with Scrappy Tablebase Generator. If not, see <https://www.gnu.org/licenses/>.
*/


#ifndef PROMOTION_H_
#define PROMOTION_H_

#include "../utils/utils.h"
#include "../core/coords_grid.hpp"
#include "../retrograde_analysis/state.hpp"

#include <map>

// This is essentially just a bidirectional map.
// Every piece type must be in this map, or you might get outOfBound error
class PromotionScheme {
private:
    // from unpromoted to possible promotions
    std::map<piece_label_t, std::vector<piece_label_t>> promotionList;
    // from promoted to possible unpromotions
    std::map<piece_label_t, std::vector<piece_label_t>> unpromotionList;

public:
    // Builds unpromotionList using given promotionList.
    // WARNING: this function does not assume pieces of different colors have same promotions.
    // If that is the case, then use createFromColorSymmetricList instead.
    PromotionScheme(std::map<piece_label_t, std::vector<piece_label_t>> _promotionList);

    // Builds unpromotionList using given promotionList of only black pieces: white pieces will automatically follow this promotion scheme too.
    // WARNING: do not provide white pieces to this function!
    static PromotionScheme createFromColorSymmetricList(std::map<piece_label_t, std::vector<piece_label_t>> _blackPromotionList);

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