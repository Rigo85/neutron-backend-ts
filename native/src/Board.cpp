/**
 * Authors:
 * Rigoberto Leander Salgado Reyes <rlsalgado2006@gmail.com>
 *
 * Copyright 2018 by Rigoberto Leander Salgado Reyes.
 *
 * This program is licensed to you under the terms of version 3 of the
 * GNU Affero General Public License. This program is distributed WITHOUT
 * ANY EXPRESS OR IMPLIED WARRANTY, INCLUDING THOSE OF NON-INFRINGEMENT,
 * MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE. Please refer to the
 * AGPL (http://www.gnu.org/licenses/agpl-3.0.txt) for more details.
 *
 * Modified 2025 by Rigoberto Leander Salgado Reyes.
 */

#include <Board.h>
#include <PieceKind.h>
#include <cleaners.h>

#include <algorithm>
#include <cstdlib>
#include <functional>
#include <limits>

Board::Board(const std::array<uint8_t, 25> &ptable) : table(ptable) {
}

Board::~Board() = default;

std::unique_ptr<Move> Board::findNeutron() const {
    int index = 0;
    for (const auto cell : this->table) {
        const int col = index / 5;
        const int row = index % 5;
        if (cell == static_cast<uint8_t>(PieceKind::NEUTRON)) {
            return std::make_unique<Move>(row, col, PieceKind::NEUTRON);
        }

        index++;
    }

    return nullptr;
}

std::vector<std::unique_ptr<Move>> Board::findPieces(PieceKind pieceKind) const {
    int index = 0;
    std::vector<std::unique_ptr<Move>> pos;
    pos.reserve(5);

    for (const auto &cell : this->table) {
        const int col = index / 5;
        const int row = index % 5;
        if (cell == static_cast<uint8_t>(pieceKind)) {
            pos.emplace_back(std::make_unique<Move>(row, col, pieceKind));
        }

        index++;
    }

    return pos;
}

int Board::getRowMove(const Direction direction) {
    switch (direction) {
        case Direction::NORTH:
        case Direction::NORTHEAST:
        case Direction::NORTHWEST:
            return -1;
        case Direction::SOUTH:
        case Direction::SOUTHEAST:
        case Direction::SOUTHWEST:
            return 1;
        default:
            return 0;
    }
}

int Board::getColMove(const Direction direction) {
    switch (direction) {
        case Direction::WEST:
        case Direction::NORTHWEST:
        case Direction::SOUTHWEST:
            return -1;
        case Direction::EAST:
        case Direction::NORTHEAST:
        case Direction::SOUTHEAST:
            return 1;
        default:
            return 0;
    }
}

PieceKind Board::elementAt(const int row, const int col) const {
    return static_cast<PieceKind>(this->table[col * 5 + row]);
}

void Board::setElementAt(const int row, const int col, PieceKind pieceKind) {
    this->table[col * 5 + row] = static_cast<uint8_t>(pieceKind);
}

bool Board::inBounds(const int value, const int inc) {
    return value + inc >= 0 && value + inc < 5;
}

std::unique_ptr<Move> Board::_check(const int row, const int col, const int incR, const int incC) const {
    if (!inBounds(row, incR) ||  //
        !inBounds(col, incC) ||  //
        elementAt(row + incR, col + incC) != PieceKind::CELL) {
        return std::make_unique<Move>(row, col, elementAt(row, col));
    }
    return _check(row + incR, col + incC, incR, incC);
}

std::unique_ptr<Move> Board::checkMove(const std::unique_ptr<Move> &move, const Direction direction) const {
    const auto tip = _check(move->row, move->col, getRowMove(direction), getColMove(direction));

    if (move->row == tip->row && move->col == tip->col)
        return nullptr;
    return std::make_unique<Move>(tip->row, tip->col, move->kind);
}

