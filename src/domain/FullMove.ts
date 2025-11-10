/*
* ===============================================================================
* File Name          : FullMove.ts
* Creation Date      : 2025-11-01
* Last Modified      : 
* Version            : 1.0.0
* License            : 
* Author             : Rigoberto L. Salgado Reyes
* Contact            : rlsalgado2006@gmail.com
* ===============================================================================
*/

import { Move } from "(src)/domain/Move";
import { PieceKind } from "(src)/domain/types";

class FullMove {
	private readonly _moves: Move[];
	private readonly _score: number;
	private readonly names: Record<number, string> = {1: "BLACK", 2: "WHITE", 3: "NEUTRON"};

	constructor(moves: Move[], score: number) {
		if (moves.length !== 4) throw new Error("A FullMove must contain exactly four moves");
		this._moves = moves;
		this._score = score;
	}

	get moves(): Move[] {
		return this._moves;
	}

	get score(): number {
		return this._score;
	}

	// noinspection JSUnusedGlobalSymbols
	clone() {
		const clonedMoves = this.moves.map(move => move.clone());
		return new FullMove(clonedMoves, this.score);
	}

	private kind2Name(pieceKind: PieceKind) {
		return this.names[pieceKind] || "NO KIND";
	}

	toString() {
		if (this.moves.length) {
			const piece1Kind = this.kind2Name(this.moves[1].kind);
			const piece2Kind = this.kind2Name(this.moves[3].kind);
			return `${piece1Kind}: ${this.moves[0].toString()}-${this.moves[1].toString()}, ` +
				`${piece2Kind}: ${this.moves[2].toString()}-${this.moves[3].toString()}`;
		}

		return `EMPTY FULLMOVE with score = ${this.score}`;
	}

	// noinspection JSUnusedGlobalSymbols
	toJSON() {
		return {
			__typename: "FullMove",
			moves: this.moves.map(move => move.toJSON()),
			score: this.score
		};
	}

	static fromJSON(json: any): FullMove {
		const moves = json.moves.map((moveJson: any) => Move.fromJSON(moveJson));
		return new FullMove(moves, json.score);
	}

	empty(): boolean {
		return !this.moves.length;
	}
}

export { FullMove };
