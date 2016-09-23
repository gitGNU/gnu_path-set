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

#ifndef __CLOCKS_IMPL_H
#define __CLOCKS_IMPL_H

#include <sys/time.h>
#include <sys/resource.h>
#include <string.h>
#include <stddef.h>
#include <stdio.h>

#include "common.h"
#include "clocks.h"
#include "ptr-traits.h"

#define CLOCKS_MAX_USECS SZ(999999)
#define CLOCKS_LIM_USECS SZ(1000000)

#define ALWAYS_INLINE inline __attribute__((always_inline))

static ALWAYS_INLINE void
    clocks_time_init(
        struct clocks_time_t* time,
        const struct timeval* timeval)
{
    time->secs = INT_AS_SIZE(timeval->tv_sec),
    time->usecs = INT_AS_SIZE(timeval->tv_usec);

    if (time->usecs >= CLOCKS_LIM_USECS) {
        size_t q = time->usecs / CLOCKS_LIM_USECS;

        time->usecs %= CLOCKS_LIM_USECS;
        ASSERT_SIZE_ADD_NO_OVERFLOW(
            time->secs, q);
        time->secs += q;
    }
}

static ALWAYS_INLINE void
    clocks_time_assign(
        struct clocks_time_t* time,
        const struct clocks_time_t* time2)
{
    ASSERT(time2->usecs <= CLOCKS_MAX_USECS);

    time->secs = time2->secs;
    time->usecs = time2->usecs;
}

static ALWAYS_INLINE void
    clocks_time_add(
        struct clocks_time_t* time,
        const struct clocks_time_t* time2)
{
    ASSERT(time->usecs <= CLOCKS_MAX_USECS);
    ASSERT(time2->usecs <= CLOCKS_MAX_USECS);

    // (0): L := CLOCKS_LIM_USECS;
    // (1): usecs, usecs2 < L =>
    //      usecs + usecs2 < 2 * L
    // (2): usecs + usecs2 >= L =>
    //      (usecs + usecs2) % L =
    //      (usecs + usecs2) - L:
    //      by 0 < b <= a < 2 * b => a % b = a - b

    SIZE_ADD_EQ(time->secs, time2->secs);
    SIZE_ADD_EQ(time->usecs, time2->usecs);

    if (time->usecs >= CLOCKS_LIM_USECS) {
        time->usecs -= CLOCKS_LIM_USECS;
        ASSERT_SIZE_INC_NO_OVERFLOW(
            time->secs);
        time->secs ++;
    }
}

static ALWAYS_INLINE void
    clocks_time_sub(
        struct clocks_time_t* time,
        const struct clocks_time_t* time2)
{
    ASSERT(time->usecs <= CLOCKS_MAX_USECS);
    ASSERT(time2->usecs <= CLOCKS_MAX_USECS);

    SIZE_SUB_EQ(time->secs, time2->secs);

    if (time->usecs < time2->usecs) {
        // time->usecs + CLOCKS_MAX_USECS
        //   >= CLOCKS_MAX_USECS
        //   >= time2->usecs

        ASSERT_SIZE_ADD_NO_OVERFLOW(
            time->usecs, CLOCKS_MAX_USECS);
        time->usecs += CLOCKS_MAX_USECS;

        ASSERT_SIZE_DEC_NO_OVERFLOW(time->secs);
        time->secs --;
    }

    SIZE_SUB_EQ(time->usecs, time2->usecs);
}

struct utime_t
{
    struct clocks_time_t rtime;
    struct clocks_time_t utime;
    struct clocks_time_t stime;
#ifdef CLOCKS_NEED_CHILDREN_TIME
    struct clocks_time_t cutime;
    struct clocks_time_t cstime;
#endif
};

static ALWAYS_INLINE void
    utime_init(
        struct utime_t* utime)
{
    struct timeval t;
    struct rusage r;

    gettimeofday(&t, NULL);
    clocks_time_init(&utime->rtime, &t);

    getrusage(RUSAGE_SELF, &r);
    clocks_time_init(&utime->utime, &r.ru_utime);
    clocks_time_init(&utime->stime, &r.ru_stime);

#ifdef CLOCKS_NEED_CHILDREN_TIME
    getrusage(RUSAGE_CHILDREN, &r);
    clocks_time_init(&utime->cutime, &r.ru_utime);
    clocks_time_init(&utime->cstime, &r.ru_stime);
#endif
}

static ALWAYS_INLINE struct clocks_t
    utime_clocks(
        const struct utime_t* time)
{
#ifdef CLOCKS_NEED_CHILDREN_TIME
    struct clocks_time_t t;
#endif
    struct clocks_t r;
    struct utime_t u;

    utime_init(&u);

    clocks_time_assign(&r.real, &u.rtime);
    clocks_time_sub(&r.real, &time->rtime);

    clocks_time_assign(&r.user, &u.utime);
    clocks_time_sub(&r.user, &time->utime);

#ifdef CLOCKS_NEED_CHILDREN_TIME
    clocks_time_assign(&t, &u.cutime);
    clocks_time_sub(&t, &time->cutime);

    clocks_time_add(&r.user, &t);
#endif

    clocks_time_assign(&r.sys, &u.stime);
    clocks_time_sub(&r.sys, &time->stime);

#ifdef CLOCKS_NEED_CHILDREN_TIME
    clocks_time_assign(&t, &u.cstime);
    clocks_time_sub(&t, &time->cstime);

    clocks_time_add(&r.sys, &t);
#endif

    return r;
}

#endif // __CLOCKS_IMPL_H