std::vector<std::unique_ptr<Move>> Board::moves(const std::unique_ptr<Move> &startPoint) const {
    std::vector<std::unique_ptr<Move>> result;
    result.reserve(8);

    constexpr Direction directions[8] = {Direction::NORTH,  //
                                         Direction::SOUTH,  //
                                         Direction::EAST,   //
                                         Direction::WEST,   //
                                         Direction::NORTHEAST,
                                         Direction::NORTHWEST,  //
                                         Direction::SOUTHEAST,  //
                                         Direction::SOUTHWEST};

    for (const auto d : directions) {
        if (auto m = checkMove(startPoint, d))
            result.emplace_back(std::move(m));
    }

    return result;
}

void Board::applyMove(const std::unique_ptr<Move> &from, const std::unique_ptr<Move> &to) {
    setElementAt(to->row, to->col, to->kind);
    if (from->col * 5 + from->row != to->col * 5 + to->row) {
        setElementAt(from->row, from->col, PieceKind::CELL);
    }
}

void Board::applyFullMove(const std::unique_ptr<FullMove> &fullMove, const bool apply) {
    applyMove(fullMove->moves.at(apply ? 0 : 3), fullMove->moves.at(apply ? 1 : 2));
    applyMove(fullMove->moves.at(apply ? 2 : 1), fullMove->moves.at(apply ? 3 : 0));
}

std::vector<std::unique_ptr<FullMove>> Board::allMoves(const PieceKind pieceKind) {
    auto neutron = this->findNeutron();
    auto lastNeutron = std::make_unique<Move>(*neutron);
    const auto playerHome = pieceKind == PieceKind::BLACK ? 0 : 4;
    const auto opponentHome = pieceKind == PieceKind::BLACK ? 4 : 0;

    // eliminar movimientos perdedores.
    auto neutronMoves = this->moves(neutron);

    std::erase_if(                                                              //
        neutronMoves,                                                           //
        [&](const std::unique_ptr<Move> &m) { return m->row == opponentHome; }  //
    );

    // sí aparece un movimiento ganador, descartar el resto.
    const auto itWin = std::ranges::find_if(                     //
        neutronMoves,                                            //
        [&playerHome](auto &m) { return m->row == playerHome; }  //
    );

    if (itWin != neutronMoves.end()) {
        auto neutronMove = std::make_unique<Move>(**itWin);
        neutronMoves.clear();
        neutronMoves.emplace_back(std::move(neutronMove));
    }

    auto pieces = this->findPieces(pieceKind);

    std::vector<std::unique_ptr<FullMove>> allFullMoves;
    // tamaño máximo teórico: (<=8 neutrón) * (<=5 piezas * <=8 mov) ≈ 320
    allFullMoves.reserve(256);

    for (const auto &neutronMove : neutronMoves) {
        this->applyMove(lastNeutron, neutron);
        this->applyMove(neutron, neutronMove);
        lastNeutron = std::make_unique<Move>(*neutronMove);

        for (const auto &piece : pieces) {
            for (auto moves = this->moves(piece); const auto &pieceMove : moves) {
                FullMove fullMove(
                    {
                        Move(neutron->row, neutron->col, neutron->kind),              //
                        Move(neutronMove->row, neutronMove->col, neutronMove->kind),  //
                        Move(piece->row, piece->col, piece->kind),                    //
                        Move(pieceMove->row, pieceMove->col, pieceMove->kind)         //
                    },
                    0);

                allFullMoves.push_back(std::make_unique<FullMove>(std::move(fullMove)));
            }
        }
    }

    this->applyMove(lastNeutron, neutron);

    return allFullMoves;
}

// std::ostream &operator<<(std::ostream &ostr, const Board &board) {
//     auto r = 5;
//     for (auto &row : *board.table) {
//         ostr << r-- << " ||";
//         for (auto &e : row) {
//             ostr << board.pieceToString(e) << "|";
//         }
//         ostr << "|" << std::endl;
//     }
//     ostr << "    - - - - -" << std::endl;
//     ostr << "    A B C D E" << std::endl;
//
//     return ostr;
// }

// std::string Board::pieceToString(PieceKind pieceKind) const {
//     if (pieceKind == PieceKind::BLACK)
//         return "B";
//     if (pieceKind == PieceKind::WHITE)
//         return "W";
//     if (pieceKind == PieceKind::NEUTRON)
//         return "N";
//     return " ";
// }
