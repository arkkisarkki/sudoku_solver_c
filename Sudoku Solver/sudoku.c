#include "sudoku.h"
#include <stdio.h>

/**
 * Print the game state to stdout.
 * 
 * \param sudoku The sudoku to print.
 */
void print_game(Sudoku* sudoku)
{
	for (Coordinate row = 0; row < 9; row++)
	{
		if (row != 0 && row % 3 == 0)
		{
			printf("----------------------\n");
		}
		for (Coordinate column = 0; column < 9; column++)
		{
			if (column != 0 && column % 3 == 0)
			{
				printf("| ");
			}
			Value current = sudoku->squares_2d[row][column];
			if (current)
			{
				printf("%d ", sudoku->squares_2d[row][column]);
			}
			else
			{
				printf("  ");
			}
		}
		printf("\n");
	}
	printf("\n");
}

/**
 * Initialize a Sudoku to a given state.
 * 
 * \param sudoku The sudoku to initialize.
 * \param state The state to set.
 */
void set_state(Sudoku* sudoku, const Value* state)
{
	for (Index i = 0; i < 9 * 9; i++)
	{
		sudoku->squares[i] = state[i];
		if (state[i])
		{
			sudoku->set_count++;
		}
	}
}

/**
 * Take a snapshot of the current state.
 * 
 * \param sudoku
 */
void set_last_secure_state(Sudoku* sudoku)
{
	for (Index i = 0; i < 9 * 9; i++)
	{
		sudoku->last_secure_state.squares[i] = sudoku->squares[i];
	}
	sudoku->last_secure_state_set = true;
}

/**
 * Get the full row containing the given coordinates.
 * 
 * \param sudoku
 * \param buffer The buffer to store the values into.
 * \param row The index of the row to get.
 */
void get_row(Sudoku* sudoku, Value* buffer, Coordinate row)
{
	for (Coordinate column = 0; column < 9; column++)
	{
		buffer[column] = sudoku->squares_2d[row][column];
	}
}

/**
 * Get the full column containing the given coordinates.
 * 
 * \param sudoku
 * \param buffer The buffer to store the values into.
 * \param column The index of the column to get.
 */
void get_column(Sudoku* sudoku, Value* buffer, Coordinate column)
{
	for (Coordinate row = 0; row < 9; row++)
	{
		buffer[row] = sudoku->squares_2d[row][column];
	}
}

/**
 * Get the full block containing the given coordinates.
 * 
 * \param sudoku
 * \param buffer The buffer to store the values into.
 * \param block_row The row index of the *block* (0-2).
 * \param block_column The column index of the *block* (0-2).
 */
void get_block(Sudoku* sudoku,
			   Block* buffer,
			   Coordinate block_row,
			   Coordinate block_column)
{
	for (Coordinate row = 0; row < 3; row++)
	{
		for (Coordinate column = 0; column < 3; column++)
		{
			buffer->squares_2d[row][column] =
				sudoku->squares_2d[(block_row * 3 + row)][column + block_column * 3];
		}
	}
}

/**
 * Get a block from the last secure snapshot.
 * 
 * \param sudoku
 * \param buffer The buffer to store the values into.
 * \param block_row The row index of the *block* (1-2).
 * \param block_column The column index of the *block* (1-2).
 */
void get_last_secure_block(Sudoku* sudoku,
						   Block* buffer,
						   Coordinate block_row,
						   Coordinate block_column)
{
	for (Coordinate row = 0; row < 3; row++)
	{
		for (Coordinate column = 0; column < 3; column++)
		{
			buffer->squares_2d[row][column] =
				sudoku->last_secure_state
				.squares_2d[(block_row * 3 + row)][column + block_column * 3];
		}
	}
}