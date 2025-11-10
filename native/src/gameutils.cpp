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

#include <Move.h>
#include <gameutils.h>

#include <json.hpp>

using json = nlohmann::json;

int heuristic(const std::unique_ptr<Board> &board) {
    const auto neutron = board->findNeutron();

    if (neutron->row == 4)
        return std::numeric_limits<short>::min();
    if (neutron->row == 0)
        return std::numeric_limits<short>::max();

    const auto neutronMoves = board->moves(neutron);

    auto count = 0;
    for (const auto &neutronMove : neutronMoves) {
        if (neutronMove->row == 4)
            count += -5000;
        if (neutronMove->row == 0)
            count += 1000;
    }

    return count;
}

// Table *getTable(std::string &jsonStringTable) {
//     auto table = new Table();
//     auto j = 0;
//
//     auto boardJsonObj = json::parse(jsonStringTable);
//     auto board = boardJsonObj["board"].get<std::vector<std::vector<int>>>();
//
//     for (auto &row:board) {
//         Row r;
//         auto i = 0;
//         for (auto &e :row) {
//             r[i++] = intToPieceKind(e);
//         }
//         (*table)[j++] = r;
//     }
//
//     return table;
// }

PieceKind intToPieceKind(int piece) {
    if (piece == 1)
        return PieceKind::BLACK;
    if (piece == 2)
        return PieceKind::WHITE;
    if (piece == 3)
        return PieceKind::NEUTRON;
    return PieceKind::CELL;
}
