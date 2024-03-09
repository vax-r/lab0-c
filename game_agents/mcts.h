#pragma once

#define ITERATIONS 100000
#define EXPLORATION_FACTOR fixed_sqrt(2048)

int mcts(char *table, char player);