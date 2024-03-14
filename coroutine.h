#pragma once

#include <setjmp.h>
#include "list.h"

/* Structure contains the current status of a coroutine */
struct coroutine {
    jmp_buf env;
    struct list_head list;
    char task_name[20];
    char *table;
    char player;
    int rt_value;
};

struct arg {
    char *table;
    char player;
    char task_name[20];
};

struct coroutine *cur_coroutine;
struct list_head crlist;

void coroutine_add(struct coroutine *c)
{
    list_add_tail(&c->list, &crlist);
}

void coroutine_switch()
{
    if (!list_empty(&crlist)) {
        struct coroutine *c = list_first_entry(&crlist, struct coroutine, list);
        list_del(&c->list);
        cur_coroutine = c;
        longjmp(c->env, 1); /* Switch to execute c */
    }
}