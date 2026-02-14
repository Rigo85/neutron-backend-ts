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
import path from "path";
import {Server} from "socket.io";
import {v4 as uuidv4} from "uuid";
import type {NextFunction, Request, Response} from "express";
import {safeHttp, withAck} from "(src)/infra/errors";
import {config} from "(src)/infra/config";
import {logger} from "(src)/infra/logger";
import {GameStore} from "(src)/game/store";
import {
    CellClickSchema,
    GameChangeDifficultySchema,
    GameIdSchema,
    GameLoadSchema,
    GameNewSchema
} from "(src)/domain/schemas";
import {GameState} from "(src)/domain/GameState";
import {isRlAvailable, isRlMode, loadRlModel, onClickCell} from "(src)/game/engine";
import {pgConnect, pgDisconnect, pgIsConnected, pgPing} from "(src)/infra/pg";
import {insertSession, closeSession, logEvent} from "(src)/infra/event-log";

moduleAlias.addAliases({
    "@root": __dirname + "/..",
    "(src)": __dirname
});

const app = express();
app.disable("x-powered-by");
app.use(express.json({limit: "1mb"}));

app.use(express.static(path.join(__dirname, "public"), {
    maxAge: 31557600000,
    immutable: true,
    index: false
}));


// Health check endpoint
app.get("/health", safeHttp((_req, res) => {
    res.json({ok: true, env: config.env, node: process.version});
}));

app.get("/ready", safeHttp(async (_req, res) => {
    let redisOk = false;
    let postgresOk = false;

    try {
        redisOk = store.isConnected();
        if (redisOk) await store.ping();
    } catch {
        redisOk = false;
    }

    try {
        postgresOk = pgIsConnected();
        if (postgresOk) await pgPing();
    } catch {
        postgresOk = false;
    }

    const ok = redisOk && postgresOk;
    const status = ok ? 200 : 503;

    res.status(status).json({
        ok,
        env: config.env,
        checks: {
            redis: redisOk ? "ok" : "down",
            postgres: postgresOk ? "ok" : "down"
        }
    });
}));

// SPA fallback for Express 5: wildcard must be a named parameter.
app.get("/{*path}", safeHttp((req, res) => {
    // Avoid hijacking non-HTML requests (e.g. ws polling, assets, APIs).
    if (!req.accepts("html")) {
        res.status(404).json({ok: false, error: "not_found"});
        return;
    }
    return res.sendFile(path.join(__dirname, "public", "index.html"));
}));

app.use((_req, res) => {
    res.status(404).json({ok: false, error: "not_found"});
});

app.use((err: unknown, _req: Request, res: Response, _next: NextFunction) => {
    logger.error({ns: "http", ev: "unhandled_error", err: String((err as Error)?.message ?? err)});
    if (res.headersSent) return;
    res.status(500).json({ok: false, error: "internal_error"});
});

const server = http.createServer(app);
const io = new Server(server, {cors: {origin: config.corsOrigins}});

const store = new GameStore();
const ns = io.of("/game");

/**
 * WS Namespace handlers.
 */
