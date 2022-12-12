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