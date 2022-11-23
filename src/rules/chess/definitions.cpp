#include "definitions.h"

PromotionScheme promotionScheme (std::map<piece_label_t, std::vector<piece_label_t>> {
#ifdef UNDERPROMOTION_ENABLED
    {'p', {'q', 'k', 'b', 'r'}},
#else
    {'p', {'q'}},
#endif
    {'r', {}},
    {'n', {}},
    {'b', {}},
    {'q', {}},
    {'k', {}},
});