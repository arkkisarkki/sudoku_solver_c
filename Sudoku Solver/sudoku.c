#include "sudoku.h"
#include <stdio.h>

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

void print_state(Value* state)
{
	printf("[");
	for (Index i = 0; i < 9 * 9; i++)
	{
		printf("%d,", state[i]);
	}
	printf("]\n");
}

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

void set_last_secure_state(Sudoku* sudoku)
{
	for (Index i = 0; i < 9 * 9; i++)
	{
		sudoku->last_secure_state.squares[i] = sudoku->squares[i];
	}
	sudoku->last_secure_state_set = true;
}

void get_row(Sudoku* sudoku, Value* buffer, Coordinate row)
{
	for (Coordinate column = 0; column < 9; column++)
	{
		buffer[column] = sudoku->squares_2d[row][column];
	}
}

void get_column(Sudoku* sudoku, Value* buffer, Coordinate column)
{
	for (Coordinate row = 0; row < 9; row++)
	{
		buffer[row] = sudoku->squares_2d[row][column];
	}
}

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