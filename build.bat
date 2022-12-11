@echo off
tcc -Wall -Wextra -Werror -std=c99 main.c -o build/spg.exe
tcc -Wall -Wextra -Werror -std=c99 test.c -o build/test.exe
