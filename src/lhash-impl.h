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

#ifndef LHASH_ALLOC_NODE_FUNC
#error  LHASH_ALLOC_NODE_FUNC is not defined
#endif

#ifndef LHASH_ALLOC_OBJ_TYPE
#error  LHASH_ALLOC_OBJ_TYPE is not defined
#endif

#ifdef  LHASH_NAME
#define LHASH_MAKE_NAME__(n, s) n ## _lhash ## s
#define LHASH_MAKE_NAME_(n, s)  LHASH_MAKE_NAME__(n, s)
#define LHASH_MAKE_NAME(s)      LHASH_MAKE_NAME_(LHASH_NAME, _ ## s)
#else
#define LHASH_MAKE_NAME_(s) lhash ## s
#define LHASH_MAKE_NAME(s)  LHASH_MAKE_NAME_(_ ## s)
#endif

#define LHASH_TYPE          LHASH_MAKE_NAME(t)
#define LHASH_NODE_TYPE     LHASH_MAKE_NAME(node_t)

#define LHASH_SET_STATS_TYPE LHASH_MAKE_NAME(set_stats_t)

#define LHASH_INIT          LHASH_MAKE_NAME(init)
#define LHASH_DONE          LHASH_MAKE_NAME(done)

#define LHASH_LOOKUP        LHASH_MAKE_NAME(lookup)
#define LHASH_INSERT        LHASH_MAKE_NAME(insert)
#define LHASH_IS_EMPTY      LHASH_MAKE_NAME(is_empty)
#define LHASH_GET_STRUCT_MEM LHASH_MAKE_NAME(get_struct_mem)
#define LHASH_PRINT         LHASH_MAKE_NAME(print)

#ifdef LHASH_NEED_STATISTICS

#ifdef LHASH_NAME
#define SET_STATS_NAME LHASH_MAKE_NAME_(LHASH_NAME, )
#else
#define SET_STATS_NAME lhash
#endif // LHASH_NAME

#undef  SET_STATS_NEED_CLOCKS
#define SET_STATS_NEED_NODE_SIZES
#undef  SET_STATS_NEED_LINE_SIZES
#include "set-stats-impl.h"

#endif // LHASH_NEED_STATISTICS

struct LHASH_NODE_TYPE
{
#ifdef LHASH_VAL_TYPE
    LHASH_VAL_TYPE val;
#endif
#ifndef LHASH_NEED_NULL_TERM_KEY
    uint8_t len;
#endif
    char str[0];
};

#ifdef LHASH_NEED_STATISTICS
SET_STATS_STRUCT_DECL(insert_ne)
#endif

struct LHASH_TYPE
{
    LHASH_ALLOC_OBJ_TYPE* alloc_obj;
    struct LHASH_NODE_TYPE** table;
    size_t size;
    size_t used;

#ifdef LHASH_NEED_STATISTICS
    struct LHASH_SET_STATS_TYPE stats;
#endif
};

// stev: we presume that the alloc function
// return a pointer to a zeroed-out structure

#define LHASH_NEW_NODE_(s, l, a)      \
    ({                                \
        struct LHASH_NODE_TYPE* __r = \
            LHASH_ALLOC_NODE_FUNC(    \
                hash->alloc_obj,      \
                (l) + 1);             \
        memcpy(__r->str, s, l);       \
        __r->str[l] = 0;              \
        a;                            \
        __r;                          \
    })

#ifdef LHASH_NEED_NULL_TERM_KEY
#define LHASH_NEW_NODE(s, l) \
    LHASH_NEW_NODE_(s, l, )
#else // LHASH_NEED_NULL_TERM_KEY
#define LHASH_NEW_NODE(s, l)         \
    LHASH_NEW_NODE_(s, l, __r->len = \
        l <= UINT8_MAX ? l : 0)
#endif // LHASH_NEED_NULL_TERM_KEY

#ifndef LHASH_PRIME_FUNCS
#define LHASH_PRIME_FUNCS

