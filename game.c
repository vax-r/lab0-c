#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "game.h"

void draw_board(const char *t)
{
    for (int i = 0; i < BOARD_SIZE; i++) {
        if (BOARD_SIZE < 10)
            printf("%2d | ", i + 1);
        else if (BOARD_SIZE >= 10 && BOARD_SIZE < 100)
            printf("%3d | ", i + 1);
        else
            printf("%4d | ", i + 1);

        for (int j = 0; j < BOARD_SIZE; j++) {
            // make background color alter between high-intensity and standard
            if ((i + j) & 1U)
                printf("\x1b[47m");
            else
                printf("\x1b[107m");

            switch (t[GET_INDEX(i, j)]) {
            case 'O':
                printf("\x1b[31m");
                printf(" ○ ");
                printf("\x1b[39m");
                break;
            case 'X':
                printf("\x1b[34m");
                printf(" × ");
                printf("\x1b[39m");
                break;
            default:
                printf("   ");
                break;
            }
            printf("\x1b[49m");
        }
        printf("\n");
    }
    if (BOARD_SIZE >= 10)
        printf("-");
    if (BOARD_SIZE >= 100)
        printf("-");
    printf("---+-");
    for (int i = 0; i < BOARD_SIZE; i++)
        printf("---");
    printf("\n");
    if (BOARD_SIZE >= 10)
        printf(" ");
    if (BOARD_SIZE >= 100)
        printf(" ");
    printf("    ");
    for (int i = 0; i < BOARD_SIZE; i++)
        printf(" %2c", 'A' + i);
    printf("\n");
}