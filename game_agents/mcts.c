#include <assert.h>
#include <float.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "game.h"
#include "mcts.h"
#include "util.h"

struct node {
    int move;
    char player;
    int n_visits;
    fixed_point_t score;
    struct node *parent;
    struct node *children[N_GRIDS];
};

static struct node *new_node(int move, char player, struct node *parent)
{
    struct node *node = malloc(sizeof(struct node));
    node->move = move;
    node->player = player;
    node->n_visits = 0;
    node->score = 0;
    node->parent = parent;
    memset(node->children, 0, sizeof(node->children));
    return node;
}

static void free_node(struct node *node)
{
    for (int i = 0; i < N_GRIDS; i++)
        if (node->children[i])
            free_node(node->children[i]);
    free(node);
}

fixed_point_t fixed_sqrt(fixed_point_t x)
{
    if (!x)
        return x;

    x += (1U << (FIXED_SCALE_BITS - 1));
    fixed_point_t q, b, r;
    r = x >> FIXED_SCALE_BITS;
    b = 1U << 18;
    q = 0;
    while (b > 0) {
        fixed_point_t t = q + b;
        if (r >= t) {
            r -= t;
            q = t + b;
        }
        r <<= 1;
        b >>= 1;
    }
    if (r > q)
        q++;
    return q;
}

fixed_point_t fixed_log(fixed_point_t v)
{
    fixed_point_t y = ((v - (1U << FIXED_SCALE_BITS)) << (FIXED_SCALE_BITS)) /
                      (v + (1U << FIXED_SCALE_BITS));

    fixed_point_t ans = 0U;
    for (unsigned i = 1; i < 20; i += 2) {
        fixed_point_t z = (1U << FIXED_SCALE_BITS);
        for (int j = 0; j < i; j++) {
            z *= y;
            z >>= FIXED_SCALE_BITS;
        }
        z <<= FIXED_SCALE_BITS;
        z /= (i << FIXED_SCALE_BITS);

        ans += z;
    }
    ans <<= 1;
    return ans;
}

static inline fixed_point_t uct_score(int n_total,
                                      int n_visits,
                                      fixed_point_t score)
{
    if (n_visits == 0)
        return FIXED_MAX;
    fixed_point_t result =
        score << FIXED_SCALE_BITS /
                     (fixed_point_t) (n_visits << FIXED_SCALE_BITS);
    fixed_point_t tmp =
        EXPLORATION_FACTOR *
        fixed_sqrt(fixed_log(n_total << FIXED_SCALE_BITS / n_visits));
    tmp >>= FIXED_SCALE_BITS;
    return result + tmp;
}

static struct node *select_move(struct node *node)
{
    struct node *best_node = NULL;
    fixed_point_t best_score = 0U;
    for (int i = 0; i < N_GRIDS; i++) {
        if (!node->children[i])
            continue;
        fixed_point_t score =
            uct_score(node->n_visits, node->children[i]->n_visits,
                      node->children[i]->score);
        if (score > best_score) {
            best_score = score;
            best_node = node->children[i];
        }
    }
    return best_node;
}

static fixed_point_t simulate(char *table, char player)
{
    char current_player = player;
    char temp_table[N_GRIDS];
    memcpy(temp_table, table, N_GRIDS);
    while (1) {
        int *moves = available_moves(temp_table);
        if (moves[0] == -1) {
            free(moves);
            break;
        }
        int n_moves = 0;
        while (n_moves < N_GRIDS && moves[n_moves] != -1)
            ++n_moves;
        int move = moves[rand() % n_moves];
        free(moves);
        temp_table[move] = current_player;
        char win;
        if ((win = check_win(temp_table)) != ' ')
            return calculate_win_value(win, player);
        current_player ^= 'O' ^ 'X';
    }
    return (fixed_point_t) (1UL << (FIXED_SCALE_BITS - 1));
}

static void backpropagate(struct node *node, fixed_point_t score)
{
    while (node) {
        node->n_visits++;
        node->score += score;
        node = node->parent;
        score = 1 - score;
    }
}

static void expand(struct node *node, char *table)
{
    int *moves = available_moves(table);
    int n_moves = 0;
    while (n_moves < N_GRIDS && moves[n_moves] != -1)
        ++n_moves;
    for (int i = 0; i < n_moves; i++) {
        node->children[i] = new_node(moves[i], node->player ^ 'O' ^ 'X', node);
    }
    free(moves);
}

int mcts(char *table, char player)
{
    char win;
    struct node *root = new_node(-1, player, NULL);
    for (int i = 0; i < ITERATIONS; i++) {
        struct node *node = root;
        char temp_table[N_GRIDS];
        memcpy(temp_table, table, N_GRIDS);
        while (1) {
            if ((win = check_win(temp_table)) != ' ') {
                fixed_point_t score =
                    calculate_win_value(win, node->player ^ 'O' ^ 'X');
                backpropagate(node, score);
                break;
            }
            if (node->n_visits == 0) {
                fixed_point_t score = simulate(temp_table, node->player);
                backpropagate(node, score);
                break;
            }
            if (node->children[0] == NULL)
                expand(node, temp_table);
            node = select_move(node);
            assert(node);
            temp_table[node->move] = node->player ^ 'O' ^ 'X';
        }
    }
    struct node *best_node = root;
    int most_visits = -1;
    for (int i = 0; i < N_GRIDS; i++) {
        if (root->children[i] && root->children[i]->n_visits > most_visits) {
            most_visits = root->children[i]->n_visits;
            best_node = root->children[i];
        }
    }
    int best_move = best_node->move;
    free_node(root);
    return best_move;
}
