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

#ifndef LHASH_NEED_32BIT_OFFSETS

#ifndef LHASH_ALLOC_NODE_FUNC
#error  LHASH_ALLOC_NODE_FUNC is not defined
#endif

#ifndef LHASH_ALLOC_OBJ_TYPE
#error  LHASH_ALLOC_OBJ_TYPE is not defined
#endif

#endif // LHASH_NEED_32BIT_OFFSETS

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
#define LHASH_REHASH_TYPE   LHASH_MAKE_NAME(rehash_t)

#define LHASH_SET_STATS_TYPE LHASH_MAKE_NAME(set_stats_t)

#define LHASH_INIT          LHASH_MAKE_NAME(init)
#define LHASH_DONE          LHASH_MAKE_NAME(done)

#ifdef LHASH_NEED_32BIT_OFFSETS
#define LHASH_NODE_ALLOC_TYPE \
                            LHASH_MAKE_NAME(node_alloc_t)
#define LHASH_NODE_ALLOC_INIT \
                            LHASH_MAKE_NAME(node_alloc_init)
#define LHASH_NODE_ALLOC_DONE \
                            LHASH_MAKE_NAME(node_alloc_done)
#define LHASH_NODE_ALLOC_ALLOCATE \
                            LHASH_MAKE_NAME(node_alloc_allocate)
#define LHASH_NODE_ALLOC_GET_OBJ_MEM \
                            LHASH_MAKE_NAME(node_alloc_get_obj_mem)
#define LHASH_NODE_ALLOC_GET_STRUCT_MEM \
                            LHASH_MAKE_NAME(node_alloc_get_struct_mem)
#endif // LHASH_NEED_32BIT_OFFSETS

#define LHASH_LOOKUP        LHASH_MAKE_NAME(lookup)
#define LHASH_INSERT        LHASH_MAKE_NAME(insert)
#define LHASH_IS_EMPTY      LHASH_MAKE_NAME(is_empty)
#define LHASH_GET_STRUCT_MEM LHASH_MAKE_NAME(get_struct_mem)
#define LHASH_PRINT         LHASH_MAKE_NAME(print)

#ifdef LHASH_NEED_32BIT_OFFSETS
#define LHASH_PRINT_ONE     LHASH_MAKE_NAME(print_one)
#endif

struct LHASH_REHASH_TYPE
{
    float size;
    float load;
};

#ifdef LHASH_NEED_STATISTICS

#ifdef LHASH_NAME
#define SET_STATS_NAME LHASH_MAKE_NAME_(LHASH_NAME, )
#else
#define SET_STATS_NAME lhash
#endif // LHASH_NAME

#undef  SET_STATS_STRUCT_DECL_CLOCKS
#define SET_STATS_STRUCT_DECL_CLOCKS() \
    rehash_time

#define SET_STATS_NEED_CLOCKS
#define SET_STATS_NEED_NODE_SIZES
#undef  SET_STATS_NEED_LINE_SIZES
#include "set-stats-impl.h"

#include "clocks-impl.h"

#endif // LHASH_NEED_STATISTICS

#ifndef LHASH_NEED_32BIT_OFFSETS
#define LHASH_PTR_TYPE struct LHASH_NODE_TYPE*
#define LHASH_PTR_TYPE_CONST const LHASH_PTR_TYPE
#define LHASH_PTR_NULL NULL
#else
#define LHASH_PTR_TYPE uint32_t
#define LHASH_PTR_TYPE_CONST LHASH_PTR_TYPE
#define LHASH_PTR_NULL 0
#endif

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

SET_STATS_STRUCT_DECL(
#ifdef LHASH_NEED_32BIT_OFFSETS
    node_struct,
    node_mem,
#endif
    rehash_op,
    rehash_hit,
    insert_hit,
    insert_ne
)

#undef SET_STATS_STRUCT_DECL_CLOCKS

#endif // LHASH_NEED_STATISTICS

#ifdef LHASH_NEED_32BIT_OFFSETS

#define OBJ_ALLOC_NAME      LHASH_MAKE_NAME(node)
#undef  OBJ_ALLOC_OBJ_SIZE
#undef  OBJ_ALLOC_OBJ_ALIGN
#define OBJ_ALLOC_NODE_BITS 0
#ifdef  LHASH_NEED_STATISTICS
#define OBJ_ALLOC_NEED_STATISTICS
#else
#undef  OBJ_ALLOC_NEED_STATISTICS
#endif
#include "obj-alloc-impl.h"

