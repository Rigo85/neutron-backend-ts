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

#include <cleaners.h>

void clean(FullMove *fullMove){
    delete fullMove;
}

void clean(Move *move){
    delete move;
}

void clean(std::vector<FullMove *> *fullMoves){
    while (!fullMoves->empty()) {
        delete *fullMoves->begin();
        fullMoves->erase(fullMoves->begin());
    }
    delete fullMoves;
}

void clean(std::vector<Move *> *moves){
    while (!moves->empty()) {
        delete *moves->begin();
        moves->erase(moves->begin());
    }
    delete moves;
}

