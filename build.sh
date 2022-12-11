#!/bin/bash
tcc -Wall -Wextra -Werror -std=c99 main.c -o build/spg.linux
tcc -Wall -Wextra -Werror -std=c99 test.c -o build/test.linux
