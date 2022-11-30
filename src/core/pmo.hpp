#ifndef PMO_H_
#define PMO_H_
// #include "piece_type.hpp" // caution! circularity approaching

#include "../utils/utils.h"
#include "../retrograde_analysis/state.hpp"
#include "coords_grid.hpp"
#include "promotion.h"
#include "ignore_macros.hpp"

// Shorthand: 
// FS = FlattenedSize
// NPDT = NonPlacementDataType
// CT = CoordsType

// Maximum number of pieces that can be on the board specified by user.
const size_t MAN_LIMIT = N_MAN;

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

// Guarantees getUnpromotions is implemented.
template<::std::size_t FS, typename NPDT, typename CT>
class PromotablePMO : public PMO<FS, NPDT, CT> {
public:
    // The same as getReverses, but exclusively used for unpromotions.
    // unpromotedLabel is the type we are unpromoting to, promotedLabel is the type we are unpromoting from.
    virtual ::std::vector<BoardState<FS, NPDT>> 
    getUnpromotions(const BoardState<FS, NPDT>& b, CT piecePos, piece_label_t unpromotedLabel, piece_label_t promotedLabel) const = 0;
};

// Any move that takes a piece and moves it somewhere else. Exposes displacement of moves to make complex move implementation easier.
template<::std::size_t FS, typename NPDT, typename CT>
class DisplacementPMO : public PromotablePMO<FS, NPDT, CT> {
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

    virtual ::std::vector<BoardState<FS, NPDT>> 
    getUnpromotions(const BoardState<FS, NPDT>& b, CT piecePos, piece_label_t unpromotedLabel, piece_label_t promotedLabel) const override {
        return getUnpromotionsWithDisplacement(b, piecePos, unpromotedLabel, promotedLabel).first;
    }

    virtual ::std::pair<::std::vector<BoardState<FS, NPDT>>, ::std::vector<CT>>
    getUnpromotionsWithDisplacement(const BoardState<FS, NPDT>& b, CT piecePos, piece_label_t unpromotedLabel, piece_label_t promotedLabel) const {
        auto moves = getReversesWithDisplacement(b, piecePos);
        for (size_t i = 0; i < moves.first.size(); ++i) {
            BoardState<FS, NPDT> &moveState = moves.first.at(i);
            CT moveDisplacement = moves.second.at(i);

            // just set the end of each unmove to be the unpromoted piece
            moveState.m_board.at((piecePos + moveDisplacement).flatten()) = unpromotedLabel;
        }
        return moves;
    };
};

template<::std::size_t FS, typename NPDT, typename CT>
class PMOPreMod {
public:
    // Return false if we should reject this PMO; return true if it passes this test (may still fail others).
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

template<typename CT>
using RegionEvalPtr = bool(*)(CT piecePos);

// prohibits moves using only piece color and starting position.
template<::std::size_t FS, typename NPDT, typename CT>
class DirectedRegionPMOPreMod : public PMOPreMod<FS, NPDT, CT> {
private:
    const RegionEvalPtr<CT> whiteEval;
    const RegionEvalPtr<CT> blackEval;
public:
    DirectedRegionPMOPreMod(RegionEvalPtr<CT> _whiteEval, RegionEvalPtr<CT> _blackEval) : whiteEval(_whiteEval), blackEval(_blackEval) { }

    virtual bool operator()(const BoardState<FS, NPDT>& b, CT piecePos) const override {
        bool pieceColor = isWhite(b.m_board.at(piecePos.flatten()));
        return (pieceColor? (*whiteEval)(piecePos) : (*blackEval)(piecePos));
    }
};


// Abstract class for any PMOPostMod that for each board, replaces it with a (possibly empty) set of modified boards.
template<::std::size_t FS, typename NPDT, typename CT>
class Replace1ToManyPMOPostMod : public PMOPostMod<FS, NPDT, CT> {
public:
    virtual void operator()(
            ::std::pair<::std::vector<BoardState<FS, NPDT>>, ::std::vector<CT>>& moves
            , const BoardState<FS, NPDT>& b, CT piecePos) const override {
        //TODO: implement if needed
    };
};

// Abstract class for any PMOPostMod that modifies each board independently of other boards or adding/removing number of boards.
template<::std::size_t FS, typename NPDT, typename CT>
class ModifyEachPMOPostMod : public PMOPostMod<FS, NPDT, CT> {
public:
    // Modify moveState and moveDisplacement, given knowledge of prior board b and its starting position piecePos
    virtual void modify(
            BoardState<FS, NPDT>& moveState, CT& moveDisplacement
            , const BoardState<FS, NPDT>& b, CT piecePos) const = 0;

