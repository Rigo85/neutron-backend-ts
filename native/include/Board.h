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

#include <Direction.h>
#include <FullMove.h>
#include <Move.h>
#include <PieceKind.h>

#include <array>

using Row = std::array<PieceKind, 5>;
using Table = std::array<Row, 5>;

class Board {
   public:
    explicit Board(const std::array<uint8_t, 25> &table);
    ~Board();

    [[nodiscard]] std::unique_ptr<Move> findNeutron() const;

    [[nodiscard]] std::vector<std::unique_ptr<FullMove>> allMoves(PieceKind pieceKind);

    [[nodiscard]] std::vector<std::unique_ptr<Move>> moves(const std::unique_ptr<Move> &startPoint) const;

    [[nodiscard]] std::vector<std::unique_ptr<Move>> findPieces(PieceKind pieceKind) const;

    void applyFullMove(const std::unique_ptr<FullMove> &fullMove, bool apply = true);

    // friend std::ostream &operator<<(std::ostream &ostr, const Board &board);

   private:
    static int getRowMove(Direction direction);

    static int getColMove(Direction direction);

    static bool inBounds(int value, int inc);

    [[nodiscard]] std::unique_ptr<Move> checkMove(const std::unique_ptr<Move> &move, const Direction direction) const;

    [[nodiscard]] std::unique_ptr<Move> _check(int row, int col, int incR, int incC) const;

    void applyMove(const std::unique_ptr<Move> &from, const std::unique_ptr<Move> &to);

    // std::string pieceToString(PieceKind pieceKind) const;

    [[nodiscard]] PieceKind elementAt(int row, int col) const;

    void setElementAt(int row, int col, PieceKind pieceKind);

    std::array<uint8_t, 25> table{};
};
