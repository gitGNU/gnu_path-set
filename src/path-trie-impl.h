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

#ifndef PATH_TRIE_ELEM_NAME
#error  PATH_TRIE_ELEM_NAME is not defined
#endif

#ifndef PATH_TRIE_VAL_TYPE
#error  PATH_TRIE_VAL_TYPE is not defined
#endif

#ifndef PATH_TRIE_DEFAULT_SEP
#error  PATH_TRIE_DEFAULT_SEP is not defined
#endif

#ifndef PATH_TRIE_ALLOC_NODE_FUNC
#error  PATH_TRIE_ALLOC_NODE_FUNC is not defined
#endif

#ifndef PATH_TRIE_ALLOC_OBJ_TYPE
#error  PATH_TRIE_ALLOC_OBJ_TYPE is not defined
#endif

#ifdef  PATH_TRIE_NAME
#define PATH_TRIE_MAKE_NAME__(n, s) n ## _path_trie ## s
#define PATH_TRIE_MAKE_NAME_(n, s) PATH_TRIE_MAKE_NAME__(n, s)
#define PATH_TRIE_MAKE_NAME(s)  PATH_TRIE_MAKE_NAME_(PATH_TRIE_NAME, _ ## s)
#else
#define PATH_TRIE_MAKE_NAME_(s) path_trie ## s
#define PATH_TRIE_MAKE_NAME(s)  PATH_TRIE_MAKE_NAME_(_ ## s)
#endif

#define PATH_TRIE_TYPE          PATH_TRIE_MAKE_NAME(t)
#define PATH_TRIE_NODE_TYPE     PATH_TRIE_MAKE_NAME(node_t)

#define PATH_TRIE_SET_STATS_TYPE PATH_TRIE_MAKE_NAME(set_stats_t)

#define PATH_TRIE_INIT          PATH_TRIE_MAKE_NAME(init)
#define PATH_TRIE_DONE          PATH_TRIE_MAKE_NAME(done)

#ifdef PATH_TRIE_NEED_NODE_32BIT_OFFSETS
#define PATH_TRIE_TABLE_TYPE    PATH_TRIE_MAKE_NAME(table_t)
#define PATH_TRIE_ALLOC_NODE    PATH_TRIE_MAKE_NAME(alloc_node)
#define PATH_TRIE_GET_NODE_COUNT PATH_TRIE_MAKE_NAME(get_node_count)
#endif

#define PATH_TRIE_SPLIT         PATH_TRIE_MAKE_NAME(split)
#define PATH_TRIE_GET_ELEM      PATH_TRIE_MAKE_NAME(get_elem)
#define PATH_TRIE_PUT_ELEM      PATH_TRIE_MAKE_NAME(put_elem)
#define PATH_TRIE_LOOKUP        PATH_TRIE_MAKE_NAME(lookup)
#define PATH_TRIE_INSERT        PATH_TRIE_MAKE_NAME(insert)
#define PATH_TRIE_IS_EMPTY      PATH_TRIE_MAKE_NAME(is_empty)
#define PATH_TRIE_GET_STRUCT_MEM PATH_TRIE_MAKE_NAME(get_struct_mem)
#define PATH_TRIE_PRINT         PATH_TRIE_MAKE_NAME(print)

#define PATH_TRIE_NODE_STACK_NAME \
                                PATH_TRIE_MAKE_NAME(node)
#define PATH_TRIE_NODE_STACK_TYPE \
                                PATH_TRIE_MAKE_NAME(node_stack_t)

#define PATH_TRIE_PRINTER_TYPE  PATH_TRIE_MAKE_NAME(printer_t)
#define PATH_TRIE_PRINTER_INIT  PATH_TRIE_MAKE_NAME(printer_init)
#define PATH_TRIE_PRINTER_DONE  PATH_TRIE_MAKE_NAME(printer_done)
#define PATH_TRIE_PRINTER_PRINT PATH_TRIE_MAKE_NAME(printer_print)

#define PATH_TRIE_MAKE_ELEM_NAME__(n, s) \
                                n ## _ ## s
#define PATH_TRIE_MAKE_ELEM_NAME_(n, s) \
                                PATH_TRIE_MAKE_ELEM_NAME__(n, s)
