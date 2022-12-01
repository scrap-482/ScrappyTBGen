#include "definitions.h"

PromotionScheme promotionScheme = PromotionScheme::createFromColorSymmetricList(std::map<piece_label_t, std::vector<piece_label_t>> {
#ifdef UNDERPROMOTION_ENABLED
    {'p', {'q', 'n', 'b', 'r', 'c', 'a'}},
#else
    {'p', {'q'}},
#endif
    {'r', {}},
    {'n', {}},
    {'b', {}},
    {'q', {}},
    {'k', {}},
    {'c', {}},
    {'a', {}},
});