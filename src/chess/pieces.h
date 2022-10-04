#ifndef PIECES_H
#define PIECES_H

//TODO: consider renaming? Idk if pieces.h is sufficently descriptive. 

#include "constants.hpp"
#include "move.h"

#include <vector>
#include <iostream>
#include <memory>

typedef int CaptureBehavior;
enum : CaptureBehavior {NORMAL_MOVEMENT, CAPTURE_ONLY, MOVE_ONLY, PROJECTILE}; // Note: PROJECTILE is not implemented almost anywhere in code. Consider removing it?

struct MoveOptionProperties {
    bool m_forwardOnly = false;

    // defines how piece can interact with enemy pieces for this MoveOption
    CaptureBehavior m_captureMode = NORMAL_MOVEMENT;

    bool m_flyOverGaps = false;

    bool m_flyOverPieces = false;
};

typedef int MOType;
enum : MOType {INVALID_MO_TYPE, SLIDE_MO_TYPE, LEAP_MO_TYPE};

class MoveOption {
    public:
        virtual std::unique_ptr<MoveOption> clone() const = 0; // Did this so white/black having same moveoptions utilize copying 
        MoveOptionProperties m_properties;
        MOType m_type = INVALID_MO_TYPE;
    protected:
        // only should be called by its children
        MoveOption(MOType _type) : m_type(_type) {}
};

class SlideMoveOption : public MoveOption {
    public:
        SlideMoveOption() : MoveOption(SLIDE_MO_TYPE) { }
        // Max distance piece can slide. Set to -1 for no limit
        int m_maxDist = -1;

        // If true, this is diagonal. Otherwise, this is orthogonal
        bool m_isDiagonal;

        std::unique_ptr<MoveOption> clone() const {
            // tdout << "slide clone called" << std::endl;
            return std::make_unique<SlideMoveOption>(*this);
        }
};

class LeapMoveOption : public MoveOption {
    public:
        LeapMoveOption() : MoveOption(LEAP_MO_TYPE) { }
        // How many tiles primary direction of leap translates. Note that if m_forwardOnly is true, this translation is the forward component.
        int m_forwardDist = 2; // default is normal Knight behavior
        // How many tiles secondary direction of leap translates. Secondary direction is ortho to primary direction.
        int m_sideDist = 1; // default is normal Knight behavior

        std::unique_ptr<MoveOption> clone() const {
            // tdout << "leap clone called" << std::endl;
            return std::make_unique<LeapMoveOption>(*this);
        }
};

// Note that if you want to add castling, promotion, en passant, or another special movement, you can extend MoveOption

#endif