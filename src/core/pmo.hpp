#ifndef PMO_H_
#define PMO_H_
// #include "piece_type.hpp" // caution! circularity approaching

#include "../utils/utils.h"
#include "../core/coords_grid.hpp"
#include "../retrograde_analysis/state.hpp"

// Shorthand: 
// FS = FlattenedSize
// NPDT = NonPlacementDataType
// CT = CoordsType

template<::std::size_t FS, typename NPDT, typename CT>
class PMO {
public:
    // takes a board state and the piece's current position and returns a list of new possible board states
    virtual ::std::vector<BoardState<FS, NPDT>> 
    getForwards(const BoardState<FS, NPDT>& b, CT piecePos) const = 0;

    // takes a board state and the piece's current position and returns a list of possible board states that could have resulted in it
    virtual ::std::vector<BoardState<FS, NPDT>> 
    getReverses(const BoardState<FS, NPDT>& b, CT piecePos) const = 0;
};

/* -------- Below are some PMO templates that may be useful extending ------- */

// Any move that takes a piece and moves it somewhere else. Exposes displacement of moves to make complex move implementation easier.
template<::std::size_t FS, typename NPDT, typename CT>
class DisplacementPMO : public PMO<FS, NPDT, CT> {
public:
    // takes a board state and the piece's current position and returns a list of new possible board states 
    // AND a parallel vector of the displacements of the moving piece 
    virtual ::std::pair<::std::vector<BoardState<FS, NPDT>>, ::std::vector<CT>>
    getForwardsWithDisplacement(const BoardState<FS, NPDT>& b, CT piecePos) const = 0;

    // takes a board state and the piece's current position and returns a list of possible board states that could have resulted in it
    // AND a parallel vector of the displacements of the moving piece 
    virtual ::std::pair<::std::vector<BoardState<FS, NPDT>>, ::std::vector<CT>>
    getReversesWithDisplacement(const BoardState<FS, NPDT>& b, CT piecePos) const = 0;

    ::std::vector<BoardState<FS, NPDT>> 
    getForwards(const BoardState<FS, NPDT>& b, CT piecePos) const override {
        return getForwardsWithDisplacement(b, piecePos).first;
    };

    ::std::vector<BoardState<FS, NPDT>> 
    getReverses(const BoardState<FS, NPDT>& b, CT piecePos) const override {
        return getReversesWithDisplacement(b, piecePos).first;
    };
};

template<::std::size_t FS, typename NPDT, typename CT>
class PMOPreMod {
public:
    virtual bool operator()(const BoardState<FS, NPDT>& b, CT piecePos) const = 0;
};

template<::std::size_t FS, typename NPDT, typename CT>
class PMOPostMod {
public:
    // virtual ::std::pair<::std::vector<BoardState<FS, NPDT>>, ::std::vector<CT>>
    virtual void operator()(
            ::std::pair<::std::vector<BoardState<FS, NPDT>>, ::std::vector<CT>>& moves
            , const BoardState<FS, NPDT>& b, CT piecePos) const = 0;
};

// Abstract class for any PMOPostMod that conditionally prohibits moves
template<::std::size_t FS, typename NPDT, typename CT>
class DeletionPMOPostMod : public PMOPostMod<FS, NPDT, CT> {
public:
    // Returns true if this move needs to be deleted
    virtual bool isProhibited(
            BoardState<FS, NPDT>& moveState, CT moveDisplacement
            , const BoardState<FS, NPDT>& b, CT piecePos) const = 0;

    virtual void operator()(
            ::std::pair<::std::vector<BoardState<FS, NPDT>>, ::std::vector<CT>>& moves
            , const BoardState<FS, NPDT>& b, CT piecePos) const override {
        for (size_t i = 0; i < moves.first.size(); ) {
            BoardState<FS, NPDT> &moveState = moves.first.at(i);
            CT moveDisplacement = moves.second.at(i);

            if (isProhibited(moveState, moveDisplacement, b, piecePos)) {
                moves.first.erase(moves.first.begin() + i);
                moves.second.erase(moves.second.begin() + i);
            } else {
                // only increment if we didn't delete, since deleting moves remaining indices forwards
                ++i;
            }
        }
    };
};

// Can be used for either no-capture allowed, or capture mandatory
template<::std::size_t FS, typename NPDT, typename CT>
class FwdCaptureDependentPMOPostMod : public DeletionPMOPostMod<FS, NPDT, CT> {
private:
    bool canCapture;
public:
    // Set capture=true for mandatory capture, set capture=false for prohibited capture.
    FwdCaptureDependentPMOPostMod(bool _canCapture) : canCapture(_canCapture) { }