#endif // LHASH_NEED_32BIT_OFFSETS

struct LHASH_TYPE
{
#ifndef LHASH_NEED_32BIT_OFFSETS
    LHASH_ALLOC_OBJ_TYPE* alloc_obj;
#else
    struct LHASH_NODE_ALLOC_TYPE node_alloc;
#endif
    LHASH_PTR_TYPE* table;
    size_t size;
    size_t used;

    struct LHASH_REHASH_TYPE    rehash;
#ifdef LHASH_NEED_STATISTICS
    struct LHASH_SET_STATS_TYPE stats;
#endif
};

// stev: we presume that the alloc function
// return a pointer to a zeroed-out structure

#ifndef LHASH_NEED_32BIT_OFFSETS
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
#else // LHASH_NEED_32BIT_OFFSETS
#define LHASH_NEW_NODE_(s, l, a)         \
    ({                                   \
        uint32_t __b = 0;                \
        size_t __n = sizeof(             \
            struct LHASH_NODE_TYPE);     \
        size_t __a = MEM_ALIGNOF(        \
            struct LHASH_NODE_TYPE);     \
        struct LHASH_NODE_TYPE* __r =    \
            LHASH_NODE_ALLOC_ALLOCATE(   \
                &hash->node_alloc,       \
                SIZE_ADD(__n, (l) + 1),  \
                __a, &__b);              \
        ENSURE(__r != NULL,              \
            "hash node alloc failed");   \
        ASSERT(__b > 0);                 \
        hash->stats.node_struct ++;      \
        hash->stats.node_mem += (l) + 1; \
        memcpy(__r->str, s, l);          \
        __r->str[l] = 0;                 \
        a;                               \
        __b;                             \
    })
#endif // LHASH_NEED_32BIT_OFFSETS

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
        VERIFY_SIZE_ADD_NO_OVERFLOW(d, SZ(2));
        d ++;
        i = VERIFY_SIZE_MUL(d, SZ(4));
        VERIFY_SIZE_ADD_EQ(s, i);
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

// stev: Knuth, TAOCP, vol 3, 3rd edition,
// 6.4 Hashing, p. 528
#define LHASH_REHASH_LOAD 0.75f

// stev: double the size of the table
// each time decided to enlarge it; a
// variant would be to double its size
// every second time enlarging it --
// which amounts to the factor below
// be sqrt(2) ~= 1.4142f
#define LHASH_REHASH_SIZE 2.0f

static void LHASH_INIT(
    struct LHASH_TYPE* hash,
#ifndef LHASH_NEED_32BIT_OFFSETS
    LHASH_ALLOC_OBJ_TYPE* alloc_obj,
#endif
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

#define FLOAT_EPSILON 0.1f
    if (!isnan(opt->rehash_load)) {
        if (!FLOAT_GS(opt->rehash_load, 0.0f) ||
            !FLOAT_LS(opt->rehash_load, 1.0f))
            INVALID_ARG("%f", opt->rehash_load);
    }
    if (!isnan(opt->rehash_size)) {
        if (!FLOAT_GS(opt->rehash_size, 0.0f) ||
            // stev: this upper-bound is arbitrary!
            !FLOAT_LE(opt->rehash_size, 4.0f))
            INVALID_ARG("%f", opt->rehash_size);
    }
#undef FLOAT_EPSILON

    memset(hash, 0, sizeof(struct LHASH_TYPE));

    s = lhash_next_prime(
        s == 0 ? 1024 : s < 32 ? 32 : s);
#ifdef LHASH_NEED_32BIT_OFFSETS
    ASSERT(s < UINT32_MAX);
#endif

#ifndef LHASH_NEED_32BIT_OFFSETS
    hash->alloc_obj = alloc_obj;
#else
    LHASH_NODE_ALLOC_INIT(&hash->node_alloc, s, 0); //!!! opt->???_size
#endif
    hash->table = calloc(s, sizeof(LHASH_PTR_TYPE));
    ENSURE(hash->table != NULL, "calloc failed");

    hash->size = s;

    hash->rehash.size = !isnan(opt->rehash_size)
        ? opt->rehash_size
        : LHASH_REHASH_SIZE;
    hash->rehash.load = !isnan(opt->rehash_load)
        ? opt->rehash_load
        : LHASH_REHASH_LOAD;
}

