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

#ifndef SET_NAME
#error  SET_NAME is not defined
#endif

#ifndef SET_IMPL_NAME
#error  SET_IMPL_NAME is not defined
#endif

#define SET_MAKE_NAME__(n, s)   n ## _set_ ## s
#define SET_MAKE_NAME_(n, s)    SET_MAKE_NAME__(n, s)
#define SET_MAKE_NAME(s)        SET_MAKE_NAME_(SET_NAME, s)

#define SET_MAKE_IMPL_NAME__(n, s) n ## _ ## s
#define SET_MAKE_IMPL_NAME_(n, s) SET_MAKE_IMPL_NAME__(n, s)
#define SET_MAKE_IMPL_NAME(s)   SET_MAKE_IMPL_NAME_(SET_IMPL_NAME, s)

#define SET_TYPE                SET_MAKE_NAME(t)
#define SET_CREATE              SET_MAKE_NAME(create)
#define SET_DESTROY             SET_MAKE_NAME(destroy)
#define SET_LOAD                SET_MAKE_NAME(load)
#define SET_IS_EMPTY            SET_MAKE_NAME(is_empty)
#define SET_GET                 SET_MAKE_NAME(get)
#define SET_PUT                 SET_MAKE_NAME(put)
#define SET_PRINT_SET           SET_MAKE_NAME(print_set)
#define SET_GET_NODE_SIZE       SET_MAKE_NAME(get_node_size)
#define SET_GET_ELEM_SIZE       SET_MAKE_NAME(get_elem_size)

#define SET_IMPL_TYPE           SET_MAKE_IMPL_NAME(t)
#define SET_IMPL_INIT           SET_MAKE_IMPL_NAME(init)
#define SET_IMPL_DONE           SET_MAKE_IMPL_NAME(done)

#define SET_IMPL_NODE_TYPE      SET_MAKE_IMPL_NAME(node_t)
#define SET_IMPL_LOOKUP         SET_MAKE_IMPL_NAME(lookup)
#define SET_IMPL_INSERT         SET_MAKE_IMPL_NAME(insert)
#define SET_IMPL_IS_EMPTY       SET_MAKE_IMPL_NAME(is_empty)
#define SET_IMPL_PRINT          SET_MAKE_IMPL_NAME(print)

#define SET_STAT_PARAM_SIZE_FUNC_TYPE \
                                SET_MAKE_NAME(stat_param_size_func_t)
#define SET_STAT_PARAM_FRAC_FUNC_TYPE \
                                SET_MAKE_NAME(stat_param_size_frac_t)
#define SET_STAT_PARAMS_PRINT   SET_MAKE_NAME(stat_params_print)

#include "clocks-impl.h"

struct SET_TYPE
{
#ifdef SET_NEED_POOL_ALLOC
    struct pool_alloc_t  pool;
#endif
    struct SET_IMPL_TYPE impl;
    struct set_stats_t   stats;
};

#define SET_ALLOCATE_STRUCT(n, t)        \
    ({                                   \
        void* __r = pool_alloc_allocate( \
            &set->pool,                  \
            sizeof(struct t),            \
            MEM_ALIGNOF(struct t));      \
        ENSURE(__r != NULL,              \
            "pool alloc failed");        \
        set->stats.n ## _struct ++;      \
        __r;                             \
    })
#define SET_ALLOCATE_VAR_STRUCT(n, t, l) \
    ({                                   \
        size_t __n = sizeof(struct t);   \
        void* __r = pool_alloc_allocate( \
            &set->pool,                  \
            SIZE_ADD_EQ(__n, l),         \
            MEM_ALIGNOF(struct t));      \
        ENSURE(__r != NULL,              \
            "pool alloc failed");        \
        set->stats.n ## _struct ++;      \
        set->stats.n ## _mem += l;       \
        __r;                             \
    })
#define SET_ALLOCATE_ARRAY(n, t, l)          \
    ({                                       \
        size_t __n = SIZE_MUL(l, sizeof(t)); \
        void* __r = pool_alloc_allocate(     \
            &set->pool, __n,                 \
            MEM_ALIGNOF(t[0]));              \
        ENSURE(__r != NULL,                  \
            "pool alloc failed");            \
        set->stats.n ## _mem += __n;         \
        __r;                                 \
    })

struct SET_TYPE* SET_CREATE(
    const struct options_t* opt)
{
    const size_t n = sizeof(struct SET_TYPE);
    struct SET_TYPE* r;

    r = malloc(n);
    ENSURE(r != NULL, "malloc failed");
    memset(r, 0, n);

#ifdef SET_NEED_POOL_ALLOC
    pool_alloc_init(&r->pool, opt->pool_size);
#endif
    SET_IMPL_INIT(&r->impl, r, opt);

    return r;
}

void SET_DESTROY(
    struct SET_TYPE* set)
{
    SET_IMPL_DONE(&set->impl);
#ifdef SET_NEED_POOL_ALLOC
    pool_alloc_done(&set->pool);
#endif
}

#if 0
static size_t SET_GET(
    struct SET_TYPE* set,
    const char* key, size_t len)
{
    const struct SET_IMPL_NODE_TYPE* n = NULL;
    struct clocks_t c;
    struct utime_t t;
    bool b;

    utime_init(&t);
    b = SET_IMPL_LOOKUP(&set->impl, key, len, &n);
    c = utime_clocks(&t);

    clocks_add(&set->stats.lookup_time, &c);

    if (b) {
        ASSERT(n != NULL);
        return n->val;
    }
    else {
        ASSERT(n == NULL);
        return 0;
    }
}
#endif

