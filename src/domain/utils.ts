/*
* ===============================================================================
* File Name          : utils.ts
* Creation Date      : 2025-11-02
* Last Modified      : 
* Version            : 1.0.0
* License            : 
* Author             : Rigoberto L. Salgado Reyes
* Contact            : rlsalgado2006@gmail.com
* ===============================================================================
*/

import { PieceKind } from "(src)/domain/types";
import { Move } from "(src)/domain/Move";
import { FullMove } from "(src)/domain/FullMove";
import { GameState } from "(src)/domain/GameState";

export function getDefaultBoard(): PieceKind[] {
	return [
		PieceKind.BLACK, PieceKind.CELL, PieceKind.CELL, PieceKind.CELL, PieceKind.WHITE,       // col 1
		PieceKind.BLACK, PieceKind.CELL, PieceKind.CELL, PieceKind.CELL, PieceKind.WHITE,       // col 2
		PieceKind.BLACK, PieceKind.CELL, PieceKind.NEUTRON, PieceKind.CELL, PieceKind.WHITE,    // col 3
		PieceKind.BLACK, PieceKind.CELL, PieceKind.CELL, PieceKind.CELL, PieceKind.WHITE,       // col 4
		PieceKind.BLACK, PieceKind.CELL, PieceKind.CELL, PieceKind.CELL, PieceKind.WHITE        // col 5
	];
}

export function getReviver(typename: string) {
	return (_k: string, value: any) => {
		const tag = value?.__typename ?? value?.__type;
		if (tag === typename) {
			switch (typename) {
				case "Move":
					return Move.fromJSON(value);
				case "FullMove":
					return FullMove.fromJSON(value);
				case "GameState":
					return GameState.fromJSON(value);
			}
		}
		return value;
	};
}

