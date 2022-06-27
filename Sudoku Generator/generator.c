typedef unsigned int Value;

typedef struct Sudoku_
{
	union
	{
		Value squares[9 * 9];
		Value squares_2d[9][9];
	};
} Sudoku;

int main()
{
	return 0;
}