#define PATH_TRIE_MAKE_ELEM_NAME(s) \
                                PATH_TRIE_MAKE_ELEM_NAME_(PATH_TRIE_ELEM_NAME, s)

#define PATH_TRIE_ELEM_TYPE     PATH_TRIE_MAKE_ELEM_NAME(t)
#define PATH_TRIE_ELEM_NODE_TYPE_ \
                                PATH_TRIE_MAKE_ELEM_NAME(node_t)

#define PATH_TRIE_ELEM_INIT     PATH_TRIE_MAKE_ELEM_NAME(init)
#define PATH_TRIE_ELEM_DONE     PATH_TRIE_MAKE_ELEM_NAME(done)
#define PATH_TRIE_ELEM_LOOKUP   PATH_TRIE_MAKE_ELEM_NAME(lookup)
#define PATH_TRIE_ELEM_INSERT   PATH_TRIE_MAKE_ELEM_NAME(insert)

#define PATH_TRIE_ELEM_GET_NODE_SIZE \
                                PATH_TRIE_MAKE_ELEM_NAME(get_node_size)
#define PATH_TRIE_ELEM_GET_STRUCT_MEM \
                                PATH_TRIE_MAKE_ELEM_NAME(get_struct_mem)

#define PATH_TRIE_ELEM_PRINTER_TYPE \
                                PATH_TRIE_MAKE_ELEM_NAME(printer_t)
#define PATH_TRIE_ELEM_PRINTER_INIT \
                                PATH_TRIE_MAKE_ELEM_NAME(printer_init)
#define PATH_TRIE_ELEM_PRINTER_DONE \
                                PATH_TRIE_MAKE_ELEM_NAME(printer_done)
#define PATH_TRIE_ELEM_PRINTER_PRINT_ONE \
                                PATH_TRIE_MAKE_ELEM_NAME(printer_print_one)
#define PATH_TRIE_ELEM_PRINTER_PRINT \
                                PATH_TRIE_MAKE_ELEM_NAME(printer_print)

#ifdef PATH_TRIE_NEED_STATISTICS

#ifdef PATH_TRIE_NAME
#undef  SET_STATS_NAME
#define SET_STATS_NAME PATH_TRIE_MAKE_NAME_(PATH_TRIE_NAME, )
#else
#undef  SET_STATS_NAME
#define SET_STATS_NAME path_trie
#endif // PATH_TRIE_NAME

#undef  SET_STATS_NEED_CLOCKS
#define SET_STATS_NEED_NODE_SIZES
#undef  SET_STATS_NEED_LINE_SIZES
#include "set-stats-impl.h"

#endif // PATH_TRIE_NEED_STATISTICS

// stev: the trie implementations below is due to the work of
// Bentley and Sedgewick; see the web page and the papers
// listed below:
//
// Ternary Search Trees
// http://www.cs.princeton.edu/~rs/strings/
//
// Bentley and Sedgewick: Ternary Search Trees
// Dr. Dobbs Journal April, 1998
// http://www.drdobbs.com/database/ternary-search-trees/184410528
//
// Bentley and Sedgewick: Fast Algorithms for Sorting
//   and Searching Strings
// SODA '97: Proceedings of the eighth annual ACM-SIAM
//   symposium on Discrete algorithms,
//   pp. 360-369, 1997
// http://www.cs.princeton.edu/~rs/strings/paper.pdf

#ifndef PATH_TRIE_NEED_ELEM_32BIT_OFFSETS
#define PATH_TRIE_ELEM_NODE_TYPE struct PATH_TRIE_ELEM_NODE_TYPE_*
#define PATH_TRIE_ELEM_NODE_TYPE_CONST const PATH_TRIE_ELEM_NODE_TYPE
#define PATH_TRIE_ELEM_NULL NULL
#else
#define PATH_TRIE_ELEM_NODE_TYPE uint32_t
#define PATH_TRIE_ELEM_NODE_TYPE_CONST PATH_TRIE_ELEM_NODE_TYPE
#define PATH_TRIE_ELEM_NULL 0
#endif

