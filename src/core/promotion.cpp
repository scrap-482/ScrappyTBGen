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