static void LHASH_DONE(
    struct LHASH_TYPE* hash)
{
#ifdef LHASH_NEED_32BIT_OFFSETS
    LHASH_NODE_ALLOC_DONE(&hash->node_alloc);
#endif
    free(hash->table);
}

#ifndef LHASH_NEED_32BIT_OFFSETS
#define LHASH_PTR_DEREF(p) (p)
#else
#define LHASH_PTR_DEREF(p)                    \
    (                                         \
        (struct LHASH_NODE_TYPE*)             \
        OBJ_ALLOC_DEREF(&hash->node_alloc, p) \
    )
#endif

#ifdef LHASH_NEED_NULL_TERM_KEY
#define LHASH_EQ(p, s, l) \
    (!strcmp(LHASH_PTR_DEREF(p)->str, s))
#else // LHASH_NEED_NULL_TERM_KEY
#define LHASH_EQ(p, s, l)                    \
    ({                                       \
        struct LHASH_NODE_TYPE* __p =        \
            LHASH_PTR_DEREF(p);              \
        size_t __n = __p->len == 0           \
            ? strlen(__p->str) : __p->len;   \
        __n == l && !memcmp(__p->str, s, l); \
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

#define LHASH_ASSERT_INVARIANTS(h) \
    do {                           \
        ASSERT(h->size > 0);       \
        ASSERT(h->used < h->size); \
    } while (0)

// stev: Knuth, TAOCP, vol 3, 3rd edition,
// 6.4 Hashing, Algorithm L, p. 526

#ifdef LHASH_NEED_LOOKUP

static bool LHASH_LOOKUP(
    const struct LHASH_TYPE* hash,
    const char* key, size_t len,
    LHASH_PTR_TYPE_CONST* result)
{
    LHASH_PTR_TYPE* p;
    size_t h;

    LHASH_ASSERT_INVARIANTS(hash);

    h = LHASH_HASH(key, len);
    for (p = hash->table + h % hash->size; *p; ) {
        if (LHASH_EQ(*p, key, len)) {
            *result = *p;
            return true;
        }
        if (p == hash->table)
            p += hash->size - 1;
        else
            p --;
    }

    *result = LHASH_PTR_NULL;
    return false;
}

#endif // LHASH_NEED_LOOKUP

// stev: TODO: promote SIZE_MUL_FLOAT
// to a global scope ('int-traits.h'?)
#define SIZE_MUL_FLOAT(x, f)            \
    ({                                  \
        float __r;                      \
        STATIC(TYPEOF_IS(f, float));    \
        STATIC(TYPEOF_IS_SIZET(x));     \
        __r = (float) (x) * (f);        \
        VERIFY(__r < (float) SIZE_MAX); \
        (size_t) __r;                   \
    })

#ifdef LHASH_NEED_NULL_TERM_KEY
#define LHASH_KEY_LENGTH(p) \
    (                       \
        strlen((p)->str)    \
    )
#else // LHASH_NEED_NULL_TERM_KEY
#define LHASH_KEY_LENGTH(p) \
    (                       \
        (p)->len == 0       \
        ? strlen((p)->str)  \
        : (p)->len          \
    )
#endif // LHASH_NEED_NULL_TERM_KEY

static void LHASH_REHASH(
    struct LHASH_TYPE* hash)
{
    typedef LHASH_PTR_TYPE* ptr_t;
#ifdef LHASH_NEED_STATISTICS
    struct clocks_t c;
    struct utime_t u;
#endif
    ptr_t t, p, e, q;
    size_t h, s;

#ifdef LHASH_NEED_STATISTICS
    utime_init(&u);
#endif

    LHASH_ASSERT_INVARIANTS(hash);

    s = lhash_next_prime(
        SIZE_MUL_FLOAT(hash->size,
            hash->rehash.size));
    VERIFY(s > hash->size);

    t = calloc(s, sizeof(LHASH_PTR_TYPE));
    ENSURE(t != NULL, "calloc failed");

    for (p = hash->table,
         e = p + hash->size;
         p < e;
         p ++) {
        const struct LHASH_NODE_TYPE* n;
        size_t l;

        if (*p == LHASH_PTR_NULL)
            continue;

        n = LHASH_PTR_DEREF(*p);
        l = LHASH_KEY_LENGTH(n);
        h = LHASH_HASH(n->str, l);

        for (q = t + h % s; *q; ) {
#ifdef LHASH_NEED_STATISTICS
            hash->stats.rehash_hit ++;
#endif
            if (q == t)
                q += s - 1;
            else
                q --;
        }

        *q = *p;
    }

    free(hash->table);

    hash->table = t;
    hash->size = s;
    // the new size > the old size =>
    // the invariants are preserved
#ifdef LHASH_NEED_STATISTICS
    c = utime_clocks(&u);
    clocks_add(&hash->stats.rehash_time, &c);
    hash->stats.rehash_op ++;
#endif
}

static bool LHASH_INSERT(
    struct LHASH_TYPE* hash,
    const char* key, size_t len,
    LHASH_PTR_TYPE* result)
{
    LHASH_PTR_TYPE* p;
    size_t h, s;

    LHASH_ASSERT_INVARIANTS(hash);

    h = LHASH_HASH(key, len);
    for (p = hash->table + h % hash->size; *p; ) {
        if (LHASH_EQ(*p, key, len)) {
#ifdef LHASH_NEED_STATISTICS
            hash->stats.dups_node ++;
            hash->stats.insert_eq ++;
#endif
            *result = *p;
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

    s = SIZE_MUL_FLOAT(hash->size,
            hash->rehash.load);
    VERIFY(s > 0 && s < hash->size);
    // => s <= hash->size - 1

    if (hash->used < s) {
        // => hash->used < hash->size - 1
        goto new_node;
    }

    // stev: let 'S' be the size of the hash table
    // before 'LHASH_REHASH' increases it strictly;
    // thus the 2nd invariant gives: hash->used < S

    LHASH_REHASH(hash);
    // stev: we have that:
    //   hash->used < hash->size - 1
    // indeed:
    //   hash->size increased strictly =>
    //   S < hash->size <=>
    //   S <= hash->size - 1 =>
    //   hash->used < S <= hash->size - 1

    for (p = hash->table + h % hash->size; *p; ) {
#ifdef LHASH_NEED_STATISTICS
        hash->stats.insert_hit ++;
#endif
        if (p == hash->table)
            p += hash->size - 1;
        else
            p --;
    }

new_node:
    ASSERT(hash->used < hash->size - 1);

    *result = *p = LHASH_NEW_NODE(key, len);
#ifdef LHASH_NEED_STATISTICS
    hash->stats.uniq_node ++;
#endif
    hash->used ++;
    return true;
}

#undef SIZE_MUL_FLOAT

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
#ifdef LHASH_NEED_32BIT_OFFSETS
    size_t r, s;
#else
    size_t r;
#endif

    LHASH_ASSERT_INVARIANTS(hash);

    r = hash->size - hash->used;
    SIZE_MUL_EQ(r, sizeof(LHASH_PTR_TYPE));
    SIZE_ADD_EQ(r, sizeof(struct LHASH_TYPE));

#ifdef LHASH_NEED_32BIT_OFFSETS
    s = LHASH_NODE_ALLOC_GET_STRUCT_MEM(
        &hash->node_alloc);
    SIZE_ADD_EQ(r, s);
#endif

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

static inline void LHASH_PRINT_ONE(
    const struct LHASH_TYPE* hash, FILE* file,
    LHASH_PTR_TYPE ptr)
{
    fputs(LHASH_PTR_DEREF(ptr)->str, file);
}

#endif // LHASH_NEED_32BIT_OFFSETS

#ifdef LHASH_NEED_PRINT

static void LHASH_PRINT(
    const struct LHASH_TYPE* hash,
    FILE* file)
{
    LHASH_PTR_TYPE* p;
    LHASH_PTR_TYPE* e;

    for (p = hash->table,
         e = p + hash->size;
         p < e;
         p ++) {
        if (*p != LHASH_PTR_NULL) {
            fputs(LHASH_PTR_DEREF(*p)->str, file);
            fputc('\n', file);
        }
    }
}

#endif // LHASH_NEED_PRINT