#ifndef PATH_TRIE_NEED_NODE_32BIT_OFFSETS
#define PATH_TRIE_PTR_TYPE struct PATH_TRIE_NODE_TYPE*
#define PATH_TRIE_PTR_NULL NULL
#else
#define PATH_TRIE_PTR_TYPE uint32_t
#define PATH_TRIE_PTR_NULL 0
#endif

PACKED
struct PATH_TRIE_NODE_TYPE
{
    PACKED PATH_TRIE_ELEM_NODE_TYPE_CONST elem;

    PACKED PATH_TRIE_PTR_TYPE lo;
    PACKED PATH_TRIE_PTR_TYPE hi;

    // stev: note that 'elem' is the discriminator
    // attached to the union below by the conditions:
    //   elem != PATH_TRIE_ELEM_NULL => use 'eq'
    //   elem == PATH_TRIE_ELEM_NULL => use 'val'
    PACKED union {
        PATH_TRIE_PTR_TYPE eq  PACKED;
        PATH_TRIE_VAL_TYPE val PACKED;
    };
};

#ifdef PATH_TRIE_NEED_STATISTICS
SET_STATS_STRUCT_DECL(insert_lt, insert_gt)
#endif

#ifdef PATH_TRIE_NEED_NODE_32BIT_OFFSETS
struct PATH_TRIE_TABLE_TYPE
{
    struct PATH_TRIE_NODE_TYPE* ptr;
    size_t                      size;
    PATH_TRIE_PTR_TYPE          end;
};
#endif

struct PATH_TRIE_TYPE
{
    struct PATH_TRIE_ELEM_TYPE  elem_set;
#ifndef PATH_TRIE_NEED_NODE_32BIT_OFFSETS
    PATH_TRIE_ALLOC_OBJ_TYPE*   alloc_obj;
#else
    struct PATH_TRIE_TABLE_TYPE table;
#endif
    PATH_TRIE_PTR_TYPE          root_node;

    const char* sep_set;
    size_t      sep_len;

#ifdef PATH_TRIE_NEED_STATISTICS
    struct PATH_TRIE_SET_STATS_TYPE stats;
#endif
};

static void PATH_TRIE_INIT(
    struct PATH_TRIE_TYPE* trie,
    PATH_TRIE_ALLOC_OBJ_TYPE* alloc_obj,
    const struct options_t* opt)
{
#ifdef PATH_TRIE_NEED_NODE_32BIT_OFFSETS
    const size_t n = sizeof(struct PATH_TRIE_NODE_TYPE);

    if (opt->node_size >= UINT32_MAX ||
        !SIZE_MUL_NO_OVERFLOW(n, opt->node_size))
        INVALID_ARG("%zu", opt->node_size);
#endif

    memset(trie, 0, sizeof(struct PATH_TRIE_TYPE));

    PATH_TRIE_ELEM_INIT(
        &trie->elem_set,
        alloc_obj,
        opt);
#ifndef PATH_TRIE_NEED_NODE_32BIT_OFFSETS
    trie->alloc_obj = alloc_obj;
#else
#if 0
#ifndef PATH_TRIE_NEED_ELEM_32BIT_OFFSETS
    STATIC(sizeof(struct PATH_TRIE_NODE_TYPE) == 20);
#else
    STATIC(sizeof(struct PATH_TRIE_NODE_TYPE) == 16);
#endif
#endif
    trie->table.size = opt->node_size;
    trie->table.ptr = malloc(n * trie->table.size);
    ENSURE(trie->table.ptr != NULL, "malloc failed");
#endif

    trie->sep_set = opt->sep_set == NULL
        ? PATH_TRIE_DEFAULT_SEP
        : opt->sep_set;
    trie->sep_len = strlen(trie->sep_set);
}

static void PATH_TRIE_DONE(
    struct PATH_TRIE_TYPE* trie)
{
#ifdef PATH_TRIE_NEED_NODE_32BIT_OFFSETS
    free(trie->table.ptr);
#endif
    PATH_TRIE_ELEM_DONE(&trie->elem_set);
}

#ifndef PATH_TRIE_NEED_NODE_32BIT_OFFSETS

// stev: we presume that the alloc function
// return a pointer to a zeroed-out structure

