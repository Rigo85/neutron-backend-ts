/*
* ===============================================================================
* File Name          : logger.ts
* Creation Date      : 2025-11-01
* Last Modified      : 2025-11-01
* Version            : 1.0.0
* License            :
* Author             : Rigoberto L. Salgado Reyes
* Contact            : rlsalgado2006@gmail.com
* ===============================================================================
*/

import pino from "pino";
import { config } from "(src)/infra/config";

export const logger = pino({
	level: config.logLevel,
	base: undefined, // avoid pid/hostname
	messageKey: "msg",
	formatters: {level(label) { return {level: label}; }},
	timestamp: pino.stdTimeFunctions.isoTime,
	transport: config.isDev && config.logPretty ? {
		target: "pino-pretty",
		options: {colorize: true, translateTime: "SYS:standard"}
	} : undefined
});


