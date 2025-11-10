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
 */

#include <string>

#include <Move.h>

Move::Move(int row, int col, PieceKind kind) {
    this->kind = kind;
    this->row = row;
    this->col = col;
}

Move *Move::clone() {
    return new Move(row, col, kind);
}

std::ostream &operator<<(std::ostream &ostr, const Move &move) {
    char chars[] = {'a', 'b', 'c', 'd', 'e'};
    ostr << chars[move.col] << 5 - move.row;
    return ostr;
}

std::string Move::toJson() {
    std::string str;

    return str + "{\"row\":" + std::to_string(row) +
           ", \"col\":" + std::to_string(col) +
           ", \"kind\":" + std::to_string(static_cast<int>(kind)) + "}";
}
