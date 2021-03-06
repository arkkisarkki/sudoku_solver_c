#ifndef SUDOKU_H
#define SUDOKU_H

#include <stdint.h>

#define ALL_POSSIBLE	0x1FFU
#define ERROR			0xFF
#define false			0
#define true			1

typedef unsigned int Count;
typedef unsigned int Value;
typedef unsigned int Coordinate;
typedef unsigned int Index;

typedef uint32_t bool;

/**
 * Data type for a sudoku. Also contains a snapshot of the last secure state (before any quesses have been made) to revert back to if a wrong guess is made.
 */
typedef struct Sudoku_
{
	union
	{
		Value squares[9 * 9];
		Value squares_2d[9][9];
	};
	union
	{
		Value squares[9 * 9];
		Value squares_2d[9][9];
	} last_secure_state;

	Count set_count;
	bool last_secure_state_set;

} Sudoku;

/**
 * Data type for the nine special 3*3 blocks in a sudoku.
 */
typedef struct Block_
{
	union
	{
		Value squares[3 * 3];
		Value squares_2d[3][3];
	};
} Block;

/**
 * Simple data type for x and y coordinates.
 */
typedef struct Coordinates_
{
	Coordinate row;
	Coordinate column;

} Coordinates;


void print_game(Sudoku* sudoku);

void set_state(Sudoku* sudoku, const Value* state);

void set_last_secure_state(Sudoku* sudoku);

void get_row(Sudoku* sudoku, Value* buffer, Coordinate row);

void get_column(Sudoku* sudoku, Value* buffer, Coordinate column);

void get_block(Sudoku* sudoku,
			   Block* buffer,
			   Coordinate block_row,
			   Coordinate block_column);

void get_last_secure_block(Sudoku* sudoku,
						   Block* buffer,
						   Coordinate block_row,
						   Coordinate block_column);


#endif
