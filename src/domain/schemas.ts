/*
* ===============================================================================
* File Name          : schemas.ts
* Creation Date      : 2025-11-01
* Last Modified      : 2025-11-02
* Version            : 1.0.0
* License            : 
* Author             : Rigoberto L. Salgado Reyes
* Contact            : rlsalgado2006@gmail.com
* ===============================================================================
*/
import { z } from "zod";

export const GameIdSchema = z.object({gameId: z.string().min(1)});
const AllowedDifficultySchema = z.union([z.literal(2), z.literal(4), z.literal(11), z.literal(12), z.literal(13)]);
export const GameNewSchema = z.object({
	gameId: z.string().min(1).optional(),
	difficulty: AllowedDifficultySchema.optional()
});

export const GameChangeDifficultySchema = z.object({
	difficulty: AllowedDifficultySchema,
	gameId: z.string().min(1)
});

export const PieceKindSchema = z.number().int().min(1).max(8);

export const MoveSchema = z.object({
	row: z.number().int().min(0).max(4),
	col: z.number().int().min(0).max(4),
	kind: z.string()
});

const RowSchema = z.tuple([
	PieceKindSchema, PieceKindSchema, PieceKindSchema, PieceKindSchema, PieceKindSchema
]);

const MatrixBoardSchema = z.tuple([
	RowSchema, RowSchema, RowSchema, RowSchema, RowSchema
]);

const FlatBoardSchema = z.array(PieceKindSchema).length(25);

export const BoardSchema = z
	.union([FlatBoardSchema, MatrixBoardSchema])
	.transform((board) => (Array.isArray(board[0]) ? board.flat() : board));

export const FullMoveSchema = z.object({
	moves: z.array(MoveSchema).length(4),
	score: z.number()
});

export const GameStateSchema = z.object({
	__typename: z.literal("GameState"),
	id: z.string().min(1),
	board: BoardSchema,
	movements: z.array(FullMoveSchema),
	whoMove: z.number().int(),
	version: z.number().int().min(0),
	selectedChip: MoveSchema.optional(),
	neutronFrom: MoveSchema.optional(),
	neutronTo: MoveSchema.optional()
});

export const GameLoadSchema = GameStateSchema;

export const CellClickSchema = z.object({
	gameId: z.string().min(1),
	row: z.number().int().min(0).max(4),
	col: z.number().int().min(0).max(4)
});