#define PATH_TRIE_NEW_NODE(e)             \
    ({                                    \
        struct PATH_TRIE_NODE_TYPE* __r = \
            PATH_TRIE_ALLOC_NODE_FUNC(    \
                trie->alloc_obj);         \
        __r->elem = e;                    \
        __r;                              \
    })

#else // PATH_TRIE_NEED_NODE_32BIT_OFFSETS

#define PATH_TRIE_NEW_NODE(e)         \
    (                                 \
        PATH_TRIE_ALLOC_NODE(trie, e) \
    )

static PATH_TRIE_PTR_TYPE PATH_TRIE_ALLOC_NODE(
    struct PATH_TRIE_TYPE* trie,
    PATH_TRIE_ELEM_NODE_TYPE_CONST elem)
{
    struct PATH_TRIE_NODE_TYPE* n;
    PATH_TRIE_PTR_TYPE r;

    ENSURE(trie->table.end < trie->table.size,
        "node table overflow");

    r = trie->table.end ++;
    n = trie->table.ptr + r;

    memset(n, 0, sizeof(*n));
    n->elem = elem;

    return r + 1;
}

static size_t PATH_TRIE_GET_NODE_COUNT(
    const struct PATH_TRIE_TYPE* trie)
{
    return trie->table.end;
}

#endif // PATH_TRIE_NEED_NODE_32BIT_OFFSETS

static size_t PATH_TRIE_SPLIT(
    const struct PATH_TRIE_TYPE* trie,
    const char** key, size_t* len)
{
    const char *p = *key, *q, *s, *e;
    size_t n = *len, d = n;

    for (s = trie->sep_set,
         e = s + trie->sep_len;
         s < e;
         s ++) {
        if ((q = memchr(p, *s, d)))
            d = PTR_DIFF(q, p);
    }
    ASSERT(d <= n);

    if (d < n) {
        ASSERT_SIZE_INC_NO_OVERFLOW(d);
        d ++;

        ASSERT_SIZE_SUB_NO_OVERFLOW(n, d);
        p += d;
        n -= d;
    }
    else {
        p += n;
        n = 0;
    }

    *key = p;
    *len = n;

    return d;
}

#ifdef PATH_TRIE_NEED_GET_ELEM

static PATH_TRIE_ELEM_NODE_TYPE_CONST
    PATH_TRIE_GET_ELEM(
        const struct PATH_TRIE_TYPE* trie,
        const char** key, size_t* len)
{
    PATH_TRIE_ELEM_NODE_TYPE_CONST e;
    const char* p;
    size_t d;

    if (*len == 0)
        return PATH_TRIE_ELEM_NULL;

    p = *key;
    d = PATH_TRIE_SPLIT(trie, key, len);

    e = PATH_TRIE_ELEM_NULL;
    if (PATH_TRIE_ELEM_LOOKUP(
            &trie->elem_set, p, d, &e))
        ASSERT(e != PATH_TRIE_ELEM_NULL);
    else
        ASSERT(e == PATH_TRIE_ELEM_NULL);

    return e;
}

#endif // PATH_TRIE_NEED_GET_ELEM

static PATH_TRIE_ELEM_NODE_TYPE_CONST
    PATH_TRIE_PUT_ELEM(
        struct PATH_TRIE_TYPE* trie,
        const char** key, size_t* len)
{
    PATH_TRIE_ELEM_NODE_TYPE e;
    const char* p;
    size_t d;

    if (*len == 0)
        return PATH_TRIE_ELEM_NULL;

    p = *key;
    d = PATH_TRIE_SPLIT(trie, key, len);

    e = PATH_TRIE_ELEM_NULL;
    PATH_TRIE_ELEM_INSERT(
        &trie->elem_set, p, d, &e);

    ASSERT(e != PATH_TRIE_ELEM_NULL);
    return e;
}

#ifdef CONFIG_PATH_TRIE_UINT_HASH_IDENTOP
#define PATH_TRIE_UINT_HASH(p) (p)
#else // CONFIG_PATH_TRIE_UINT_HASH_IDENTOP
#ifndef __WORDSIZE
#error __WORDSIZE is not defined
#elif __WORDSIZE != 32 && __WORDSIZE != 64
#error __WORDSIZE is neither 32 nor 64
#endif