    // virtual ::std::pair<::std::vector<BoardState<FS, NPDT>>, ::std::vector<CT>>
    virtual bool isProhibited(
            BoardState<FS, NPDT>& moveState, CT moveDisplacement
            , const BoardState<FS, NPDT>& b, CT piecePos) const override {
        // ASSUMPTION: Capturing piece ends its displacement on captured piece
        bool isCapture = b.m_board.at((piecePos + moveDisplacement).flatten()) != '\0';
        // if is is a capture and capture prohibited, erase it. Similarly if not a capture and mandatory
        return isCapture ^ canCapture;
    };
};
// Can be used for either no-capture allowed, or capture mandatory
template<::std::size_t FS, typename NPDT, typename CT>
class BwdCaptureDependentPMOPostMod : public DeletionPMOPostMod<FS, NPDT, CT> {
private:
    bool canCapture;
public:
    // Set capture=true for mandatory capture, set capture=false for prohibited capture.
    BwdCaptureDependentPMOPostMod(bool _canCapture) : canCapture(_canCapture) { }

    // virtual ::std::pair<::std::vector<BoardState<FS, NPDT>>, ::std::vector<CT>>
    virtual bool isProhibited(
            BoardState<FS, NPDT>& moveState, CT moveDisplacement
            , const BoardState<FS, NPDT>& b, CT piecePos) const override {
        // ASSUMPTION: Capturing piece ends its displacement on captured piece
        bool isCapture = moveState.m_board.at(piecePos.flatten()) != '\0';
        // if is is a capture and capture prohibited, erase it. Similarly if not a capture and mandatory
        return isCapture ^ canCapture;
    };
};


template<::std::size_t FS, typename NPDT, typename CT>
class ModdablePMO : public DisplacementPMO<FS, NPDT, CT> {
public:
    virtual ::std::pair<::std::vector<BoardState<FS, NPDT>>, ::std::vector<CT>>
    getModdableMovesWithDisplacement(const BoardState<FS, NPDT>& b, CT piecePos) const = 0;

    // takes a board state and the piece's current position and returns a list of possible board states that could have resulted in it
    // AND a parallel vector of the displacements of the moving piece 
    virtual ::std::pair<::std::vector<BoardState<FS, NPDT>>, ::std::vector<CT>>
    getModdableUnmovesWithDisplacement(const BoardState<FS, NPDT>& b, CT piecePos) const = 0;

    ::std::pair<::std::vector<BoardState<FS, NPDT>>, ::std::vector<CT>>
    getForwardsWithDisplacement(const BoardState<FS, NPDT>& b, CT piecePos) const override {
        // prune illegal moves based on these starting conditions
        for (const auto pre : preFwdMods) {
            if (!(*pre)(b, piecePos)) {
                // return empty set
                return ::std::pair<::std::vector<BoardState<FS, NPDT>>, ::std::vector<CT>>();
            }
        }
        // main move functionality specified by subclasses
        auto moves = getModdableMovesWithDisplacement(b, piecePos);

        // each postFwdMod modifies moves set in-place
        for (const auto post : postFwdMods) {
            (*post)(moves, b, piecePos);
        }
        return moves;
    };

    ::std::pair<::std::vector<BoardState<FS, NPDT>>, ::std::vector<CT>>
    getReversesWithDisplacement(const BoardState<FS, NPDT>& b, CT piecePos) const override {
        // prune illegal moves based on these starting conditions
        for (const auto pre : preBwdMods) {
            if (!(*pre)(b, piecePos)) {
                // return empty set
                return ::std::pair<::std::vector<BoardState<FS, NPDT>>, ::std::vector<CT>>();
            }
        }
        // main move functionality specified by subclasses
        auto moves = getModdableUnmovesWithDisplacement(b, piecePos);

        // each postFwdMod modifies moves set in-place
        for (const auto post : postBwdMods) {
            (*post)(moves, b, piecePos);
        }
        return moves;
    }

    ModdablePMO(
        std::vector<const PMOPreMod  <FS, NPDT, CT>*> _preFwdMods,
        std::vector<const PMOPostMod <FS, NPDT, CT>*> _postFwdMods,
        std::vector<const PMOPreMod  <FS, NPDT, CT>*> _preBwdMods,
        std::vector<const PMOPostMod <FS, NPDT, CT>*> _postBwdMods
    ) : preFwdMods(_preFwdMods), postFwdMods(_postFwdMods), preBwdMods(_preBwdMods), postBwdMods(_postBwdMods) { }

    ModdablePMO() { }

private:
    // TODO: should these be made constant? and if so, do we need a different data type than vector?
    const std::vector<const PMOPreMod <FS, NPDT, CT>*> preFwdMods;
    const std::vector<const PMOPostMod<FS, NPDT, CT>*> postFwdMods;
    const std::vector<const PMOPreMod <FS, NPDT, CT>*> preBwdMods;
    const std::vector<const PMOPostMod<FS, NPDT, CT>*> postBwdMods;
};
// TODO: generalize chess classes to interfaces that can be put here

#endif