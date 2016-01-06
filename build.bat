REM gcc sudoku.c -o sudoku.exe -std=c99 -g
REM sudoku.exe

call vcvarsall.bat x64

REM 16mb stack
cl.exe sudoku.c /F 16777216 /Zi /link /subsystem:console
