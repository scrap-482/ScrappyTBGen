#ifndef PMO_MODS_H_
#define PMO_MODS_H_

#include "pmo_moddable.hpp"

// Shorthand: 
// FS = FlattenedSize
// NPDT = NonPlacementDataType
// CT = CoordsType
// PTC = "Piece Type Count" AKA NumPieceTypes // TODO: bad naming, refactor?

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

#endif