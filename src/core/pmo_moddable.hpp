#ifndef PMO_PROMOTABLE_H_
#define PMO_PROMOTABLE_H_

#include "pmo.hpp"
#include "piece_type.hpp"

// Shorthand: 
// FS = FlattenedSize
// NPDT = NonPlacementDataType
// CT = CoordsType
// PTC = "Piece Type Count" AKA NumPieceTypes // TODO: bad naming, refactor?

// Guarantees getUnpromotions is implemented.
template<::std::size_t FS, typename NPDT, typename CT, ::std::size_t PTC>
class PromotablePMO : public PMO<FS, NPDT, CT> {
public:
    // The same as getReverses, but exclusively used for unpromotions.
    // unpromotedLabel is the type we are unpromoting to, promotedLabel is the type we are unpromoting from.
    virtual ::std::vector<BoardState<FS, NPDT>> 
    getUnpromotions(const BoardState<FS, NPDT>& b, CT piecePos, piece_label_t unpromotedLabel, piece_label_t promotedLabel) const = 0;
};

// Any move that takes a piece and moves it somewhere else. Exposes displacement of moves to make complex move implementation easier.
template<::std::size_t FS, typename NPDT, typename CT, ::std::size_t PTC>
class DisplacementPMO : public PromotablePMO<FS, NPDT, CT, PTC> {
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

template<::std::size_t FS, typename NPDT, typename CT, ::std::size_t PTC>
class ModdablePMO : public DisplacementPMO<FS, NPDT, CT, PTC> {
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
        auto moves = DisplacementPMO<FS, NPDT, CT, PTC>::getUnpromotionsWithDisplacement(b, piecePos, unpromotedLabel, promotedLabel);

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