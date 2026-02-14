/*
* ===============================================================================
* File Name          : store.ts
* Creation Date      : 2025-11-01
* Last Modified      : 
* Version            : 1.0.0
* License            : 
* Author             : Rigoberto L. Salgado Reyes
* Contact            : rlsalgado2006@gmail.com
* ===============================================================================
*/
import { createClient, type RedisClientType } from "redis";
import { config } from "(src)/infra/config";
import { GameState } from "(src)/domain/GameState";
import { getReviver } from "(src)/domain/utils";

export class GameStore {
	private client: RedisClientType;
	private readonly prefix = "neutron:game:";

	constructor() {
		this.client = createClient({url: config.redisUrl});
	}

	async connect() {
		if (!this.client.isOpen)
			await this.client.connect();
	}

	isConnected(): boolean {
		return this.client.isOpen;
	}

	async ping(): Promise<void> {
		await this.client.ping();
	}

	async disconnect() {
		if (this.client.isOpen)
			await this.client.quit();
	}

	private key(id: string) {
		return this.prefix + id;
	}

	async load(gameId: string): Promise<GameState | undefined> {
		const raw = await this.client.get(this.key(gameId));
		return raw ?
			JSON.parse(raw, getReviver("GameState")) as GameState :
			undefined
			;
	}

	async save(next: GameState) {
		const key = this.key(next.id);
		await this.client.watch(key);
		const currentRaw = await this.client.get(key);
		const current: GameState | undefined = currentRaw ? JSON.parse(currentRaw, getReviver("GameState")) : undefined;

		if (!current) {
			if (next.version !== 0) {
				await this.client.unwatch();
				throw new Error("First save must start at version 0");
			}

			const res = await this.client
				.multi()
				.set(key, JSON.stringify(next), {EX: 3600 * 2})
				.exec();

			if (res === undefined)
				throw new Error("Concurrent update detected");

			return;
		}

		if (current?.version !== next.version - 1) {
			await this.client.unwatch();
			throw new Error(`version_conflict: key=${key}, expected=${current ? current.version + 1 : "n/a"}, got=${next.version}`);
		}

		const multi = this.client.multi();
		multi.set(key, JSON.stringify(next), {EX: 3600 * 2});
		const res = await multi.exec();

		if (res === undefined)
			throw new Error(`concurrent_update: key=${key}, attempted_version=${next.version}, cause=watched_key_modified`);
	}

	async removeOldGame(gameId: string): Promise<void> {
		try {
			const key = this.key(gameId);
			await this.client.del(key);
		} catch (error) {

		}
	}

	async initIfMissing(state: GameState): Promise<GameState> {
		const existing = await this.load(state.id);
		if (existing) return existing;
		state.updateVersion();
		await this.save(state);

		return state;
	}
}
