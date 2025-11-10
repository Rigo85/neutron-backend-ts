/*
* ===============================================================================
* File Name          : engine.ts
* Creation Date      : 2025-11-01
* Last Modified      : 
* Version            : 1.0.0
* License            : 
* Author             : Rigoberto L. Salgado Reyes
* Contact            : rlsalgado2006@gmail.com
* ===============================================================================
*/
import { Direction, PieceKind } from "(src)/domain/types";
import { GameState } from "(src)/domain/GameState";
import { Move } from "(src)/domain/Move";

import path from "path";
import { FullMove } from "(src)/domain/FullMove";
import { logger } from "(src)/infra/logger";

type NativeMove = { row: number; col: number; kind: number };
type NativeOutput = { moves: NativeMove[]; score: number };

const addon: { minimaxAsync(input: { board: Uint8Array; depth: number }): Promise<NativeOutput> } =
	require(path.join(__dirname, "..", "..", "native", "build", "Release", "neutron_minimax.node"));

export function nativeMinimax(input: { board: Uint8Array; depth: number }): Promise<NativeOutput> {
	return addon.minimaxAsync(input);
}

const rotation = [PieceKind.NEUTRON, PieceKind.WHITE];

const mappingForCleaningBoard: Record<PieceKind, PieceKind> = {
	[PieceKind.SBLACK]: PieceKind.BLACK,
	[PieceKind.SWHITE]: PieceKind.WHITE,
	[PieceKind.SNEUTRON]: PieceKind.NEUTRON,
	[PieceKind.SCELL]: PieceKind.CELL,
	[PieceKind.BLACK]: PieceKind.BLACK,
	[PieceKind.WHITE]: PieceKind.WHITE,
	[PieceKind.NEUTRON]: PieceKind.NEUTRON,
	[PieceKind.CELL]: PieceKind.CELL
};

const mappingForHighlightingBoard: Record<PieceKind, PieceKind> = {
	[PieceKind.BLACK]: PieceKind.SBLACK,
	[PieceKind.WHITE]: PieceKind.SWHITE,
	[PieceKind.NEUTRON]: PieceKind.SNEUTRON,
	[PieceKind.CELL]: PieceKind.SCELL,
	[PieceKind.SBLACK]: PieceKind.SBLACK,
	[PieceKind.SWHITE]: PieceKind.SWHITE,
	[PieceKind.SNEUTRON]: PieceKind.SNEUTRON,
	[PieceKind.SCELL]: PieceKind.SCELL
};

const rowMoveMapping: Record<Direction, number> = {
	[Direction.NORTH]: -1,
	[Direction.SOUTH]: 1,
	[Direction.EAST]: 0,
	[Direction.WEST]: 0,
	[Direction.NORTHEAST]: -1,
	[Direction.NORTHWEST]: -1,
	[Direction.SOUTHEAST]: 1,
	[Direction.SOUTHWEST]: 1
};

const colMoveMapping: Record<Direction, number> = {
	[Direction.NORTH]: 0,
	[Direction.SOUTH]: 0,
	[Direction.EAST]: 1,
	[Direction.WEST]: -1,
	[Direction.NORTHEAST]: 1,
	[Direction.NORTHWEST]: -1,
	[Direction.SOUTHEAST]: 1,
	[Direction.SOUTHWEST]: -1
};

const directions: Direction[] = [
	Direction.NORTH,
	Direction.SOUTH,
	Direction.EAST,
	Direction.WEST,
	Direction.NORTHEAST,
	Direction.NORTHWEST,
	Direction.SOUTHEAST,
	Direction.SOUTHWEST
];

function getWhoMove(state: GameState): PieceKind {
	if (state.whoMove !== 0 && state.whoMove !== 1) {
		throw new Error(`Invalid whoMove value: ${state.whoMove}`);
	}

	return rotation[state.whoMove];
}

function updateWhoMove(state: GameState): void {
	state.whoMove = (state.whoMove + 1) % 2;
}

function updateBoard(moves: Move[], state: GameState): void {
	for (let c = 0; c < 5; c++) {
		for (let r = 0; r < 5; r++) {
			state.setElementAt(r, c, mappingForCleaningBoard[state.elementAt(r, c)]);
		}
	}

	moves.forEach((m: Move) =>
		state.setElementAt(m.row, m.col, mappingForHighlightingBoard[state.elementAt(m.row, m.col)])
	);
}

function inBounds(value: number, inc: number): boolean {
	return value + inc >= 0 && value + inc < 5;
}