static bool lhash_is_prime(size_t n)
{
    size_t d = 3, s = 9, i;

    while (s < n && (n % d)) {
        // (a + 2) ^ 2 = a^2 + 4*(a + 1)
        // invariant: s == d^2
        ASSERT_SIZE_ADD_NO_OVERFLOW(d, SZ(2));
        d ++;
        i = SIZE_MUL(d, SZ(4));
        SIZE_ADD_EQ(s, i);
        d ++;
    }
    return s >= n;
}

// $ (cat next-prime.bc; echo 'n=2^32-1; prev_prime(n); n; next_prime(n)')|bc -q
// 4294967291
// 4294967295
// 4294967311

static size_t lhash_next_prime(size_t n)
{
    const size_t N = SZ(4294967291);

    STATIC(SIZE_BIT >= 32);

    n |= 1;
    while (n < N && !lhash_is_prime(n))
        n += 2;

    return n;
}

#endif // LHASH_PRIME_FUNCS

static void LHASH_INIT(
    struct LHASH_TYPE* hash,
    LHASH_ALLOC_OBJ_TYPE* alloc_obj,
    const struct options_t* opt)
{
#ifdef LHASH_NEED_32BIT_OFFSETS
    const size_t p = SZ(4294967291);
#endif
    size_t s = opt->hash_size;

#ifdef LHASH_NEED_32BIT_OFFSETS
    ENSURE(s <= p, "hash-size %zu > %zu -- "
        "the previous prime of UINT32_MAX", s, p);
#endif

    memset(hash, 0, sizeof(struct LHASH_TYPE));

    s = lhash_next_prime(
        s == 0 ? 1024 : s < 32 ? 32 : s);
#ifdef LHASH_NEED_32BIT_OFFSETS
    ASSERT(s < UINT32_MAX);
#endif

    hash->alloc_obj = alloc_obj;
    hash->table = calloc(
        s, sizeof(struct LHASH_NODE_TYPE*));
    ENSURE(hash->table != NULL, "calloc failed");

    hash->size = s;
}

static void LHASH_DONE(
    struct LHASH_TYPE* hash)
{
    free(hash->table);
}

#ifdef LHASH_NEED_NULL_TERM_KEY
#define LHASH_EQ(p, s, l) \
    (!strcmp((p)->str, s))
#else // LHASH_NEED_NULL_TERM_KEY
#define LHASH_EQ(p, s, l)                    \
    ({                                       \
        size_t __n = (p)->len == 0           \
            ? strlen((p)->str) : (p)->len;   \
        __n == l && !memcmp((p)->str, s, l); \
    })
#endif // LHASH_NEED_NULL_TERM_KEY

// stev: the following hash function is a modified
// PJW hash function due to the following work:
//
// Bruno Haible: Demythifying the Aho-Hopcroft-Ullman Hash Function
// http://www.haible.de/bruno/hashfunc.html

#define LHASH_HASH(s, l)                   \
    ({                                     \
        size_t __r = l;                    \
        const char *__p, *__e;             \
        for (__p = s, __e = s + l;         \
             __p < __e;                    \
             __p ++)                       \
            __r = (size_t) *__p +          \
                ((__r << 9) |              \
                 (__r >> (SIZE_BIT - 9))); \
        __r;                               \
    })

#ifndef LHASH_NEED_32BIT_OFFSETS
#define LHASH_NODE_RESULT_TYPE struct LHASH_NODE_TYPE*
#define LHASH_NODE_RESULT_TYPE_CONST const LHASH_NODE_RESULT_TYPE
#define LHASH_NODE_RESULT(p) (*p)
#define LHASH_NULL_RESULT NULL
#else // LHASH_NEED_32BIT_OFFSETS
#define LHASH_NODE_RESULT_TYPE uint32_t
#define LHASH_NODE_RESULT_TYPE_CONST LHASH_NODE_RESULT_TYPE
#define LHASH_NODE_RESULT(p)                   \
    ({                                         \
        size_t __r = PTR_DIFF(p, hash->table); \
        ASSERT(__r < UINT32_MAX);              \
        (uint32_t) (__r + 1);                  \
    })
#define LHASH_NULL_RESULT 0
#endif // LHASH_NEED_32BIT_OFFSETS

