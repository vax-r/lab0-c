#pragma once

#include <setjmp.h>
#include "list.h"

/* Structure contains the current status of a coroutine */
struct coroutine {
    jmp_buf env;
    struct list_head list;
    char *table;
    char player;
};

struct arg {
    char *table;
    char player;
};

struct list_head *crlist;

void coroutine_add(struct coroutine *c)
{
    list_add_tail(&c->list, crlist);
}

void coroutine_switch()
{
    if (!list_empty(crlist)) {
        struct coroutine *c = list_first_entry(crlist, struct coroutine, list);
        list_del(&c->list);
        longjmp(c->env, 1); /* Switch to execute c */
    }
}