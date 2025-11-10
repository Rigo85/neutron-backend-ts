/*
* ===============================================================================
* File Name          : GameState.ts
* Creation Date      : 2025-11-01
* Last Modified      : 
* Version            : 1.0.0
* License            : 
* Author             : Rigoberto L. Salgado Reyes
* Contact            : rlsalgado2006@gmail.com
* ===============================================================================
*/

import { v4 as uuid4 } from "uuid";

import { PieceKind } from "(src)/domain/types";
import { FullMove } from "(src)/domain/FullMove";
import { Move } from "(src)/domain/Move";
import { getDefaultBoard } from "(src)/domain/utils";

const defaultMovements: FullMove[] = [];

class GameState {
	private readonly _id: string;
	private readonly _board: PieceKind[];
	private readonly _movements: FullMove[];
	private _whoMove: number = 0;
	private _selectedChip?: Move;
	private _neutronFrom?: Move;
	private _neutronTo?: Move;
	private _version: number = -1;
	private _difficulty: number = 2;

	constructor(
		id: string = uuid4(),
		board: PieceKind[] = getDefaultBoard(),
		movements: FullMove[] = defaultMovements,
		whoMove: number = 0,
		selectedChip?: Move,
		neutronFrom?: Move,
		neutronTo?: Move,
		version: number = -1,
		difficulty: number = 2
	) {
		this._id = id;
		this._board = board;
		this._movements = movements;
		this._whoMove = whoMove;
		this._selectedChip = selectedChip;
		this._neutronFrom = neutronFrom;
		this._neutronTo = neutronTo;
		this._version = version;
		this._difficulty = difficulty;
	}

	get difficulty(): number {
		return this._difficulty;
	}

	set difficulty(value: number) {
		this._difficulty = value;
	}

	get id(): string {
		return this._id;
	}

	get board(): PieceKind[] {
		return this._board;
	}

	elementAt(row: number, col: number): PieceKind {
		return this._board[col * 5 + row];
	}

	setElementAt(row: number, col: number, kind: PieceKind): void {
		this._board[col * 5 + row] = kind;
	}

	get movements(): FullMove[] {
		return this._movements;
	}

	get whoMove(): number {
		return this._whoMove;
	}

	set whoMove(value: number) {
		this._whoMove = value;
	}

	get selectedChip(): Move | undefined {
		return this._selectedChip;
	}

	set selectedChip(value: Move | undefined) {
		this._selectedChip = value;
	}

	get neutronFrom(): Move | undefined {
		return this._neutronFrom;
	}

	set neutronFrom(value: Move | undefined) {
		this._neutronFrom = value;
	}

	get neutronTo(): Move | undefined {
		return this._neutronTo;
	}

	set neutronTo(value: Move | undefined) {
		this._neutronTo = value;
	}

	get version(): number {
		return this._version;
	}

	set version(value: number) {
		this._version = value;
	}

	updateVersion() {
		this._version += 1;
	}

	// noinspection JSUnusedGlobalSymbols
	toJSON() {
		return {
			__typename: "GameState",
			id: this._id,
			board: this._board,
			movements: this._movements,
			whoMove: this._whoMove,
			selectedChip: this._selectedChip?.toJSON(),
			neutronFrom: this._neutronFrom?.toJSON(),
			neutronTo: this._neutronTo?.toJSON(),
			version: this._version,
			difficulty: this._difficulty
		};
	}

	static fromJSON(json: any): GameState {
		const id: string = json.id;
		const board: PieceKind[] = json.board;
		const movements: FullMove[] = json.movements.map((m: any) => FullMove.fromJSON(m));
		const whoMove: number = json.whoMove;
		const selectedChip: Move | undefined = json.selectedChip ? Move.fromJSON(json.selectedChip) : undefined;
		const neutronFrom: Move | undefined = json.neutronFrom ? Move.fromJSON(json.neutronFrom) : undefined;
		const neutronTo: Move | undefined = json.neutronTo ? Move.fromJSON(json.neutronTo) : undefined;
		const version = json.version;
		const difficulty = json.difficulty;

		return new GameState(id, board, movements, whoMove, selectedChip, neutronFrom, neutronTo, version, difficulty);
	}
}

export { GameState };
