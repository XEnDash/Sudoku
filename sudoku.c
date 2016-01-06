#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* TODO:

#: check for none numbers that got in somehow
#: check for corrupt file (check file length)
#: accept user given file name
#: prettier visuals (maybe with windows gui)
#: multiple solutions (and maybe find the best one? rate them based on heuristics?)
#: catch impossible solutions instead of crashing with stack overflow
#: optimise to use less recursion?
#: improve user input (allow cursor movement, changing prev numbers etc...)

 */

//#define SUDOKU_DEBUG

typedef int bool;
#define true 1
#define false 0

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

FILE *log;

int board[9][9] = 
{
    //1  2  3  4  5  6  7  8  9
    { 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // 1
    { 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // 2
    { 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // 3
    { 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // 4
    { 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // 5
    { 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // 6
    { 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // 7
    { 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // 8
    { 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // 9
};

bool IsOnlyNumberOnRow(int n, int x, int y)
{
    for(int x0 = 0; x0 < 9; x0++)
    {
	if(x0 == x)
	    continue;
	
	if(board[x0][y] == n)
	    return false;
    }

    return true;
}

bool IsOnlyNumberOnColumn(int n, int x, int y)
{
    for(int y0 = 0; y0 < 9; y0++)
    {
	if(y0 == y)
	    continue;
	
	if(board[x][y0] == n)
	    return false;
    }

    return true;
}

bool IsOnlyNumberOnSub(int n, int x, int y)
{
    int tx = x / 3;
    int ty = y / 3;

    for(int x0 = tx * 3; x0 < tx * 3 + 3; x0++)
    {
	for(int y0 = ty * 3; y0 < ty * 3 + 3; y0++)
	{
	    if(x0 == x && y0 == y)
		continue;
	    
	    if(board[x0][y0] == n)
		return false;
	}
    }

    return true;
}

void PrintBoard()
{
#ifndef SUDOKU_DEBUG
    printf("\n\n");

    printf("------------------------------------------");
    printf("\n");

    for(int y = 0; y < 9; y++)
    {
	for(int x = 0; x < 9; x++)
	{
	    if(board[x][y] == 0)
	    {
		//printf(ANSI_COLOR_RED);
		printf("[%i] ", board[x][y]);
		//printf(ANSI_COLOR_RESET);
	    }
	    else
	    {
		printf("[%i] ", board[x][y]);
	    }
	    
	    if((x + 1) % 3 == 0)
		printf("| ");
	}
	printf("\n");
	
	if((y + 1) % 3 == 0)
	{
	    printf("------------------------------------------");
	    //printf(" -     -   -   -     -   -   -     -   -  ");
	}

	printf("\n");
    }
    
    printf("\n\n");
#else
    if(!log)
	return;
    
    fprintf(log, "\n\n");

    fprintf(log, "------------------------------------------");
    fprintf(log, "\n");

    for(int y = 0; y < 9; y++)
    {
	for(int x = 0; x < 9; x++)
	{
	    if(board[x][y] == 0)
	    {
		//fprintf(log, ANSI_COLOR_RED);
		fprintf(log, "[%i] ", board[x][y]);
		//fprintf(log, ANSI_COLOR_RESET);
	    }
	    else
	    {
		fprintf(log, "[%i] ", board[x][y]);
	    }
	    
	    if((x + 1) % 3 == 0)
		fprintf(log, "| ");
	}
	fprintf(log, "\n");
	
	if((y + 1) % 3 == 0)
	{
	    fprintf(log, "------------------------------------------");
	    //fprintf(log, " -     -   -   -     -   -   -     -   -  ");
	}

	fprintf(log, "\n");
    }

    fprintf(log, "\n\n");
#endif
}

#define REJECTED_ALL { 0, 0, 0, 0, 0, 0, 0, 0, 0 }
#define REJECTED_ALL_ROW { REJECTED_ALL, REJECTED_ALL, REJECTED_ALL, REJECTED_ALL, REJECTED_ALL, REJECTED_ALL, REJECTED_ALL, REJECTED_ALL, REJECTED_ALL }

bool rejected_nums[9][9][9] = 
{
    REJECTED_ALL_ROW,
    REJECTED_ALL_ROW,
    REJECTED_ALL_ROW,
    REJECTED_ALL_ROW,
    REJECTED_ALL_ROW,
    REJECTED_ALL_ROW,
    REJECTED_ALL_ROW,
    REJECTED_ALL_ROW,
    REJECTED_ALL_ROW,
};

void ClearAllRejectedInSlot(int x, int y)
{
    for(int n = 0; n < 9; n++)
    {
	rejected_nums[x][y][n] = false;
    }
}

typedef struct
{
    //BacktrackStackFrame bt_stack[256];
    //int bt_size = 0;

    int x;
    int y;
    bool moving_forwards;
    bool brk;
} RecursiveBacktrackDS;

bool MoveToNextSlot(RecursiveBacktrackDS *data)
{
    data->moving_forwards = true;
    
    if(data->y > 8)
	return false;
    
    data->x += 1;
    if(data->x > 8)
    {
	data->x = 0;
	data->y += 1;

#ifdef SUDOKU_DEBUG
	PrintBoard();
#endif
	
	if(data->y > 8)
	{
	    return false;
	    data->brk = true;
	}
    }

    return true;
}

bool MoveToPrevSlot(RecursiveBacktrackDS *data)
{
    data->moving_forwards = false;
    
    if(data->y < 0)
	return false;

    data->x--;
    if(data->x < 0)
    {
	data->x = 8;
	data->y--;
	if(data->y < 0)
	{
	    return false;
	    data->brk = true;
	}
    }

    return true;
}

#define DO_NOT_CHANGE_ROW { 0, 0, 0, 0, 0, 0, 0, 0, 0 }
bool do_not_change[9][9] =
{
    DO_NOT_CHANGE_ROW,
    DO_NOT_CHANGE_ROW,
    DO_NOT_CHANGE_ROW,
    DO_NOT_CHANGE_ROW,
    DO_NOT_CHANGE_ROW,
    DO_NOT_CHANGE_ROW,
    DO_NOT_CHANGE_ROW,
    DO_NOT_CHANGE_ROW,
    DO_NOT_CHANGE_ROW
};

void RecursiveBacktrackFill(RecursiveBacktrackDS *data)
{
    if(data->brk)
	return;
    
    if(do_not_change[data->x][data->y])
    {
	if(data->moving_forwards)
	{
	    if(!MoveToNextSlot(data))
		return;
	}
	else
	{
	    if(!MoveToPrevSlot(data))
		return;
	}

	RecursiveBacktrackFill(data);
	return;
    }
    
    bool found_match = false;
    
    for(int n = 1; n <= 9; n++)
    {
	if(rejected_nums[data->x][data->y][n - 1])
	    continue;
	
	if(IsOnlyNumberOnRow(n, data->x, data->y) &&
	   IsOnlyNumberOnColumn(n, data->x, data->y) &&
	   IsOnlyNumberOnSub(n, data->x, data->y))
	{
	    board[data->x][data->y] = n;
	    found_match = true;
	    break;
	}
	else
	{
	    rejected_nums[data->x][data->y][n - 1] = true;
	}
    }

    if(!found_match)
    {
	ClearAllRejectedInSlot(data->x, data->y);

	if(!MoveToPrevSlot(data))
	    return;
	
	while(do_not_change[data->x][data->y])
	{
	    if(!MoveToPrevSlot(data))
		return;
	}

	int n = board[data->x][data->y];
	board[data->x][data->y] = 0;

	rejected_nums[data->x][data->y][n - 1] = true;
	
	RecursiveBacktrackFill(data);
    }

    if(!MoveToNextSlot(data))
	return;

    RecursiveBacktrackFill(data);
}

bool CheckBoard(int b[9][9])
{
    bool board_ok = true;
 
    for(int y = 0; y < 9; y++)
    {
	for(int x = 0; x < 9; x++)
	{
	    int n = b[x][y];

	    if(IsOnlyNumberOnRow(n, x, y) &&
	       IsOnlyNumberOnColumn(n, x, y) &&
	       IsOnlyNumberOnSub(n, x, y) &&
	       n != 0)
	    {
	    }
	    else
	    {
		board_ok = false;
		
		//printf(ANSI_COLOR_RED);
		printf("\n ERROR: [%i][%i] = %i", x, y, n);
		//printf(ANSI_COLOR_RESET);
	    }
	}
    }

    return board_ok;
}

void ReadBoardFromUser()
{
    for(int y = 0; y < 9; y++)
    {
	for(int x = 0; x < 9; x++)
	{
	    char c = getch();
	    printf("%c, ", c);
	    board[x][y] = c - 48;

	    if(c - 48 != 0)
	    {
		do_not_change[x][y] = true;
	    }
	}
	printf("\n");
    }
}

void ReadBoardFromFile(char *filename)
{
    FILE *f = fopen(filename, "r");

    if(!f)
	return;
    
    //fread(board, 1, 81, f);
    
    for(int y = 0; y < 9; y++)
    {
	for(int x = 0; x < 9; x++)
	{
	    char c = fgetc(f);
	    printf("%c, ", c);
	    board[x][y] = c - 48;

	    if(c - 48 != 0)
	    {
		do_not_change[x][y] = true;
	    }
	}
	printf("\n");
    }
}

void WriteBoardToFile(char *filename)
{
    FILE *f = fopen(filename, "w");

    if(!f)
	return;
    
    //fread(board, 1, 81, f);
    
    for(int y = 0; y < 9; y++)
    {
	for(int x = 0; x < 9; x++)
	{
	    char c = board[x][y] + 48;
	    fprintf(f, "%c", c);
	}
    }
}

int main()
{
    srand(time(0));

    log = fopen("log.txt", "w");

    while(1)
    {
	printf("\nDo you want to read the board from (F)ile or (U)ser? Q to quit\n");
	char answer = getch();
	if(answer == 'f' || answer == 'F')
	{
	    system("cls");
	    ReadBoardFromFile("board.txt");	    
	    break;
	}
	if(answer == 'u' || answer == 'U')
	{
	    system("cls");
	    ReadBoardFromUser();
	    WriteBoardToFile("board.txt");
	    break;
	}
	if(answer == 'q' || answer == 'Q')
	    return;
    }

    system("cls");

    PrintBoard();
    
    RecursiveBacktrackDS data = { 0, 0, true, false };
    RecursiveBacktrackFill(&data);

    PrintBoard();

    //ReadBoardFromUser();
    //WriteBoardToFile("board.txt");
    //ReadBoardFromFile("board.txt");
    if(CheckBoard(board))
    {
	printf("\n\n Board OK!");
    }
    else
    {
	printf("\n\n Board Not OK!");
    }

    printf("\n Do you wan't to quit? N or Y");
    
    char quit = getch();
    while(quit != 'y')
    {
	printf("\n Do you wan't to quit? N or Y");
	quit = getch();
    }
}
