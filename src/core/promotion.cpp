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