static size_t SET_PUT(
    struct SET_TYPE* set,
    const char* key, size_t len,
    size_t line)
{
    struct SET_IMPL_NODE_TYPE* n = NULL;
    struct clocks_t c;
    struct utime_t t;
    bool b;

    ASSERT(line > 0);

    utime_init(&t);
    b = SET_IMPL_INSERT(&set->impl, key, len, &n);
    c = utime_clocks(&t);
    ASSERT(n != NULL);

    clocks_add(&set->stats.insert_time, &c);

    if (!b)
        return n->val;
    else {
        n->val = line;
        return 0;
    }
}

void SET_LOAD(
    struct SET_TYPE* set,
    FILE* file, bool verbose)
{
    char* b = NULL;
    size_t n = 0, k = 1;
    ssize_t r;

    while ((r = getline(&b, &n, file)) >= 0) {
        size_t l = INT_AS_SIZE(r);
        size_t r;
        char* p;

        ASSERT(b != NULL);
        ASSERT(n > 0);

        ASSERT(l > 0);
        ASSERT(l < n);

        if ((p = memchr(b, 0, l)) != NULL) {
            size_t d;

            ASSERT(p >= b);
            ASSERT(p < b + l);
            d = PTR_DIFF(p, b);

            if (verbose)
                warning("NUL char in line #%zu: truncating "
                    "it from length %zu to %zu", k, l, d);

            l = d; 
        }

        if (b[l - 1] == '\n')
            b[-- l] = 0;

        if (!(r = SET_PUT(set, b, l, k)))
            set->stats.uniq_line ++;
        else {
            set->stats.dups_line ++;

            if (verbose)
                warning("duplicate lines #%zu and #%zu: '%.*s'",
                    r, k, SIZE_AS_INT(l), b);
        }

        SIZE_ADD_EQ(set->stats.line_mem, l);
        k ++;
    }

    if (b != NULL) {
        ASSERT(n > 0);
        free(b);
    }
}

bool SET_IS_EMPTY(
    const struct SET_TYPE* set)
{
    return SET_IMPL_IS_EMPTY(&set->impl);
}

#ifdef SET_NEED_PRINT_SET

void SET_PRINT_SET(
    const struct SET_TYPE* set,
    FILE* file)
{
    SET_IMPL_PRINT(&set->impl, file);
}

#endif // SET_NEED_PRINT_SET

static size_t SET_GET_NODE_SIZE(
    const struct SET_TYPE* set UNUSED)
{
    return sizeof(struct SET_IMPL_NODE_TYPE);
}

#ifdef SET_PATH_TRIE_ELEM_NODE_TYPE
static size_t SET_GET_ELEM_SIZE(
    const struct SET_TYPE* set UNUSED)
{
    return sizeof(struct SET_PATH_TRIE_ELEM_NODE_TYPE);
}
#endif

typedef size_t
    (*SET_STAT_PARAM_SIZE_FUNC_TYPE)(
        const struct SET_TYPE*);
typedef struct size_frac_t
    (*SET_STAT_PARAM_FRAC_FUNC_TYPE)(
        const struct SET_TYPE*);

// stev: cannot use STATIC inside the macro
// below since this macro must be evaluated
// to a constant for it to be expandable as
// element initializer into an array
#define SET_STAT_PARAM_OFFSETOF(f, t)     \
    (                                     \
        TYPEOF_IS(                        \
            ((struct SET_TYPE*) 0)->f, t) \
        ? offsetof(struct SET_TYPE, f)    \
        : SIZE_MAX                        \
    )

#define SET_STAT_PARAM_FUNC_TYPE__(t) \
    stat_param_ ## t ## _func_t
#define SET_STAT_PARAM_FUNC_TYPE_(n) \
    SET_STAT_PARAM_ ## n ## _FUNC_TYPE
// stev: cannot use STATIC inside the macro
// below for the same reason as above
#define SET_STAT_PARAM_FUNC_(n, t, f)       \
    (                                       \
        TYPEOF_IS(&f,                       \
            SET_STAT_PARAM_FUNC_TYPE_(n))   \
        ? (SET_STAT_PARAM_FUNC_TYPE__(t)) f \
        : NULL                              \
    )

#define SET_STAT_PARAM_(n, f, v)           \
    {                                      \
        .name = stat_param_ ## n,          \
        .type = stat_param_ ## f ## _type, \
        .val.f = v                         \
    }
#define SET_STAT_PARAM_SIZE_FIELD(n, f) \
    SET_STAT_PARAM_(n, size_field, \
        SET_STAT_PARAM_OFFSETOF(f, size_t))
#define SET_STAT_PARAM_TIME_FIELD(n, f) \
    SET_STAT_PARAM_(n, time_field, \
        SET_STAT_PARAM_OFFSETOF(f, struct clocks_t))
#define SET_STAT_PARAM_SIZE_FUNC(n, f) \
    SET_STAT_PARAM_(n, size_func, \
        SET_STAT_PARAM_FUNC_(SIZE, size, f))
#define SET_STAT_PARAM_FRAC_FUNC(n, f) \
    SET_STAT_PARAM_(n, frac_func, \
        SET_STAT_PARAM_FUNC_(FRAC, frac, f))