ns.on("connection", async (socket) => {
    logger.info({ns: "ws", ev: "connected", sid: socket.id});

    // --- Extract device info & persist session ---
    const headers = socket.handshake.headers;
    const auth = socket.handshake.auth ?? {};
    const ip = (headers["x-forwarded-for"] as string)?.split(",")[0]?.trim()
        ?? socket.handshake.address;

    let sessionId: number | undefined;
    try {
        sessionId = await insertSession({
            socketId: socket.id,
            ip,
            userAgent: headers["user-agent"] as string | undefined,
            acceptLanguage: headers["accept-language"] as string | undefined,
            platform: auth.platform,
            language: auth.language,
            screenWidth: auth.screenWidth,
            screenHeight: auth.screenHeight,
            colorDepth: auth.colorDepth,
            timezone: auth.timezone
        });
        (socket.data as any).sessionId = sessionId;
    } catch (err: any) {
        logger.warn({ns: "pg", ev: "insert_session_error", err: String(err?.message ?? err)});
    }

    // --- Event handlers ---

    socket.on("join", withAck(GameIdSchema, async ({gameId}) => {
        logEvent(sessionId, "join", gameId);
        socket.join(gameId);
        let st = await store.load(gameId);
        if (!st) st = await store.initIfMissing(new GameState(gameId));
        socket.emit("state", st);
        return st;
    }));

    socket.on("game:new", withAck(GameNewSchema, async ({gameId, difficulty}) => {
        const gid = gameId ?? uuidv4();
        logEvent(sessionId, "game_new", gid);
        const created = await store.initIfMissing(new GameState(gid));
        if (typeof difficulty === "number") {
            if (isRlMode(difficulty) && !isRlAvailable()) {
                throw new Error("rl_unavailable: RL addon/model not available");
            }
            created.difficulty = difficulty;
            created.version += 1;
            await store.save(created);
        }

        socket.join(gid);
        socket.emit("state", created);
        socket.to(gid).emit("state", created);

        return created;
    }));

    socket.on("game:change:diff", withAck(GameChangeDifficultySchema, async ({difficulty, gameId}) => {
        logEvent(sessionId, "game_change_difficulty", gameId, {difficulty});

        if (isRlMode(difficulty) && !isRlAvailable()) {
            throw new Error("rl_unavailable: RL addon/model not available");
        }

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
        logEvent(sessionId, "cell_click", gameId, {row, col});

        const current = await store.load(gameId);
        if (!current)
            throw new Error(`game_not_found: game not found (id=${gameId})`);

        const {current: next, endGame} = await applyClickAndEvolve(current, {row, col});
        await store.save(next);

        ns.to(gameId).emit("state", next);
        if (endGame.success) {
            logger.info({ns: "game", ev: "game_over", gameId, winner: endGame.kind});
            logEvent(sessionId, "game_over", gameId, {
                winner: endGame.kind,
                difficulty: next.difficulty,
                moveCount: next.movements.length,
                board: Array.from(next.board)
            });
            ns.to(gameId).emit("game:over", {winner: endGame.kind});
        }

        return next;
    }));

    socket.on("disconnect", (reason) => {
        logger.warn({ns: "ws", ev: "disconnected", sid: socket.id, reason});
        logEvent(sessionId, "disconnected", undefined, {reason});
        if (sessionId) closeSession(sessionId);
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
    await pgConnect();
    await loadRlModel(config.rlModelPath);

    server.listen(
        config.port,
        config.host,
        () => {
            logger.info({ns: "http", ev: "listening", host: config.host, port: config.port, env: config.env});
        }
    );

    let shuttingDown = false;
    const shutdown = async (signal: "SIGINT" | "SIGTERM") => {
        if (shuttingDown) return;
        shuttingDown = true;

        logger.info({ns: "proc", ev: "shutdown", signal});
        const forceExitTimer = setTimeout(() => {
            logger.error({ns: "proc", ev: "shutdown_timeout_forced_exit"});
            process.exit(1);
        }, 10_000);
        forceExitTimer.unref();

        try {
            await new Promise<void>((resolve, reject) => {
                io.close((err?: Error) => (err ? reject(err) : resolve()));
            });
            await store.disconnect();
            await pgDisconnect();
            await new Promise<void>((resolve, reject) => {
                server.close((err?: Error) => (err ? reject(err) : resolve()));
            });
            clearTimeout(forceExitTimer);
            process.exit(0);
        } catch (err: any) {
            clearTimeout(forceExitTimer);
            logger.error({ns: "proc", ev: "shutdown_error", err: String(err?.message ?? err)});
            process.exit(1);
        }
    };

    process.once("SIGINT", () => {
        void shutdown("SIGINT");
    });
    process.once("SIGTERM", () => {
        void shutdown("SIGTERM");
    });
}

main().catch((err) => {
    logger.fatal({ns: "proc", ev: "startup_error", err: String(err?.message ?? err)});
    process.exit(1);
});
