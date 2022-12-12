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


#include "promotion.h"

PromotionScheme::PromotionScheme(std::map<piece_label_t, std::vector<piece_label_t>> _promotionList) : promotionList(_promotionList) {
    // First, ensure unpromotionList has all keys and empty arrays
    for (auto iter = promotionList.begin(); iter != promotionList.end(); ++iter) {
        unpromotionList.insert(std::make_pair(iter->first, std::vector<piece_label_t>()));
    }

    // Then, fill in the values of unpromotion map
    for (auto mapIter = promotionList.begin(); mapIter != promotionList.end(); ++mapIter) {
        for (auto listIter = mapIter->second.begin(); listIter != mapIter->second.end(); ++listIter) {
            auto unpromotedPiece = mapIter->first;
            auto promotedPiece = *listIter;
            unpromotionList[promotedPiece].push_back(unpromotedPiece);
        }
    }
}


PromotionScheme PromotionScheme::createFromColorSymmetricList(std::map<piece_label_t, std::vector<piece_label_t>> _blackPromotionList) {
    // create map of white from black
    std::map<piece_label_t, std::vector<piece_label_t>> whitePromotionList;

    // loop for all keys
    for (auto blackPromoEntry : _blackPromotionList) {
        piece_label_t whiteUnpromotedPiece = toWhite(blackPromoEntry.first);
        // create empty vector as value for this key
        whitePromotionList.insert({whiteUnpromotedPiece, std::vector<piece_label_t>()});

        // copy all values for this key
        for (auto blackPromotedPiece : blackPromoEntry.second) {
            piece_label_t whitePromotedPiece = toWhite(blackPromotedPiece);

            whitePromotionList[whiteUnpromotedPiece].push_back(whitePromotedPiece);
        }
    }

    // then copy over the entries so it is promotionList of both white and black
    whitePromotionList.insert(_blackPromotionList.begin(), _blackPromotionList.end());
    return PromotionScheme(whitePromotionList);
}
