/*
* ===============================================================================
* File Name          : event-log.ts
* Creation Date      : 2026-02-11
* Version            : 1.0.0
* Author             : Rigoberto L. Salgado Reyes
* Contact            : rlsalgado2006@gmail.com
* ===============================================================================
*/
import { getPool } from "(src)/infra/pg";
import { logger } from "(src)/infra/logger";

export interface SessionData {
	socketId: string;
	ip?: string;
	userAgent?: string;
	acceptLanguage?: string;
	platform?: string;
	language?: string;
	screenWidth?: number;
	screenHeight?: number;
	colorDepth?: number;
	timezone?: string;
}

export async function insertSession(data: SessionData): Promise<number> {
	const { rows } = await getPool().query(
		`INSERT INTO sessions
			(socket_id, ip, user_agent, accept_language, platform, language,
			 screen_width, screen_height, color_depth, timezone)
		 VALUES ($1,$2,$3,$4,$5,$6,$7,$8,$9,$10)
		 RETURNING id`,
		[
			data.socketId,
			data.ip ?? null,
			data.userAgent ?? null,
			data.acceptLanguage ?? null,
			data.platform ?? null,
			data.language ?? null,
			data.screenWidth ?? null,
			data.screenHeight ?? null,
			data.colorDepth ?? null,
			data.timezone ?? null
		]
	);
	return Number(rows[0].id);
}

export function closeSession(sessionId: number): void {
	getPool()
		.query("UPDATE sessions SET disconnected_at = NOW() WHERE id = $1", [sessionId])
		.catch((err) => logger.error({ ns: "pg", ev: "close_session_error", err: String(err?.message ?? err) }));
}

export function logEvent(
	sessionId: number | undefined,
	event: string,
	gameId?: string,
	payload?: Record<string, unknown>
): void {
	getPool()
		.query(
			`INSERT INTO game_events (session_id, game_id, event, payload)
			 VALUES ($1, $2, $3, $4)`,
			[sessionId ?? null, gameId ?? null, event, JSON.stringify(payload ?? {})]
		)
		.catch((err) => logger.error({ ns: "pg", ev: "log_event_error", err: String(err?.message ?? err) }));
}
