#include "sudoku.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/**
 * Bitfield type for possible values for a square. For example, 6 = 0b110 would imply that the values 2 and 3 are possible.
 */
typedef uint16_t Possibilities;

/**
 * Data type encoding resetable squares around a particular square.
 */
typedef struct PossibleResets_
{
	Possibilities row;
	Possibilities column;
	Possibilities block;

} PossibleResets;

/**
 * Get possible values for given coordinates.
 *
 * \param sudoku Pointer to the sudoku currently being solved
 * \param row Row coordinate
 * \param column Column coordinate
 * \return Possible values for the square
 */
static Possibilities get_possibilities(Sudoku* sudoku,
									   Coordinate row,
									   Coordinate column)
{
	if (sudoku->squares_2d[row][column])
	{
		return 1 << sudoku->squares_2d[row][column];
	}

	Possibilities retval = ALL_POSSIBLE;

	Value row_values[9];
	Value column_values[9];
	Block block;

	get_row(sudoku, row_values, row);
	get_column(sudoku, column_values, column);
	get_block(sudoku, &block, row / 3, column / 3);

	for (Index i = 0; i < 9; i++)
	{
		if (row_values[i])
		{
			retval &= ~(1 << (row_values[i] - 1));
		}
		if (column_values[i])
		{
			retval &= ~(1 << (column_values[i] - 1));
		}
		if (block.squares[i])
		{
			retval &= ~(1 << (block.squares[i] - 1));
		}
	}

	return retval;
}

/**
 * Counts the high bits in a Possibilites bitfield.
 *
 * \param possibilities
 * \return The count of possible values.
 */
static Count count_possibilities(Possibilities possibilities)
{
	Count retval = 0;
	for (Index i = 0; i < 9; i++)
	{
		if (possibilities & (1 << i))
		{
			retval++;
		}
	}

	return retval;
}

/**
 * Returns the (assumed) only possible value from a Possibilities.
 *
 * \param possibilities
 * \return The only possible value.
 */
static Value get_only_possible(Possibilities possibilities)
{
	for (Index i = 0; i < 9; i++)
	{
		if (possibilities & (1 << i))
		{
			return i + 1;
		}
	}

	return ERROR;
}

/**
 * Returns a random possible value from a possibilities.
 *
 * \param possibilities
 * \return A random possible value.
 */
static Value get_random_possible(Possibilities possibilities)
{
	Count count = count_possibilities(possibilities);
	int choice = rand() % count;

	int j = 0;
	for (Index i = 0; i < 9; i++)
	{
		if (possibilities & (1 << i))
		{
			if (j++ == choice)
			{
				return i + 1;
			}
		}
	}

	return ERROR;
}

/**
 * Performs one iteration of the solver algorithm. First inserts all "safe" values and then proceeds to make guesses. 
 * If a guess leads to a square where no value is possible, adjacent squares are cleared randomly until some value becomes possible.
 *
 * \param sudoku The game to solve.
 */