#define TYPE_IS_UINTN(t, n)           \
    (                                 \
        (TYPE_IS_UNSIGNED_INT(t)) &&  \
        (TYPE_UNSIGNED_INT_MAX_(t) == \
            UINT ## n ## _MAX)        \
    )
#define TYPEOF_IS_UINTN(p, n) \
    TYPE_IS_UINTN(TYPEOF(p), n)

// Thomas Wang: Integer Hash Function; 32-bit/64-bit Mix Functions
// http://web.archive.org/web/20071223173210/http://www.concentric.net/~Ttwang/tech/inthash.htm

#if __WORDSIZE == 32 || \
    defined(PATH_TRIE_NEED_ELEM_32BIT_OFFSETS)
#define PATH_TRIE_UINT_HASH(x)           \
    ({                                   \
        uint32_t __r = x;                \
        STATIC(TYPEOF_IS_UINTN(x, 32));  \
        __r = (~__r) + (__r << 15);      \
        __r ^= __r >> 12;                \
        __r += __r << 2;                 \
        __r ^= __r >> 4;                 \
        __r += (__r << 3) + (__r << 11); \
        __r ^= __r >> 16;                \
        __r;                             \
    })
#elif __WORDSIZE == 64
#define PATH_TRIE_UINT_HASH(x)          \
    ({                                  \
        uint64_t __r = x;               \
        STATIC(TYPEOF_IS_UINTN(x, 64)); \
        __r = (~__r) + (__r << 21);     \
        __r ^= __r >> 24;               \
        __r += (__r << 3) + (__r << 8); \
        __r ^= __r >> 14;               \
        __r += (__r << 2) + (__r << 4); \
        __r ^= __r >> 28;               \
        __r += __r << 31;               \
        __r;                            \
    })
#endif // __WORDSIZE
#endif // CONFIG_PATH_TRIE_UINT_HASH_IDENTOP

// stev: important note: is is required that
// PATH_TRIE_UINT_HASH be an invertible (that
// is a bijective) function mapping [2^n] to
// [2^n] := { x in N | 0 <= x < 2^n }, where
// n := SIZE_BIT := TYPE_WIDTH(size_t).

// stev: since the domain and the codomain of
// PATH_TRIE_UINT_HASH are finite sets of the
// same cardinality, the function is bijective
// if it is injective; that is that for all a
// and b in [2^n]:
//   PATH_TRIE_UINT_HASH(a) ==
//   PATH_TRIE_UINT_HASH(b) =>
//   a == b

// stev: for PTR_TO_SIZE to be a bijective function
// on *specific* sets of pointers into [2^n], it is
// sufficient that:
//   (1) CONFIG_PTR_TO_INT_IDENTOP == 1,
//   (2) sizeof(uintptr_t) == sizeof(void*),
//   (3) TYPE_WIDTH(uintptr_t) == n;
// (see PTR_TO_SIZE defined in 'ptr-traits.h');

// stev: for to assure the correctness of algorithms
// PATH_TRIE_LOOKUP and PATH_TRIE_INSERT below, it is
// sufficient that PATH_TRIE_PTR_LT be a strict total
// order on the set of pointers to structures of type
// PATH_TRIE_ELEM_NODE_TYPE. Let's name this set of
// pointers NODES. For PATH_TRIE_PTR_LT to be such
// kind of a relation on NODES it suffices that the
// composed function:
//   PATH_TRIE_UINT_HASH o PTR_TO_SIZE: NODES -> PTRS
// be injective, where:
//   PTRS := { PTR_TO_SIZE(p) | p in NODES }

// stev: why bothering that PATH_TRIE_UINT_HASH to
// not simply be the identity function? The answer
// is as follows: the shape of any ternary search
// trees is dependent upon the insertion order of
// elements in the data structure (see the SODA97
// paper of Bentley and Sedgewick mentioned above). 
// Precisely as in the case of binary search trees,
// TSTs perform radically better on the two basic
// operations -- 'lookup' and 'insert' -- when the
// input elements are inserted in random order. If
// PATH_TRIE_UINT_HASH is the identity function,
// the trees obtained are highly unbalanced. This
// is mainly due to the fact that successive calls
// to the allocator of PATH_TRIE_ELEM_NODE_TYPE
// structures, PATH_TRIE_NEW_NODE, produce pointers
// of which associated PTR_TO_SIZE integers form
// strictly increasing sequences.

#ifndef PATH_TRIE_NEED_ELEM_32BIT_OFFSETS
#define PATH_TRIE_PTR_LT(a, b)                 \
    ({                                         \
        size_t __a = PTR_TO_SIZE(a);           \
        size_t __b = PTR_TO_SIZE(b);           \
        size_t __x = PATH_TRIE_UINT_HASH(__a); \
        size_t __y = PATH_TRIE_UINT_HASH(__b); \
        STATIC(__WORDSIZE == SIZE_BIT);        \
        /* __a != __b => __x != __y */         \
        ASSERT(__a == __b || __x != __y);      \
        __x < __y;                             \
    })
#else // PATH_TRIE_NEED_ELEM_32BIT_OFFSETS
#define PATH_TRIE_PTR_LT(a, b)                 \
    ({                                         \
        uint32_t __x = PATH_TRIE_UINT_HASH(a); \
        uint32_t __y = PATH_TRIE_UINT_HASH(b); \
        /* a != b => __x != __y */             \
        ASSERT(a == b || __x != __y);          \
        __x < __y;                             \
    })
#endif // PATH_TRIE_NEED_ELEM_32BIT_OFFSETS

#ifndef PATH_TRIE_NEED_NODE_32BIT_OFFSETS
#define PATH_TRIE_NODE(n) (n)
#else // PATH_TRIE_NEED_NODE_32BIT_OFFSETS
#define PATH_TRIE_NODE_(t, p)            \
    ({                                   \
        ASSERT((p) <= (t)->table.end);   \
        (p) ? (t)->table.ptr + ((p) - 1) \
            : NULL;                      \
    })
#define PATH_TRIE_NODE(p) \
    PATH_TRIE_NODE_(trie, p)
#endif // PATH_TRIE_NEED_NODE_32BIT_OFFSETS

#ifdef PATH_TRIE_NEED_LOOKUP

static bool PATH_TRIE_LOOKUP(
    const struct PATH_TRIE_TYPE* trie,
    const char* key, size_t len,
    const struct PATH_TRIE_NODE_TYPE** result)
{
    PATH_TRIE_ELEM_NODE_TYPE_CONST elem;
    struct PATH_TRIE_NODE_TYPE* node;
    bool next = true;

    elem = PATH_TRIE_ELEM_NULL;
    node = PATH_TRIE_NODE(trie->root_node);
    while (node) {
        if (next) {
            elem = PATH_TRIE_GET_ELEM(
                trie, &key, &len);
            next = false;
        }
        if (elem == node->elem) {
            if (elem == PATH_TRIE_ELEM_NULL) {
                *result = node;
                return true;
            }
            node = PATH_TRIE_NODE(node->eq);
            next = true;
        }
        else
        if (PATH_TRIE_PTR_LT(elem, node->elem))
            node = PATH_TRIE_NODE(node->lo);
        else
            node = PATH_TRIE_NODE(node->hi);
    }

    *result = NULL;
    return false;
}

#endif // PATH_TRIE_NEED_LOOKUP

static bool PATH_TRIE_INSERT(
    struct PATH_TRIE_TYPE* trie,
    const char* key, size_t len,
    struct PATH_TRIE_NODE_TYPE** result)
{
    PATH_TRIE_ELEM_NODE_TYPE_CONST elem;
    struct PATH_TRIE_NODE_TYPE* node;
    PATH_TRIE_PTR_TYPE* ptr;
    bool next = true;

    elem = PATH_TRIE_ELEM_NULL;
    ptr = &trie->root_node;
    while ((node = PATH_TRIE_NODE(*ptr))) {
        if (next) {
            elem = PATH_TRIE_PUT_ELEM(
                trie, &key, &len);
            next = false;
        }
        if (elem == node->elem) {
#ifdef PATH_TRIE_NEED_STATISTICS
            trie->stats.insert_eq ++;
#endif
            if (elem == PATH_TRIE_ELEM_NULL) {
#ifdef PATH_TRIE_NEED_STATISTICS
                trie->stats.dups_node ++;
#endif
                *result = node;
                return false;
            }
            ptr = &node->eq;
            next = true;
        }
        else
        if (PATH_TRIE_PTR_LT(
                elem, node->elem)) {
#ifdef PATH_TRIE_NEED_STATISTICS
            trie->stats.insert_lt ++;
#endif
            ptr = &node->lo;
        }
        else {
#ifdef PATH_TRIE_NEED_STATISTICS
            trie->stats.insert_gt ++;
#endif
            ptr = &node->hi;
        }
    }

    while (true) {
        if (next)
            elem = PATH_TRIE_PUT_ELEM(
                trie, &key, &len);
        *ptr = PATH_TRIE_NEW_NODE(elem);
        node = PATH_TRIE_NODE(*ptr);

        if (elem == PATH_TRIE_ELEM_NULL)
            break;
        ptr = &node->eq;
        next = true;
    }

#ifdef PATH_TRIE_NEED_STATISTICS
    trie->stats.uniq_node ++;
#endif
    *result = node;
    return true;
}

static bool PATH_TRIE_IS_EMPTY(
    const struct PATH_TRIE_TYPE* trie)
{
    return trie->root_node == PATH_TRIE_PTR_NULL;
}

static size_t PATH_TRIE_GET_STRUCT_MEM(
    const struct PATH_TRIE_TYPE* trie)
{
    size_t r, e;

#ifndef PATH_TRIE_NEED_NODE_32BIT_OFFSETS
    r = sizeof(struct PATH_TRIE_TYPE);
#else
    r = trie->table.size;
    SIZE_SUB_EQ(r, (size_t) trie->table.end);
    SIZE_MUL_EQ(r, sizeof(struct PATH_TRIE_NODE_TYPE));
    SIZE_ADD_EQ(r, sizeof(struct PATH_TRIE_TYPE));
#endif
    e = PATH_TRIE_ELEM_GET_STRUCT_MEM(&trie->elem_set);
    SIZE_ADD_EQ(r, e);

    return r;
}

#ifdef PATH_TRIE_NEED_PRINT

#undef  STACK_NAME
#define STACK_NAME      PATH_TRIE_NODE_STACK_NAME
#undef  STACK_ELEM_TYPE
#define STACK_ELEM_TYPE PATH_TRIE_ELEM_NODE_TYPE_CONST

#include "stack-impl.h"

struct PATH_TRIE_PRINTER_TYPE
{
#if defined(PATH_TRIE_NEED_NODE_32BIT_OFFSETS) || \
    defined(PATH_TRIE_NEED_ELEM_32BIT_OFFSETS)
    const struct PATH_TRIE_TYPE*       trie;
#endif
    struct PATH_TRIE_NODE_STACK_TYPE   elems;
#ifndef PATH_TRIE_ELEM_PRINT_ONE
    struct PATH_TRIE_ELEM_PRINTER_TYPE elem_printer;
#endif
    FILE* file;
};

#define PATH_TRIE_NODE_STACK_INIT(s)   STACK_INIT(&prt->elems, s)
#define PATH_TRIE_NODE_STACK_DONE()    STACK_DONE(&prt->elems)
#define PATH_TRIE_NODE_STACK_PUSH(e)   STACK_PUSH(&prt->elems, e)
#define PATH_TRIE_NODE_STACK_POP()     STACK_POP(&prt->elems)
#define PATH_TRIE_NODE_STACK_SIZE()    STACK_SIZE(&prt->elems)
#define PATH_TRIE_NODE_STACK_BEG_REF() STACK_BEG_REF(&prt->elems)

static void PATH_TRIE_PRINTER_INIT(
    struct PATH_TRIE_PRINTER_TYPE* prt,
#if defined(PATH_TRIE_NEED_NODE_32BIT_OFFSETS) || \
    defined(PATH_TRIE_NEED_ELEM_32BIT_OFFSETS)
    const struct PATH_TRIE_TYPE* trie,
#endif
    FILE* file)
{
    memset(prt, 0, sizeof(*prt));

    PATH_TRIE_NODE_STACK_INIT(0);
#ifndef PATH_TRIE_ELEM_PRINT_ONE
    PATH_TRIE_ELEM_PRINTER_INIT(
        &prt->elem_printer, 
        file);
#endif

#if defined(PATH_TRIE_NEED_NODE_32BIT_OFFSETS) || \
    defined(PATH_TRIE_NEED_ELEM_32BIT_OFFSETS)
    prt->trie = trie;
#endif
    prt->file = file;
}

static void PATH_TRIE_PRINTER_DONE(
    struct PATH_TRIE_PRINTER_TYPE* prt)
{
#ifndef PATH_TRIE_ELEM_PRINT_ONE
    PATH_TRIE_ELEM_PRINTER_DONE(
        &prt->elem_printer);
#endif
    PATH_TRIE_NODE_STACK_DONE();
}

#ifdef PATH_TRIE_NEED_NODE_32BIT_OFFSETS
#undef  PATH_TRIE_NODE
#define PATH_TRIE_NODE(p) \
    PATH_TRIE_NODE_(prt->trie, p)
#endif

static void PATH_TRIE_PRINTER_PRINT(
    struct PATH_TRIE_PRINTER_TYPE* prt,
    const struct PATH_TRIE_NODE_TYPE* node)
{
    if (PATH_TRIE_NODE(node->lo) != NULL)
        PATH_TRIE_PRINTER_PRINT(
            prt, PATH_TRIE_NODE(node->lo));

    if (node->elem == PATH_TRIE_ELEM_NULL) {
        PATH_TRIE_ELEM_NODE_TYPE_CONST* p;
        PATH_TRIE_ELEM_NODE_TYPE_CONST* e;

        for (p = PATH_TRIE_NODE_STACK_BEG_REF(),
             e = p + PATH_TRIE_NODE_STACK_SIZE();
             p < e;
             p ++)
#ifndef PATH_TRIE_ELEM_PRINT_ONE
            PATH_TRIE_ELEM_PRINTER_PRINT_ONE(
                &prt->elem_printer, *p);
#else // PATH_TRIE_ELEM_PRINT_ONE
            PATH_TRIE_ELEM_PRINT_ONE(
#ifdef PATH_TRIE_NEED_ELEM_32BIT_OFFSETS
                &prt->trie->elem_set,
#endif
                prt->file, *p);
#endif // PATH_TRIE_ELEM_PRINT_ONE
        fputc('\n', prt->file);
    }
    else
    if (PATH_TRIE_NODE(node->eq) != NULL) {
        PATH_TRIE_NODE_STACK_PUSH(
            node->elem);

        PATH_TRIE_PRINTER_PRINT(
            prt, PATH_TRIE_NODE(node->eq));

        PATH_TRIE_NODE_STACK_POP();
    }

    if (PATH_TRIE_NODE(node->hi) != NULL)
        PATH_TRIE_PRINTER_PRINT(
            prt, PATH_TRIE_NODE(node->hi));
}

#ifdef PATH_TRIE_NEED_NODE_32BIT_OFFSETS
#undef  PATH_TRIE_NODE
#define PATH_TRIE_NODE(p) \
    PATH_TRIE_NODE_(trie, p)
#endif

static void PATH_TRIE_PRINT(
    const struct PATH_TRIE_TYPE* trie,
    FILE* file)
{
    struct PATH_TRIE_PRINTER_TYPE p;

    if (PATH_TRIE_IS_EMPTY(trie))
        return;

#if !defined(PATH_TRIE_NEED_NODE_32BIT_OFFSETS) && \
    !defined(PATH_TRIE_NEED_ELEM_32BIT_OFFSETS)
    PATH_TRIE_PRINTER_INIT(&p, file);
#else
    PATH_TRIE_PRINTER_INIT(&p, trie, file);
#endif
    PATH_TRIE_PRINTER_PRINT(&p,
        PATH_TRIE_NODE(trie->root_node));
    PATH_TRIE_PRINTER_DONE(&p);
}

#endif // PATH_TRIE_NEED_PRINT


