/*
* ===============================================================================
* File Name          : types.ts
* Creation Date      : 2025-11-01
* Last Modified      : 
* Version            : 1.0.0
* License            : 
* Author             : Rigoberto L. Salgado Reyes
* Contact            : rlsalgado2006@gmail.com
* ===============================================================================
*/

/**
 * Piece kind.
 */
export enum PieceKind {
	BLACK = 1,
	WHITE = 2,
	NEUTRON = 3,
	CELL = 4,
	SBLACK = 5,
	SWHITE = 6,
	SCELL = 7,
	SNEUTRON = 8
}

/**
 * Direction.
 */
export enum Direction {
	NORTH = 1,
	SOUTH = 2,
	EAST = 3,
	WEST = 4,
	NORTHEAST = 5,
	NORTHWEST = 6,
	SOUTHEAST = 7,
	SOUTHWEST = 8
}