static void step(Sudoku* sudoku)
{
	bool changed = false;
	bool certain = true;
	Possibilities lowest_possibilities = ALL_POSSIBLE;
	Coordinates lowest_possible_coords = { 0 };

	for (Coordinate row = 0; row < 9; row++)
	{
		for (Coordinate column = 0; column < 9; column++)
		{
			if (!sudoku->squares_2d[row][column])
			{
				Possibilities possibilities = get_possibilities(sudoku, row, column);

				if (possibilities == 0)
				{
					certain = false;
					while (possibilities == 0)
					{
						Coordinate row_in_block = row % 3;
						Coordinate column_in_block = column % 3;
						Coordinate block_row = row / 3;
						Coordinate block_column = column / 3;

						PossibleResets possible_resets = {
							ALL_POSSIBLE & ~(1 << column), ALL_POSSIBLE & ~(1 << row),
							ALL_POSSIBLE & ~(1 << (row_in_block * 3 + column_in_block)) };

						// Remove all definitely set
						for (Coordinate i = 0; i < 9; i++)
						{
							if (sudoku->last_secure_state.squares_2d[i][column] ||
								!sudoku->squares_2d[i][column])
							{
								possible_resets.column &= ~(1 << i);
							}
							if (sudoku->last_secure_state.squares_2d[row][i] ||
								!sudoku->squares_2d[row][i])
							{
								possible_resets.row &= ~(1 << i);
							}
						}

						Block block;
						get_block(sudoku, &block, block_row, block_column);
						Block last_secure_block;
						get_last_secure_block(sudoku, &last_secure_block, block_row,
											  block_column);
						for (Index i = 0; i < 9; i++)
						{
							if (last_secure_block.squares[i] || !block.squares[i])
							{
								possible_resets.block &= ~(1 << i);
							}
						}

						unsigned int possible_resets_count =
							count_possibilities(possible_resets.row);
						possible_resets_count +=
							count_possibilities(possible_resets.column);
						possible_resets_count += count_possibilities(possible_resets.block);

						int choice = rand() % possible_resets_count;
						Coordinates to_reset = { 0 };

						unsigned int j = 0;
						for (Index i = 0; i < 9; i++)
						{
							if (possible_resets.row & (1 << i))
							{
								if (j++ == choice)
								{
									to_reset.row = row;
									to_reset.column = i;
									break;
								}
							}
							if (possible_resets.column & (1 << i))
							{
								if (j++ == choice)
								{
									to_reset.row = i;
									to_reset.column = column;
									break;
								}
							}
							if (possible_resets.block & (1 << i))
							{
								if (j++ == choice)
								{
									to_reset.row = block_row * 3 + i / 3;
									to_reset.column = block_column * 3 + i % 3;
									break;
								}
							}
						}

						sudoku->squares_2d[to_reset.row][to_reset.column] = 0;
						sudoku->set_count--;

#if VERBOSE
						print_game(sudoku);
#endif
						possibilities = get_possibilities(sudoku, row, column);
					}

					changed = false;
				}

				if (count_possibilities(possibilities) <=
					count_possibilities(lowest_possibilities))
				{
					certain = false;
					lowest_possibilities = possibilities;
					lowest_possible_coords.row = row;
					lowest_possible_coords.column = column;
				}

				if (count_possibilities(possibilities) == 1)
				{
					changed = true;
					sudoku->squares_2d[row][column] = get_only_possible(possibilities);
					sudoku->set_count++;
				}
			}
		}
	}

	if (!changed)
	{
		sudoku->squares_2d[lowest_possible_coords.row]
			[lowest_possible_coords.column] =
			get_random_possible(lowest_possibilities);
		sudoku->set_count++;
	}
	else if (!sudoku->last_secure_state_set && certain)
	{
		set_last_secure_state(sudoku);
	}
}

/**
 * Solves a given sudoku.
 * 
 * \param sudoku The sudoku to solve.
 */
static void solve(Sudoku* sudoku)
{
	while (sudoku->set_count < 9 * 9)
	{
#if VERBOSE
		print_game(sudoku);
#endif
		step(sudoku);
	}
}

/**
 * Generates a new sudoku.
 * 
 * \param buffer The buffer to place the generated values into.
 * \param difficulty How many squares to leave empty.
 */
static void generate_sudoku(Value* buffer, unsigned int difficulty)
{
	// Produce solution from empty start
	Sudoku sudoku = { 0 };
	solve(&sudoku);

	// Remove random squares from solution to produce new puzzle
	for (Index i = 0; i < 9 * 9; i++)
	{
		if ((unsigned int) rand() % 100 < difficulty)
		{
			buffer[i] = 0;
		}
		else
		{
			buffer[i] = sudoku.squares[i];
		}
	}
}

int main()
{
	srand((unsigned int) time(NULL));

	while (true)
	{
		Value initial_state[9 * 9] = { 0 };
		int difficulty = rand() % 100;
		generate_sudoku(initial_state, difficulty);

		Sudoku game = { 0 };
		set_state(&game, initial_state);

		printf("Challenge (difficulty %d):\n", difficulty);
		print_game(&game);

		solve(&game);
		printf("Solution:\n");
		print_game(&game);
	}

	return 0;
}