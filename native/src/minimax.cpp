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

#include <PieceKind.h>
#include <gameutils.h>
#include <minimax.h>

#include <limits>

std::unique_ptr<FullMove> maxValue(std::unique_ptr<Board>& board, const int depth, const int alpha, const int beta, const PieceKind player) {
    const auto neutron = board->findNeutron();

    if (!depth || neutron->row == 0 || neutron->row == 4) {
        return std::make_unique<FullMove>(std::vector<std::unique_ptr<Move>>(), heuristic(board));
    }

    const auto fullMoves = board->allMoves(player);

    auto maxFullMove = std::make_unique<FullMove>(std::vector<std::unique_ptr<Move>>(), alpha);

    for (const auto& fullMove : fullMoves) {
        board->applyFullMove(fullMove);

        const auto minFullMove = minValue(                                    //
            board,                                                            //
            depth - 1,                                                        //
            maxFullMove->score,                                               //
            beta,                                                             //
            player == PieceKind::BLACK ? PieceKind::WHITE : PieceKind::BLACK  //
        );

        if (minFullMove->score > maxFullMove->score) {
            *maxFullMove = *fullMove;
            maxFullMove->score = minFullMove->score;
        }

        board->applyFullMove(fullMove, false);

        if (maxFullMove->score >= beta) {
            fullMove->score = beta;
            return fullMove->clone();
        }
    }

    if (maxFullMove->empty() && !fullMoves.empty()) {
        auto tmp = std::make_unique<FullMove>(std::vector<std::unique_ptr<Move>>(), std::numeric_limits<int>::min());
        for (const auto& fullMove : fullMoves) {
            board->applyFullMove(fullMove);
            const auto h = heuristic(board);
            board->applyFullMove(fullMove, false);
            fullMove->score = h;

            if (fullMove->score > tmp->score) {
                *tmp = *fullMove;
            }
        }

        return tmp;
    } else {
        return maxFullMove;
    }
}

std::unique_ptr<FullMove> minValue(std::unique_ptr<Board>& board, const int depth, const int alpha, const int beta, const PieceKind player) {
    const auto neutron = board->findNeutron();

    if (!depth || neutron->row == 0 || neutron->row == 4) {
        return std::make_unique<FullMove>(std::vector<std::unique_ptr<Move>>(), heuristic(board));
    }

    const auto fullMoves = board->allMoves(player);

    auto minFullMove = std::make_unique<FullMove>(std::vector<std::unique_ptr<Move>>(), beta);

    for (const auto& fullMove : fullMoves) {
        board->applyFullMove(fullMove);

        const auto maxFullMove = maxValue(                                    //
            board,                                                            //
            depth - 1,                                                        //
            alpha,                                                            //
            minFullMove->score,                                               //
            player == PieceKind::BLACK ? PieceKind::WHITE : PieceKind::BLACK  //
        );

        if (maxFullMove->score < minFullMove->score) {
            *minFullMove = *fullMove;
            minFullMove->score = maxFullMove->score;
        }

        board->applyFullMove(fullMove, false);

        if (alpha >= minFullMove->score) {
            fullMove->score = alpha;
            return fullMove->clone();
        }
    }

    if (minFullMove->empty() && !fullMoves.empty()) {
        auto tmp = std::make_unique<FullMove>(std::vector<std::unique_ptr<Move>>(), std::numeric_limits<int>::min());
        for (const auto& fullMove : fullMoves) {
            board->applyFullMove(fullMove);
            const auto h = heuristic(board);
            board->applyFullMove(fullMove, false);
            fullMove->score = h;

            if (fullMove->score < tmp->score) {
                *tmp = *fullMove;
            }
        }

        return tmp;
    } else {
        return minFullMove;
    }
}
