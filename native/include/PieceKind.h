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

#pragma once

#include <cstdint>

enum class PieceKind: uint8_t {
    BLACK = 1,
    WHITE = 2,
    NEUTRON = 3,
    CELL = 4,
    SBLACK = 5,
    SWHITE = 6,
    SCELL = 7,
    SNEUTRON = 8
};