    virtual void operator()(
            ::std::pair<::std::vector<BoardState<FS, NPDT>>, ::std::vector<CT>>& moves
            , const BoardState<FS, NPDT>& b, CT piecePos) const override {
        for (size_t i = 0; i < moves.first.size(); ++i) {
            BoardState<FS, NPDT> &moveState = moves.first.at(i);
            CT& moveDisplacement = moves.second.at(i);

            modify(moveState, moveDisplacement, b, piecePos);
        }
    };
};


// prohibits moves using only piece color and starting position.
// Assumes all promotions specified by promotionScheme.
// TODO: assumes only a single promotion type allowed in promotionScheme. Needs to extend Replace1ToManyPMOPostMod instead to do this. Beward losses in efficiency possible.
template<::std::size_t FS, typename NPDT, typename CT>
class RegionalForcedSinglePromotionPMOPostMod : public ModifyEachPMOPostMod<FS, NPDT, CT> {
private:
    const RegionEvalPtr<CT> whiteEval;
    const RegionEvalPtr<CT> blackEval;
public:
    // eval function parameters should return true for the zone the piece promotes in 
    RegionalForcedSinglePromotionPMOPostMod(RegionEvalPtr<CT> _whiteEval, RegionEvalPtr<CT> _blackEval) : whiteEval(_whiteEval), blackEval(_blackEval) { }

    virtual void modify(
            BoardState<FS, NPDT>& moveState, CT& moveDisplacement
            , const BoardState<FS, NPDT>& b, CT piecePos) const override {
        
        CT endPos = piecePos + moveDisplacement;
        piece_label_t unpromotedPiece = moveState.m_board.at(endPos.flatten());
        bool pieceColor = isWhite(unpromotedPiece);

        // if we are in the promotion zone of our color
        if (pieceColor? (*whiteEval)(endPos) : (*blackEval)(endPos)) {
            piece_label_t promotedPiece = promotionScheme.getPromotions(unpromotedPiece)[0];
            // literally just change end position to this piece
            moveState.m_board.at(endPos.flatten()) = promotedPiece;
        }
    }
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

    virtual ::std::vector<BoardState<FS, NPDT>> 
    getUnpromotions(const BoardState<FS, NPDT>& b, CT piecePos, piece_label_t unpromotedLabel, piece_label_t promotedLabel) const override {
        return getUnpromotionsWithDisplacement(b, piecePos, unpromotedLabel, promotedLabel).first;
    }

    virtual ::std::pair<::std::vector<BoardState<FS, NPDT>>, ::std::vector<CT>>
    getUnpromotionsWithDisplacement(const BoardState<FS, NPDT>& b, CT piecePos, piece_label_t unpromotedLabel, piece_label_t promotedLabel) const override {

        // prune illegal moves based on these starting conditions
        for (const auto pre : preUnpromotionMods) {
            if (!(*pre)(b, piecePos)) {
                // return empty set
                return ::std::pair<::std::vector<BoardState<FS, NPDT>>, ::std::vector<CT>>();
            }
        }
        // main move functionality specified by subclasses.
        auto moves = DisplacementPMO<FS, NPDT, CT>::getUnpromotionsWithDisplacement(b, piecePos, unpromotedLabel, promotedLabel);

        // each postFwdMod modifies moves set in-place
        for (const auto post : postUnpromotionMods) {
            (*post)(moves, b, piecePos);
        }
        return moves;
    };

    ModdablePMO() { }

    ModdablePMO(
        std::vector<const PMOPreMod  <FS, NPDT, CT>*> _preFwdMods,
        std::vector<const PMOPostMod <FS, NPDT, CT>*> _postFwdMods,
        std::vector<const PMOPreMod  <FS, NPDT, CT>*> _preBwdMods,
        std::vector<const PMOPostMod <FS, NPDT, CT>*> _postBwdMods
    ) : preFwdMods(_preFwdMods), postFwdMods(_postFwdMods), preBwdMods(_preBwdMods), postBwdMods(_postBwdMods) { }

    ModdablePMO(
        std::vector<const PMOPreMod  <FS, NPDT, CT>*> _preFwdMods,
        std::vector<const PMOPostMod <FS, NPDT, CT>*> _postFwdMods,
        std::vector<const PMOPreMod  <FS, NPDT, CT>*> _preBwdMods,
        std::vector<const PMOPostMod <FS, NPDT, CT>*> _postBwdMods,
        std::vector<const PMOPreMod  <FS, NPDT, CT>*> _preUnpromotionMods,
        std::vector<const PMOPostMod <FS, NPDT, CT>*> _postUnpromotionMods
    ) : preFwdMods(_preFwdMods), postFwdMods(_postFwdMods), preBwdMods(_preBwdMods), postBwdMods(_postBwdMods), preUnpromotionMods(_preUnpromotionMods), postUnpromotionMods(_postUnpromotionMods) { }

private:
    const std::vector<const PMOPreMod <FS, NPDT, CT>*> preFwdMods;
    const std::vector<const PMOPostMod<FS, NPDT, CT>*> postFwdMods;
    const std::vector<const PMOPreMod <FS, NPDT, CT>*> preBwdMods;
    const std::vector<const PMOPostMod<FS, NPDT, CT>*> postBwdMods;
    // Note: this is called before preBwdMods. Give this vector only premods unique to unpromotions
    const std::vector<const PMOPreMod <FS, NPDT, CT>*> preUnpromotionMods;
    // Note: this is called after postBwdMods. Give this vector only postmods unique to unpromotions. Do not need to specify the piece change as a postmod, already handled.
    const std::vector<const PMOPostMod<FS, NPDT, CT>*> postUnpromotionMods;
};

#endif