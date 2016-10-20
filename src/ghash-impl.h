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

#ifndef GHASH_ALLOC_NODE_FUNC
#error  GHASH_ALLOC_NODE_FUNC is not defined
#endif

#ifndef GHASH_ALLOC_OBJ_TYPE
#error  GHASH_ALLOC_OBJ_TYPE is not defined
#endif

#ifdef  GHASH_NAME
#define GHASH_MAKE_NAME__(n, s) n ## _ghash ## s
#define GHASH_MAKE_NAME_(n, s)  GHASH_MAKE_NAME__(n, s)
#define GHASH_MAKE_NAME(s)      GHASH_MAKE_NAME_(GHASH_NAME, _ ## s)
#else
#define GHASH_MAKE_NAME_(s) ghash ## s
#define GHASH_MAKE_NAME(s)  GHASH_MAKE_NAME_(_ ## s)
#endif

#define GHASH_TYPE          GHASH_MAKE_NAME(t)
#define GHASH_NODE_TYPE     GHASH_MAKE_NAME(node_t)

#define GHASH_INIT          GHASH_MAKE_NAME(init)
#define GHASH_DONE          GHASH_MAKE_NAME(done)

#define GHASH_HASH          GHASH_MAKE_NAME(hash)

#define GHASH_LOOKUP        GHASH_MAKE_NAME(lookup)
#define GHASH_INSERT        GHASH_MAKE_NAME(insert)
#define GHASH_IS_EMPTY      GHASH_MAKE_NAME(is_empty)
#define GHASH_GET_STRUCT_MEM GHASH_MAKE_NAME(get_struct_mem)
#define GHASH_PRINT_NODE    GHASH_MAKE_NAME(print_node)
#define GHASH_PRINT         GHASH_MAKE_NAME(print)

#ifdef GHASH_NEED_STATISTICS
#define _GL_HASH_NEED_STATS_INFO
#endif
#include "hash.h"

struct GHASH_NODE_TYPE
{
#ifdef GHASH_VAL_TYPE
    GHASH_VAL_TYPE val;
#endif
#ifndef GHASH_NEED_NULL_TERM_KEY
    size_t len;
#endif
    char* str;
};

struct GHASH_TYPE
{
    GHASH_ALLOC_OBJ_TYPE* alloc_obj;
    struct hash_table* table;

    struct hash_tuning tuning;
#ifdef GHASH_NEED_STATISTICS
    Hash_stats_info stats;
#endif
};

// stev: we presume that the alloc function
// return a pointer to a zeroed-out structure

#define GHASH_NEW_NODE_(s, l, a)      \
    ({                                \
        struct GHASH_NODE_TYPE* __r = \
            GHASH_ALLOC_NODE_FUNC(    \
                hash->alloc_obj,      \
                (l) + 1);             \
        memcpy(__r->str, s, l);       \
        __r->str[l] = 0;              \
        a;                            \
        __r;                          \
    })

#ifdef GHASH_NEED_NULL_TERM_KEY
#define GHASH_NEW_NODE(s, l) \
    GHASH_NEW_NODE_(s, l, )
#else // GHASH_NEED_NULL_TERM_KEY
#define GHASH_NEW_NODE(s, l) \
    GHASH_NEW_NODE_(s, l, __r->len = l)
#endif // GHASH_NEED_NULL_TERM_KEY

// stev: the following hash function is a modified
// PJW hash function due to the following work:
//
// Bruno Haible: Demythifying the Aho-Hopcroft-Ullman Hash Function
// http://www.haible.de/bruno/hashfunc.html

static size_t GHASH_HASH(
    const struct GHASH_NODE_TYPE* node,
    size_t table_size)
{
    size_t r =
#ifdef GHASH_NEED_NULL_TERM_KEY
        strlen(node->str);
#else
        node->len;
#endif
    const char *p, *e;

    for (p = node->str,
         e = p + r;
         p < e;
         p ++)
        r = (size_t) *p +
            ((r << 9) |
             (r >> (SIZE_BIT - 9)));

    return r % table_size;
}

