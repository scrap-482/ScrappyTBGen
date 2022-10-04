#include "game.h"
#include "constants.hpp"
#include "chess_utils.h"
#include "array_board.h"

Game::Game(const std::string _sfen, std::string _rulesFile) : m_rules(_rulesFile) {
    reset(_sfen);
}

void Game::reset(std::string _sfen) {
    BoardPrintSettings ps;
    if (m_board != nullptr) { // Copy over the print settings of the last board, if available
        ps = m_board->m_printSettings;
    }
    auto fields = split(_sfen, "\\s+");

    // TODO: cleanly handle all these errors
    if (fields.size() == 5) { // whole sfen is given
        dlog("got 5 fields");
        m_board = new ArrayBoard(m_rules, fields[0], algebraicToCoords(fields[1]));
        // TODO: parse minCorner
        if (fields[2] == "w") {
            m_turn = WHITE;
        } else if (fields[2] == "b") {
            m_turn = BLACK;
        } else {
            dlog("ERROR! unknown color [",fields[2],"] in SFEN [", _sfen,"]");
        }

        // TODO: check these are actually numbers
        m_50Count = std::stoi(fields[3]);
        m_moveCount = std::stoi(fields[4]);
    } else if (fields.size() == 1) { // if given just position, we'll just assume this is start of new game
        m_board = new ArrayBoard(m_rules, fields[0], DModCoords(0, 0)); // default to a0
        m_turn = WHITE;
        m_50Count = 0;
        m_moveCount = 1;
    } else { 
        dlog("ERROR! invalid number of fields in SFEN [", _sfen, "]");
    }

    m_board->m_printSettings = ps;

    dlog("Game Constructed board");
}

std::string Game::toSfen() const {
    return m_board->toSfen() + " " + 
    // minCorner already included in Board->toSfen.
    (m_turn == WHITE? "w" : "b") + " " +
    std::to_string(m_50Count) + " " +
    std::to_string(m_moveCount);
}


std::string Game::print() {
    std::string result = m_board->getAsciiBoard();
    result += m_board->toSfen() + "\n";
    if (!m_moveHistory.empty()) {
        result += "Last move: " + m_moveHistory.top()->algebraic() + "\n";
    }
    result += "Moves since last capture: " + std::to_string(m_50Count) + "\n";
    result += "Estimated score for this position: " + std::to_string(m_board->staticEvaluation()) + "\n";
    result += m_turn == WHITE? "White's Turn\n" : "Black's Turn\n";
    return result;
}

bool Game::applyMove(std::unique_ptr<Move> _move){
    if (!m_board->apply(*_move)) {
        return false;
    }
    // increment full move count when black plays
    if (m_turn == BLACK) {
        m_moveCount++;
    }
    m_turn = -m_turn;
    if ((_move->m_type == PIECE_MOVE) && (static_cast<PieceMove*>(_move.get())->m_capture != EMPTY)) {
        m_50Count = 0;
    } else {
        m_50Count++;
    }
    m_moveHistory.push(std::move(_move)); // WARNING: _move is now nullptr
    return true;
};

std::unique_ptr<Move> Game::undoMove() {
    std::unique_ptr<Move> undone = std::move(m_moveHistory.top());
    if (!m_board->undo(*undone)) {
        // TODO: this is weird behavior for when undo fails, need better error handling
        return std::unique_ptr<Move>(nullptr);
    }
    m_moveHistory.pop();
    m_turn = -m_turn;
    // decrement full move count when undoing black's move
    if (m_turn == BLACK) {
        m_moveCount--;
    }
    // TODO: how do we restore moveSinceLastCapture?
    return std::move(undone);
};