// stev: Knuth, TAOCP, vol 3, 3rd edition,
// 6.4 Hashing, Algorithm L, p. 526

#ifdef LHASH_NEED_LOOKUP

static bool LHASH_LOOKUP(
    const struct LHASH_TYPE* hash,
    const char* key, size_t len,
    LHASH_NODE_RESULT_TYPE_CONST* result)
{
    struct LHASH_NODE_TYPE** p;
    size_t h;

    ASSERT(hash->size > 0);

    h = LHASH_HASH(key, len) %
            hash->size;
    for (p = hash->table + h; *p; ) {
        if (LHASH_EQ(*p, key, len)) {
            *result =
                LHASH_NODE_RESULT(p);
            return true;
        }
        if (p == hash->table)
            p += hash->size - 1;
        else
            p --;
    }

    *result = LHASH_NULL_RESULT;
    return false;
}

#endif // LHASH_NEED_LOOKUP

static bool LHASH_INSERT(
    struct LHASH_TYPE* hash,
    const char* key, size_t len,
    LHASH_NODE_RESULT_TYPE* result)
{
    struct LHASH_NODE_TYPE** p;
    size_t h;

    ASSERT(hash->size > 0);

    h = LHASH_HASH(key, len) %
            hash->size;
    for (p = hash->table + h; *p; ) {
        if (LHASH_EQ(*p, key, len)) {
#ifdef LHASH_NEED_STATISTICS
            hash->stats.dups_node ++;
            hash->stats.insert_eq ++;
#endif
            *result =
                LHASH_NODE_RESULT(p);
            return false;
        }
#ifdef LHASH_NEED_STATISTICS
        else
            hash->stats.insert_ne ++;
#endif
        if (p == hash->table)
            p += hash->size - 1;
        else
            p --;
    }

    ENSURE(hash->used < hash->size - 1,
        "linear hash table overflow"); 

    *p = LHASH_NEW_NODE(key, len);
    *result = LHASH_NODE_RESULT(p);
#ifdef LHASH_NEED_STATISTICS
    hash->stats.uniq_node ++;
#endif
    hash->used ++;
    return true;
}

#ifdef LHASH_NEED_IS_EMPTY

static bool LHASH_IS_EMPTY(
    const struct LHASH_TYPE* hash)
{
    return hash->used == 0;
}

#endif // LHASH_NEED_IS_EMPTY

static size_t LHASH_GET_STRUCT_MEM(
    const struct LHASH_TYPE* hash)
{
    size_t r = sizeof(struct LHASH_NODE_TYPE*);

    SIZE_MUL_EQ(r, hash->size);
    SIZE_ADD_EQ(r, sizeof(struct LHASH_TYPE));

    return r;
}

#ifndef LHASH_NEED_32BIT_OFFSETS
#define LHASH_PRINT_ONE(f, p)            \
    do {                                 \
        STATIC(TYPEOF_IS(f, FILE*));     \
        STATIC(TYPEOF_IS(p, const struct \
            LHASH_NODE_TYPE*));          \
        fputs((p)->str, f);              \
    } while (0)
#else // LHASH_NEED_32BIT_OFFSETS
#define LHASH_PRINT_ONE(t, f, p)        \
    do {                                \
        STATIC(TYPEOF_IS(t, const       \
            struct LHASH_TYPE*));       \
        STATIC(TYPEOF_IS(f, FILE*));    \
        STATIC(TYPEOF_IS(p, uint32_t)); \
        uint32_t __k = p;               \
        ASSERT(__k > 0);                \
        __k --;                         \
        ASSERT(__k < (t)->size);        \
        fputs((t)->table[__k]->str, f); \
    } while (0)
#endif // LHASH_NEED_32BIT_OFFSETS

#ifdef LHASH_NEED_PRINT

static void LHASH_PRINT(
    const struct LHASH_TYPE* hash,
    FILE* file)
{
    struct LHASH_NODE_TYPE **p, **e;

    for (p = hash->table,
         e = p + hash->size;
         p < e;
         p ++) {
        if (*p != NULL) {
            fputs((*p)->str, file);
            fputc('\n', file);
        }
    }
}

#endif // LHASH_NEED_PRINT


