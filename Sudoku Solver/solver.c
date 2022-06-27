#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <stdlib.h>

#define ALL_POSSIBLE 0x1FFU
#define ERROR 0xFF
#define false 0
#define true 1

typedef unsigned int Count;
typedef unsigned int Value;
typedef unsigned int Coordinate;
typedef unsigned int Index;
typedef uint16_t Possibilities;
typedef uint32_t bool;

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
    } 
    last_secure_state;

    Count set_count;
    bool last_secure_state_set;

} Sudoku;

typedef struct Block_
{
    union 
    {
        Value squares[3 * 3];
        Value squares_2d[3][3];
    };
} Block;

typedef struct Coordinates_
{
    Coordinate row;
    Coordinate column;

} Coordinates;

typedef struct PossibleResets_
{
    Possibilities row;
    Possibilities column;
    Possibilities block;

} PossibleResets;

static bool states_are_equal(Value* state1, Value* state2)
{
    for (Index i = 0; i < 9 * 9; i++)
    {
        if (state1[i] != state2[i])
        {
            return false;
        }
    }

    return true;
}

static void print_game(Sudoku* sudoku)
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

static void set_state(Sudoku* sudoku, const Value* state)
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

static void set_last_secure_state(Sudoku* sudoku)
{
    for (Index i = 0; i < 9 * 9; i++)
    {
        sudoku->last_secure_state.squares[i] = sudoku->squares[i];
    }
    sudoku->last_secure_state_set = true;
}

static void get_row(Sudoku* sudoku, Value* buffer, Coordinate row)
{
    for (Coordinate column = 0; column < 9; column++)
    {
        buffer[column] = sudoku->squares_2d[row][column];
    }
}

static void get_column(Sudoku* sudoku, Value* buffer, Coordinate column)
{
    for (Coordinate row = 0; row < 9; row++)
    {
        buffer[row] = sudoku->squares_2d[row][column];
    }
}

static void get_block(Sudoku* sudoku, Block* buffer, Coordinate block_row, Coordinate block_column)
{
    for (Coordinate row = 0; row < 3; row++)
    {
        for (Coordinate column = 0; column < 3; column++)
        {
            buffer->squares_2d[row][column] = sudoku->squares_2d[(block_row * 3 + row)][column + block_column * 3];
        }
    }
}

static void get_last_secure_block(Sudoku* sudoku, Block* buffer, Coordinate block_row, Coordinate block_column)
{
    for (Coordinate row = 0; row < 3; row++)
    {
        for (Coordinate column = 0; column < 3; column++)
        {
            buffer->squares_2d[row][column] = sudoku->last_secure_state.squares_2d[(block_row * 3 + row)][column + block_column * 3];
        }
    }
}

static Possibilities get_possibilities(Sudoku* sudoku, Coordinate row, Coordinate column)
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

static bool step(Sudoku* sudoku)
{
    bool changed = false;
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
                    while (possibilities == 0)
                    {
                        Coordinate row_in_block = row % 3;
                        Coordinate column_in_block = column % 3;
                        Coordinate block_row = row / 3;
                        Coordinate block_column = column / 3;

                        PossibleResets possible_resets = { 
                            ALL_POSSIBLE & ~(1 << column),
                            ALL_POSSIBLE & ~(1 << row),
                            ALL_POSSIBLE & ~(1 << (row_in_block * 3 + column_in_block))
                        };

                        // Remove all definitely set
                        for (Coordinate i = 0; i < 9; i++)
                        {
                            if (sudoku->last_secure_state.squares_2d[i][column] || !sudoku->squares_2d[i][column])
                            {
                                possible_resets.column &= ~(1 << i);
                            }
                            if (sudoku->last_secure_state.squares_2d[row][i] || !sudoku->squares_2d[row][i])
                            {
                                possible_resets.row &= ~(1 << i);
                            }
                        }

                        Block block;
                        get_block(sudoku, &block, block_row, block_column);
                        Block last_secure_block;
                        get_last_secure_block(sudoku, &last_secure_block, block_row, block_column);
                        for (Index i = 0; i < 9; i++)
                        {
                            if (last_secure_block.squares[i] || !block.squares[i])
                            {
                                possible_resets.block &= ~(1 << i);
                            }
                        }

                        unsigned int possible_resets_count = count_possibilities(possible_resets.row);
                        possible_resets_count += count_possibilities(possible_resets.column);
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

                if (count_possibilities(possibilities) <= count_possibilities(lowest_possibilities))
                {
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
        changed = true;
        sudoku->squares_2d[lowest_possible_coords.row][lowest_possible_coords.column] = 
            get_random_possible(lowest_possibilities);
        sudoku->set_count++;
    }
    else if (!sudoku->last_secure_state_set)
    {
        set_last_secure_state(sudoku);
    }

    return changed;
}

static void solve(Sudoku* sudoku)
{
    while (sudoku->set_count < 9*9)
    {
#if VERBOSE
        print_game(sudoku);
#endif
        step(sudoku);
    }
}

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
            buffer[i] =  0;
        }
        else
        {
            buffer[i] = sudoku.squares[i];
        }
    }
}

static void print_state(Value* state)
{
    printf("[");
    for (Index i = 0; i < 9 * 9; i++)
    {
        printf("%d,", state[i]);
    }
    printf("]\n");
}

int main()
{
    srand((unsigned int) time(NULL));

    while (true)
    {
        Value initial_state[9 * 9] = { 0,0,3,0,0,0,2,9,0,1,0,0,0,0,9,0,0,0,0,0,5,6,1,3,0,0,0,0,4,8,0,0,6,0,0,0,0,0,0,0,4,0,0,0,0,0,1,0,0,8,7,0,0,0,0,0,6,0,0,0,8,0,0,0,0,1,0,0,0,3,0,0,8,0,0,0,0,0,0,0,0 };
        /*
        generate_sudoku(initial_state, 70);
        print_state(initial_state);
        */
        Sudoku game = { 0 };
        set_state(&game, initial_state);
        print_game(&game);

        solve(&game);
        print_game(&game);
    }
    return 0;
}