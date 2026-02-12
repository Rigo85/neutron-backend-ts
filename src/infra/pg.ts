/*
* ===============================================================================
* File Name          : pg.ts
* Creation Date      : 2026-02-11
* Version            : 1.0.0
* Author             : Rigoberto L. Salgado Reyes
* Contact            : rlsalgado2006@gmail.com
* ===============================================================================
*/
import { Pool } from "pg";
import { config } from "(src)/infra/config";
import { logger } from "(src)/infra/logger";

let pool: Pool | undefined;

export function getPool(): Pool {
	if (!pool) throw new Error("PG pool not initialized – call pgConnect() first");
	return pool;
}

export async function pgConnect(): Promise<void> {
	pool = new Pool({ connectionString: config.pgUrl });
	const client = await pool.connect();
	try {
		await client.query("SELECT 1");
		logger.info({ ns: "pg", ev: "connected" });
	} finally {
		client.release();
	}
}

export async function pgDisconnect(): Promise<void> {
	if (pool) {
		await pool.end();
		logger.info({ ns: "pg", ev: "disconnected" });
		pool = undefined;
	}
}
