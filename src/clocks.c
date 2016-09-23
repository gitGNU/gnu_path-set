// Copyright (C) 2016  Stefan Vargyas
// 
// This file is part of Path-Set.
// 
// Path-Set is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// Path-Set is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with Path-Set.  If not, see <http://www.gnu.org/licenses/>.

#include "config.h"

#include "common.h"
#include "clocks.h"
#include "clocks-impl.h"

static void clocks_time_print(
    const struct clocks_time_t* time,
    bool useconds, FILE* file)
{
    ASSERT(time->usecs <= CLOCKS_MAX_USECS);

    if (!useconds)
        fprintf(file, "%zu.%06zu",
            time->secs, time->usecs);
    else {
        size_t s = time->secs;

        VERIFY_SIZE_MUL_NO_OVERFLOW(
            s, CLOCKS_LIM_USECS);
        s *= CLOCKS_LIM_USECS;

        VERIFY_SIZE_ADD_NO_OVERFLOW(
            s, time->usecs);
        fprintf(file, "%zu",
            s + time->usecs);
    }
}

void clocks_init(
    struct clocks_t* clocks)
{
    memset(clocks, 0, sizeof(*clocks));
}

void clocks_add(
    struct clocks_t* clocks,
    const struct clocks_t* clocks2)
{
    clocks_time_add(&clocks->real, &clocks2->real);
    clocks_time_add(&clocks->user, &clocks2->user);
    clocks_time_add(&clocks->sys, &clocks2->sys);
}

void clocks_print(
    const struct clocks_t* clocks,
    const char* name, size_t width,
    bool useconds, FILE* file)
{
    struct time_info_t
    {
        const char* name;
        size_t offset;
    };
#undef  CASE
#define CASE(n)                 \
    {                           \
        .name = #n,             \
        .offset = offsetof(     \
            struct clocks_t, n) \
    }
#define CLOCKS_TIME(p)                 \
    (                                  \
        (const struct clocks_time_t*)  \
        (((char*) clocks) + p->offset) \
    )
    const struct time_info_t infos[] = {
        CASE(real),
        CASE(user),
        CASE(sys)
    };
    const struct time_info_t *p, *e;
    size_t l, w;

    l = strlen(name);
    for (p = infos,
         e = infos + ARRAY_SIZE(infos);
         p < e;
         p ++) {
        w = strlen(p->name);
        w = SIZE_ADD(w, l);
        w = SIZE_INC(w);
        w = w < width ? width - w : 0;
        fprintf(file, "%s-%s:%-*s ",
            p->name, name, SIZE_AS_INT(w), "");
        clocks_time_print(
            CLOCKS_TIME(p), useconds, file);
        fputc('\n', file);
    }
}


