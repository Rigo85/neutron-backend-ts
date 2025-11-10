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

#pragma once

#include <Move.h>
#include <PieceKind.h>

#include <iostream>
#include <memory>
#include <vector>

class FullMove final {
   public:
    FullMove() = default;
    FullMove(std::initializer_list<Move> moves, int score);
    FullMove(std::vector<std::unique_ptr<Move>> &&mv, int s);
    FullMove(const FullMove &other);
    FullMove &operator=(const FullMove &other);

    std::string kind2Name(PieceKind &pieceKind) const;
    [[nodiscard]] bool empty() const;
    [[nodiscard]] std::unique_ptr<FullMove> clone() const;
    // friend std::ostream &operator<<(std::ostream &ostr, const FullMove &fullMove);

    std::vector<std::unique_ptr<Move>> moves;
    int score{0};
};
