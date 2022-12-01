#ifndef INTERFACE_LOGIC_H_
#define INTERFACE_LOGIC_H_

#include "pmo_moddable.hpp"
#include "../retrograde_analysis/state.hpp"

// Looping over all PMOs is common functionality, but what to do with them varies; as such, this is implemented as a 
// template function.
// actOnPMO will act on every move (except unpromotions), while actOnUnpromotionPMO acts only on unpromotions.
// Set reverse=true to flip the color of who we are generating moves for (which is needed for all backwards moves).
// ForEachPMOFunc is assumed to have the type:
//     (std::vector<BoardState<FS, NPDT>> newMoves, const BoardState<FS, NPDT>& b) -> bool
// ForEachPMOUnpromotionFunc is assumed to have the type:
//     (std::vector<BoardState<FS, NPDT>> newMoves, const BoardState<FS, NPDT>& b, piece_label_t unpromotedPlt, piece_label_t thisPiece) -> bool
// Can generate all PMOs for the player whose turn it is (forPlayerToMove=true) or for the other.
template <::std::size_t FS, typename NPDT, typename CT, ::std::size_t  PTC, typename ForEachPMOFunc>
void loopAllPMOs(const BoardState<FS, NPDT>& b, ForEachPMOFunc actOnPMO, bool reverse=false) {
    const auto noopLambda = [&](const BoardState<FS, NPDT>& b, const PMO<FS, NPDT, CT>* pmo, size_t flatStartPos, piece_label_t unpromoted, piece_label_t promoted) { return false; };
    loopAllPMOs<FS, NPDT, CT, PTC, ForEachPMOFunc, decltype(noopLambda) >(b, actOnPMO, reverse, noopLambda);
}
template <::std::size_t FS, typename NPDT, typename CT, ::std::size_t PTC, typename ForEachPMOFunc, typename ForEachPMOUnpromotionFunc>
void loopAllPMOs(const BoardState<FS, NPDT>& b, ForEachPMOFunc actOnPMO, bool reverse, ForEachPMOUnpromotionFunc actOnUnpromotionPMO) {
    // TODO: consider using a list of the positions of every type of piece, rather than brute-force checking all tiles
    for (size_t flatStartPos = 0; flatStartPos < 64; ++flatStartPos) {
        piece_label_t thisPiece = b.m_board.at(flatStartPos);
        if (isEmpty(thisPiece)) continue;
        // Ignore this piece if it is not for the player-to-move. Invert this result if forPlayerToMove=false.
        if (isWhite(thisPiece) ^ b.m_player ^ reverse) continue;

        PIECE_TYPE_ENUM type = getTypeEnumFromPieceLabel(thisPiece);

        for (size_t i = 0; i < getPieceTypeData<64, NPDT, CT>(type).pmoListSize; ++i) {
            auto pmo = getPieceTypeData<64, NPDT, CT>(type).pmoList[i];
            // Break if function returns false
            if (!actOnPMO(b, pmo, flatStartPos)) return;
        }

        // Now check for unpromotion PMOs
        if (reverse) {
            for (auto unpromotedPlt : promotionScheme.getUnpromotions(thisPiece)) {
                PIECE_TYPE_ENUM unpromotedType = getTypeEnumFromPieceLabel(unpromotedPlt);

                for (size_t i = 0; i < getPieceTypeData<64, NPDT, CT>(unpromotedType).pmoListSize; ++i) {
                    // ASSUMPTION: every PMO held by a promotable piece is a PromotablePMO.
                    auto pmo = (PromotablePMO<FS, NPDT, CT, PTC>*) getPieceTypeData<64, NPDT, CT>(unpromotedType).pmoList[i];
                    // Break if function returns false
                    if (!actOnUnpromotionPMO(b, pmo, flatStartPos, unpromotedPlt, thisPiece)) return;
                }
            }
        }
    }
    return;
}


#endif