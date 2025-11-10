/*
* ===============================================================================
* File Name          : server.ts
* Creation Date      : 2025-11-01
* Last Modified      : 
* Version            : 1.0.0
* License            : 
* Author             : Rigoberto L. Salgado Reyes
* Contact            : rlsalgado2006@gmail.com
* ===============================================================================
*/
import moduleAlias from "module-alias";
import express from "express";
import http from "node:http";
import { Server } from "socket.io";
import { v4 as uuidv4 } from "uuid";
import { safeHttp, withAck } from "(src)/infra/errors";
import { config } from "(src)/infra/config";
import { logger } from "(src)/infra/logger";
import { GameStore } from "(src)/game/store";
import {
	CellClickSchema,
	GameChangeDifficultySchema,
	GameIdSchema,
	GameLoadSchema,
	GameNewSchema
} from "(src)/domain/schemas";
import { GameState } from "(src)/domain/GameState";
import { onClickCell } from "(src)/game/engine";

moduleAlias.addAliases({
	"@root": __dirname + "/..",
	"(src)": __dirname
});

const app = express();
app.use(express.json());

// Health check endpoint
app.get("/health", safeHttp((_req, res) => {
	res.json({ok: true, env: config.env, node: process.version});
}));

const server = http.createServer(app);
const io = new Server(server, {cors: {origin: config.corsOrigins}});

const store = new GameStore();
const ns = io.of("/game");

/**
 * WS Namespace handlers.
 */
ns.on("connection", (socket) => {
	logger.info({ns: "ws", ev: "connected", sid: socket.id});

	socket.on("join", withAck(GameIdSchema, async ({gameId}) => {
		socket.join(gameId);
		let st = await store.load(gameId);
		if (!st) st = await store.initIfMissing(new GameState(gameId));
		socket.emit("state", st);
		return st;
	}));

	socket.on("game:new", withAck(GameNewSchema, async ({gameId}) => {
		const gid = gameId ?? uuidv4();
		const created = await store.initIfMissing(new GameState(gid));

		socket.join(gid);
		socket.emit("state", created);
		socket.to(gid).emit("state", created);

		return created;
	}));

	socket.on("game:change:diff", withAck(GameChangeDifficultySchema, async ({difficulty, gameId}) => {
		const current = await store.load(gameId);
		if (!current) {
			throw new Error(`game_not_found: game not found (id=${gameId})`);
		}

		current.difficulty = difficulty;
		current.version += 1;
		await store.save(current);
		socket.emit("state", current);

		return current;
	}));

	socket.on("game:load", withAck(GameLoadSchema, async (gameState) => {
		const id = gameState.id;
		if (!id) {
			throw new Error("invalid_game_state: missing id");
		}

		await store.removeOldGame(id);
		const gs = GameState.fromJSON(gameState);
		gs.version = 0;

		await store.save(gs);
		socket.join(gs.id);
		socket.emit("state", gs);
		return gameState;
	}));

	socket.on("cell:click", withAck(CellClickSchema, async ({gameId, row, col}) => {
		logger.info({ns: "ws", ev: "cell_click", gameId, row, col});

		const current = await store.load(gameId);
		if (!current)
			throw new Error(`game_not_found: game not found (id=${gameId})`);

		const {current: next, endGame} = await applyClickAndEvolve(current, {row, col});
		await store.save(next);

		ns.to(gameId).emit("state", next);
		if (endGame.success) {
			logger.info({ns: "game", ev: "game_over", gameId, winner: endGame.kind});
			ns.to(gameId).emit("game:over", {winner: endGame.kind});
		}

		return next;
	}));

	socket.on("disconnect", (reason) => {
		logger.warn({ns: "ws", ev: "disconnected", sid: socket.id, reason});
	});
});

async function applyClickAndEvolve(current: GameState, click: { row: number; col: number }) {
	const endGame = await onClickCell(current, click.row, click.col);
	logger.info({ns: "game", ev: "cell_clicked", row: click.row, col: click.col, endGame});
	current.version = (current.version ?? 0) + 1;

	return {current, endGame};
}

/**
 * Bootstrap the server.
 */
async function main() {
	await store.connect();

	server.listen(
		config.port,
		config.host,
		() => {
			logger.info({ns: "http", ev: "listening", host: config.host, port: config.port, env: config.env});
		}
	);

	const shutdown = async () => {
		logger.info({ns: "proc", ev: "shutdown"});

		try {
			await store.disconnect();
		} catch (err: any) {
			logger.error({ns: "proc", ev: "shutdown_error", err: String(err?.message ?? err)});
		}

		server.close(() => process.exit(0));
	};

	process.on("SIGINT", shutdown);
	process.on("SIGTERM", shutdown);
}

main().catch((err) => {
	logger.fatal({ns: "proc", ev: "startup_error", err: String(err?.message ?? err)});
	process.exit(1);
});
