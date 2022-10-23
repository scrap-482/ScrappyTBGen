#ifndef REGION_H
#define REGION_H

// The different sections of the game board
class Region {
    // Used by Chinese chess as rectangles
    // Chess pawn promotion can use regions
    // 
    // If single rectangular region, only one element. Weirdly shaped regions would require multiple
    // std::vector<std::pair<Coordinates, Coordinates>> regionSpan;
    std::pair<Coordinates, Coordinates> regionSpan;
};


#endif