function getRowMove(direction: Direction): number {
	return rowMoveMapping[direction];
}

function getColMove(direction: Direction): number {
	return colMoveMapping[direction];
}

function checkMove(move: Move, direction: Direction, state: GameState) {
	function _check(row: number, col: number, incR: number, incC: number, state: GameState): {
		row: number;
		col: number
	} {
		if (!inBounds(row, incR) ||
			!inBounds(col, incC) ||
			state.elementAt(row + incR, col + incC) !== PieceKind.CELL) return {row: row, col: col};
		return _check(row + incR, col + incC, incR, incC, state);
	}

	const {row, col} = _check(move.row, move.col, getRowMove(direction), getColMove(direction), state);

	return row === move.row && col === move.col ? undefined : new Move(row, col, move.kind);
}

function moves(startPoint: Move, state: GameState): Move[] {
	return directions
		.map((direction: Direction) => checkMove(startPoint, direction, state))
		.filter((m: Move | undefined) => m !== undefined);
}

function checkGameOver(
	neutronDestination: Move | undefined,
	pieceKind: PieceKind,
	state: GameState): { success: boolean; kind: PieceKind } {
	if (!neutronDestination) return {success: false, kind: PieceKind.CELL};

	const neutronMoves = moves(neutronDestination, state);
	if (!neutronMoves.length) return {success: true, kind: pieceKind};
	if (!neutronDestination.row) return {success: true, kind: PieceKind.BLACK};
	if (neutronDestination.row === 4) return {success: true, kind: PieceKind.WHITE};
	return {success: false, kind: PieceKind.CELL};
}

function applyMove(from: Move | undefined, to: Move | undefined, state: GameState): void {
	if (from && to) {
		state.setElementAt(to.row, to.col, to.kind);
		if (from.col * 5 + from.row != to.col * 5 + to.row)
			state.setElementAt(from.row, from.col, PieceKind.CELL);
	}
}

function applyFullMove(fullMove: FullMove, state: GameState, apply: boolean = true): void {
	applyMove(fullMove.moves[apply ? 0 : 3], fullMove.moves[apply ? 1 : 2], state);
	applyMove(fullMove.moves[apply ? 2 : 1], fullMove.moves[apply ? 3 : 0], state);
}

export async function onClickCell(state: GameState, row: number, col: number): Promise<{
	success: boolean;
	kind: PieceKind
}> {
	const pk = state.elementAt(row, col);
	let endGame: { success: boolean; kind: PieceKind } = {success: false, kind: PieceKind.CELL};

	if (pk === getWhoMove(state)) {
		updateBoard([], state);
		const move = new Move(row, col, getWhoMove(state));
		const validMoves = moves(move, state);
		updateBoard(validMoves.concat(move), state);
		state.selectedChip = move;
	} else if (pk === PieceKind.SCELL) {
		applyMove(state.selectedChip, new Move(row, col, state.selectedChip?.kind), state);
		updateBoard([], state);

		if (getWhoMove(state) === PieceKind.WHITE) {
			endGame = checkGameOver(state.neutronTo, PieceKind.WHITE, state);
			if (state.neutronFrom && state.neutronTo && state.selectedChip) {
				state.movements.push(new FullMove([state.neutronFrom, state.neutronTo, state.selectedChip, new Move(row, col, state.selectedChip.kind)], 0));
			}

			if (!endGame.success) {
				const obj = await nativeMinimax({board: Uint8Array.from(state.board), depth: state.difficulty});
				const machineFullMove = new FullMove(
					obj.moves.map((m: any) => new Move(m.row, m.col, m.kind)),
					obj.score
				);
				if (!machineFullMove.empty()) {
					state.movements.push(machineFullMove);
					state.neutronTo = machineFullMove.moves[1];
					applyFullMove(machineFullMove, state);
					updateBoard([], state);
					endGame = checkGameOver(machineFullMove.moves[1], PieceKind.BLACK, state);
				} else {
					endGame = {success: true, kind: PieceKind.WHITE};
				}
			}
		} else {
			state.neutronFrom = state.selectedChip;
			state.neutronTo = new Move(row, col, state.selectedChip?.kind);
			endGame = checkGameOver(state.neutronTo, PieceKind.WHITE, state);
		}

		updateWhoMove(state);
		state.selectedChip = undefined;
	} else {
		updateBoard([], state);
		state.selectedChip = undefined;
	}

	return endGame;
}

