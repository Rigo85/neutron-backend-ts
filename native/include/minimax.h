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

#include <FullMove.h>
#include <Board.h>

std::unique_ptr<FullMove> maxValue(std::unique_ptr<Board> &board, int depth, int alpha, int beta, PieceKind player);

std::unique_ptr<FullMove> minValue(std::unique_ptr<Board> &board, int depth, int alpha, int beta, PieceKind player);
