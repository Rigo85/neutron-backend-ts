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

#include <FullMove.h>

FullMove::FullMove(const std::initializer_list<Move> moves, const int score) {
    this->moves.reserve(moves.size());
    for (auto &m : moves) this->moves.emplace_back(std::make_unique<Move>(m));
    this->score = score;
}

FullMove::FullMove(std::vector<std::unique_ptr<Move>> &&mv, const int s) : moves(std::move(mv)), score(s) {
}

FullMove::FullMove(const FullMove &other) : score(other.score) {
    moves.reserve(other.moves.size());
    for (const auto &m : other.moves) {
        moves.emplace_back(std::make_unique<Move>(*m));
    }
}

FullMove &FullMove::operator=(const FullMove &other) {
    if (this != &other) {
        std::vector<std::unique_ptr<Move>> tmp;
        tmp.reserve(other.moves.size());
        for (const auto &m : other.moves) {
            tmp.emplace_back(std::make_unique<Move>(*m));
        }
        moves = std::move(tmp);
        score = other.score;
    }
    return *this;
}

std::unique_ptr<FullMove> FullMove::clone() const {
    return std::make_unique<FullMove>(*this);
}

std::string FullMove::kind2Name(PieceKind &pieceKind) const {
    switch (pieceKind) {
        case PieceKind::BLACK:
            return "BLACK";
        case PieceKind::WHITE:
            return "WHITE";
        case PieceKind::NEUTRON:
            return "NEUTRON";
        default:
            return "NO KIND";
    }
}

bool FullMove::empty() const {
    return moves.empty();
}

// std::ostream &operator<<(std::ostream &ostr, const FullMove &fullMove) {
//     if (!fullMove.empty()) {
//         auto piece1Kind = fullMove.kind2Name(fullMove.moves->at(1)->kind);
//         auto piece2Kind = fullMove.kind2Name(fullMove.moves->at(3)->kind);
//         ostr << piece1Kind << ": " << *fullMove.moves->at(0) << "-" << *fullMove.moves->at(1) << " ";
//         ostr << piece2Kind << ": " << *fullMove.moves->at(2) << "-" << *fullMove.moves->at(3);
//
//         return ostr;
//     }
//
//     ostr << "EMPTY FULLMOVE with score = " << fullMove.score;
//     return ostr;
// }
