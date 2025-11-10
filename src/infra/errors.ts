/*
* ===============================================================================
* File Name          : errors.ts
* Creation Date      : 2025-11-01
* Last Modified      : 
* Version            : 1.0.0
* License            : 
* Author             : Rigoberto L. Salgado Reyes
* Contact            : rlsalgado2006@gmail.com
* ===============================================================================
*/

import { ZodError, ZodSchema } from "zod";
import { logger } from "(src)/infra/logger";

type Ack = (res: { ok: true; data: unknown } | { ok: false; error: string }) => void;

export function withAck<T>(
	schema: ZodSchema<T>,
	handler: (payload: T, ack?: Ack) => Promise<unknown> | unknown
) {
	return async (payload: unknown, maybeAck?: unknown) => {
		const ack = typeof maybeAck === "function" ? (maybeAck as Ack) : undefined;
		try {
			const parsed = schema.parse(payload);
			const data = await handler(parsed, ack);
			if (ack) ack({ok: true, data});
		} catch (err: any) {
			const msg =
				err instanceof ZodError
					? "invalid payload"
					: (err?.message ?? "error");

			logger.error({
				ns: "ws",
				ev: "handler_error",
				err: msg,
				issues: err instanceof ZodError ? err.issues : undefined
			});

			if (ack) ack({ok: false, error: msg});
		}
	};
}

export function safeHttp(handler: (req: any, res: any) => Promise<void> | void) {
	return async (req: any, res: any) => {
		try {
			await handler(req, res);
		} catch (err: any) {
			logger.error({ns: "http", ev: "handler_error", err: err?.message});
			res.status(500).json({ok: false, error: "internal_error"});
		}
	};
}
