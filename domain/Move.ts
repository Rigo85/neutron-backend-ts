/*
* ===============================================================================
* File Name          : Move.ts
* Creation Date      : 2025-11-01
* Last Modified      : 
* Version            : 1.0.0
* License            : 
* Author             : Rigoberto L. Salgado Reyes
* Contact            : rlsalgado2006@gmail.com
* ===============================================================================
*/
import { PieceKind } from "(src)/domain/types";

const pieceKindToStringMapping: { [key: number]: string } = {
	[PieceKind.BLACK]: "BLACK",
	[PieceKind.WHITE]: "WHITE",
	[PieceKind.NEUTRON]: "NEUTRON",
	[PieceKind.CELL]: "CELL",
	[PieceKind.SBLACK]: "SBLACK",
	[PieceKind.SWHITE]: "SWHITE",
	[PieceKind.SCELL]: "SCELL",
	[PieceKind.SNEUTRON]: "SNEUTRON"
};

const stringToPieceKindMapping: { [key: string]: PieceKind } = {
	"BLACK": PieceKind.BLACK,
	"WHITE": PieceKind.WHITE,
	"NEUTRON": PieceKind.NEUTRON,
	"CELL": PieceKind.CELL,
	"SBLACK": PieceKind.SBLACK,
	"SWHITE": PieceKind.SWHITE,
	"SCELL": PieceKind.SCELL,
	"SNEUTRON": PieceKind.SNEUTRON
};

const chars = ["a", "b", "c", "d", "e"];

class Move {
	private readonly _row: number;
	private readonly _col: number;
	private readonly _kind: PieceKind;

	constructor(row: number, col: number, kind: PieceKind | undefined) {
		if (row < 0 || row > 4) throw new Error("Invalid row");
		if (col < 0 || col > 4) throw new Error("Invalid col");
		if (!kind) throw new Error("Invalid piece kind");
		this._row = row;
		this._col = col;
		this._kind = kind;
	}

	get row(): number {
		return this._row;
	}

	get col(): number {
		return this._col;
	}

	get kind(): PieceKind {
		return this._kind;
	}

	clone() {
		return new Move(this.row, this.col, this.kind);
	}

	toString() {
		return `${chars[this.col]}${5 - this.row}`;
	}

	toJSON() {
		if (!pieceKindToStringMapping[this._kind]) {
			throw new Error("Invalid piece kind to serialize");
		}

		return {
			__typename: "Move",
			row: this.row,
			col: this.col,
			kind: pieceKindToStringMapping[this.kind]
		};
	}

	static fromJSON(json: { row: number; col: number; kind: string }): Move {
		if (!stringToPieceKindMapping[json.kind]) {
			throw new Error("Invalid piece kind to deserialize");
		}

		return new Move(json.row, json.col, stringToPieceKindMapping[json.kind]);
	}

}

export { Move };