static bool GHASH_EQ(
    const struct GHASH_NODE_TYPE* src,
    const struct GHASH_NODE_TYPE* dest)
{
#ifdef GHASH_NEED_NULL_TERM_KEY
    return !strcmp(src->str, dest->str);
#else
    return src->len == dest->len &&
        !memcmp(src->str, dest->str, src->len);
#endif
}

// stev: gnulib/hash.c: DEFAULT_GROWTH_THRESHOLD
#define GHASH_REHASH_LOAD 0.8f

// stev: gnulib/hash.c: DEFAULT_GROWTH_FACTOR
#define GHASH_REHASH_SIZE 1.414f

static void GHASH_INIT(
    struct GHASH_TYPE* hash,
    GHASH_ALLOC_OBJ_TYPE* alloc_obj,
    const struct options_t* opt)
{
    memset(hash, 0, sizeof(struct GHASH_TYPE));

    hash->tuning.shrink_threshold = 0.0f;
    hash->tuning.shrink_factor = 1.0f;
    hash->tuning.growth_threshold =
        !isnan(opt->rehash_load)
        ? opt->rehash_load
        : GHASH_REHASH_LOAD;
    hash->tuning.growth_factor =
        !isnan(opt->rehash_size)
        ? opt->rehash_size
        : GHASH_REHASH_SIZE;
    hash->tuning.is_n_buckets = false;

    hash->alloc_obj = alloc_obj;
    hash->table = hash_initialize(
        opt->hash_size,
        &hash->tuning,
        (Hash_hasher)
        GHASH_HASH,
        (Hash_comparator)
        GHASH_EQ,
        NULL);
    ENSURE(hash->table != NULL,
        "table initialization failed");
}

static void GHASH_DONE(
    struct GHASH_TYPE* hash)
{
    hash_free(hash->table);
}

#ifdef GHASH_NEED_LOOKUP

static bool GHASH_LOOKUP(
    const struct GHASH_TYPE* hash,
    const char* key, size_t len,
    const struct GHASH_NODE_TYPE** result)
{
    struct GHASH_NODE_TYPE n;

#ifndef GHASH_NEED_NULL_TERM_KEY
    n.len = len;
#endif
    n.str = CONST_CAST(key, char); //!!!HACKISH
    *result = hash_lookup(hash->table, &n);

    return *result != NULL;
}

#endif // GHASH_NEED_LOOKUP

static bool GHASH_INSERT(
    struct GHASH_TYPE* hash,
    const char* key, size_t len,
    struct GHASH_NODE_TYPE** result)
{
    struct GHASH_NODE_TYPE n;
    void **r = NULL;

#ifndef GHASH_NEED_NULL_TERM_KEY
    n.len = len;
#endif
    n.str = CONST_CAST(key, char); //!!!HACKISH

    if (!hash_insert(hash->table, &n, &r)) {
        ASSERT(r != NULL);
        *result = *r;
        return false;
    }

    ASSERT(r != NULL);
    *result = *r =
        GHASH_NEW_NODE(key, len);
    return true;
}

#ifdef GHASH_NEED_IS_EMPTY

static bool GHASH_IS_EMPTY(
    const struct GHASH_TYPE* hash)
{
    return hash_get_n_entries(hash->table) == 0;
}

#endif // GHASH_NEED_IS_EMPTY

static size_t GHASH_GET_STRUCT_MEM(
    const struct GHASH_TYPE* hash)
{
    size_t r = hash_get_struct_mem(hash->table);

    SIZE_ADD_EQ(r, sizeof(struct GHASH_TYPE));

    return r;
}

#define GHASH_PRINT_ONE(f, p)            \
    do {                                 \
        STATIC(TYPEOF_IS(f, FILE*));     \
        STATIC(TYPEOF_IS(p, const struct \
            GHASH_NODE_TYPE*));          \
        fputs((p)->str, f);              \
    } while (0)

#ifdef GHASH_NEED_PRINT

static bool GHASH_PRINT_NODE(
    const struct GHASH_NODE_TYPE* node,
    FILE* file)
{
    fputs(node->str, file);
    fputc('\n', file);
    return true;
}

static void GHASH_PRINT(
    const struct GHASH_TYPE* hash,
    FILE* file)
{
    hash_do_for_each(
        hash->table,
        (Hash_processor)
        GHASH_PRINT_NODE,
        file);
}

#endif // GHASH_NEED_PRINT


