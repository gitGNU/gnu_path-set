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

#if defined(TRIE_VAL_TYPE) && \
    defined(TRIE_NEED_PRINT_ONE)
#error cannot have both \
    TRIE_VAL_TYPE and \
    TRIE_NEED_PRINT_ONE defined
#endif

#ifndef TRIE_ALLOC_NODE_FUNC
#error  TRIE_ALLOC_NODE_FUNC is not defined
#endif

#ifndef TRIE_ALLOC_OBJ_TYPE
#error  TRIE_ALLOC_OBJ_TYPE is not defined
#endif

#ifdef  TRIE_NAME
#define TRIE_MAKE_NAME__(n, s) n ## _trie ## s
#define TRIE_MAKE_NAME_(n, s)  TRIE_MAKE_NAME__(n, s)
#define TRIE_MAKE_NAME(s)      TRIE_MAKE_NAME_(TRIE_NAME, _ ## s)
#else
#define TRIE_MAKE_NAME_(s)  trie ## s
#define TRIE_MAKE_NAME(s)   TRIE_MAKE_NAME_(_ ## s)
#endif

#define TRIE_TYPE           TRIE_MAKE_NAME(t)
#define TRIE_NODE_TYPE      TRIE_MAKE_NAME(node_t)
#define TRIE_UCHAR_TYPE     TRIE_MAKE_NAME(uchar_t)

#define TRIE_SET_STATS_TYPE TRIE_MAKE_NAME(set_stats_t)

#define TRIE_INIT           TRIE_MAKE_NAME(init)
#define TRIE_DONE           TRIE_MAKE_NAME(done)

#define TRIE_LOOKUP         TRIE_MAKE_NAME(lookup)
#define TRIE_INSERT         TRIE_MAKE_NAME(insert)
#define TRIE_IS_EMPTY       TRIE_MAKE_NAME(is_empty)
#define TRIE_PRINT          TRIE_MAKE_NAME(print)

#define TRIE_STACK_NAME     TRIE_MAKE_NAME(node)
#define TRIE_STACK_TYPE     TRIE_MAKE_NAME(node_stack_t)

#define TRIE_PRINTER_TYPE   TRIE_MAKE_NAME(printer_t)
#define TRIE_PRINTER_INIT   TRIE_MAKE_NAME(printer_init)
#define TRIE_PRINTER_DONE   TRIE_MAKE_NAME(printer_done)
#define TRIE_PRINTER_PRINT_ONE \
                            TRIE_MAKE_NAME(printer_print_one)
#define TRIE_PRINTER_PRINT  TRIE_MAKE_NAME(printer_print)

#ifdef TRIE_NEED_STATISTICS

#ifdef TRIE_NAME
#define SET_STATS_NAME TRIE_MAKE_NAME_(TRIE_NAME, )
#else
#define SET_STATS_NAME trie
#endif // TRIE_NAME

#undef  SET_STATS_NEED_CLOCKS
#define SET_STATS_NEED_NODE_SIZES
#undef  SET_STATS_NEED_LINE_SIZES
#include "set-stats-impl.h"

#endif // TRIE_NEED_STATISTICS

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

#ifndef TRIE_UCHAR_TYPEDEF
#define TRIE_UCHAR_TYPEDEF
typedef unsigned char TRIE_UCHAR_TYPE;
#endif

#ifdef TRIE_NEED_PRINT_ONE
#define TRIE_VAL_TYPE struct TRIE_NODE_TYPE*
#endif

struct TRIE_NODE_TYPE
{
    TRIE_UCHAR_TYPE ch;

#ifdef TRIE_NEED_PRINT_ONE
    struct TRIE_NODE_TYPE* up;
#endif
    struct TRIE_NODE_TYPE* lo;
    struct TRIE_NODE_TYPE* hi;

    // stev: note that 'ch' is the discriminator
    // attached to the union below by the conditions:
    //   ch != 0 => use 'eq'
    //   ch == 0 => use 'val'
    union {
        struct TRIE_NODE_TYPE* eq;
#ifdef TRIE_VAL_TYPE
        TRIE_VAL_TYPE          val;
#endif
    };
};

#ifdef TRIE_NEED_STATISTICS
SET_STATS_STRUCT_DECL(insert_lt, insert_gt)
#endif

struct TRIE_TYPE
{
    TRIE_ALLOC_OBJ_TYPE*   alloc_obj;
    struct TRIE_NODE_TYPE* root_node;

#ifdef TRIE_NEED_STATISTICS
    struct TRIE_SET_STATS_TYPE stats;
#endif
};

static void TRIE_INIT(
    struct TRIE_TYPE* trie,
    TRIE_ALLOC_OBJ_TYPE* alloc_obj,
    const struct options_t* opt UNUSED)
{
    memset(trie, 0, sizeof(struct TRIE_TYPE));

    trie->alloc_obj = alloc_obj;
}

static void TRIE_DONE(
    struct TRIE_TYPE* trie UNUSED)
{
    // stev: nop
}

// stev: we presume that the alloc function
// return a pointer to a zeroed-out structure

#define TRIE_NEW_NODE_(c, n)         \
    ({                               \
        struct TRIE_NODE_TYPE* __r = \
            TRIE_ALLOC_NODE_FUNC(    \
                trie->alloc_obj);    \
        __r->ch = c;                 \
        n;                           \
        __r;                         \
    })

#ifdef TRIE_NEED_PRINT_ONE
#define TRIE_NEW_NODE(c, n) \
    TRIE_NEW_NODE_(c, __r->up = n)
#else
#define TRIE_NEW_NODE(c) \
    TRIE_NEW_NODE_(c, )
#endif

#ifdef TRIE_NEED_LOOKUP

static bool TRIE_LOOKUP(
    const struct TRIE_TYPE* trie,
    const char* key, size_t len,
    const struct TRIE_NODE_TYPE** result)
{
    const struct TRIE_NODE_TYPE* node;
    TRIE_UCHAR_TYPE ch;

    node = trie->root_node;
    while (node) {
        ASSERT(!len || *key);
        ch = len > 0 ? *key : 0;
        // ch == 0 <=> len == 0

        if (ch == node->ch) {
            if (ch == 0) {
                *result = node;
                return true;
            }
            node = node->eq;
            key ++;
            len --;
        }
        else
        if (ch < node->ch)
            node = node->lo;
        else
            node = node->hi;
    }

    *result = NULL;
    return false;
}

#endif // TRIE_NEED_LOOKUP

static bool TRIE_INSERT(
    struct TRIE_TYPE* trie,
    const char* key, size_t len,
    struct TRIE_NODE_TYPE** result)
{
    struct TRIE_NODE_TYPE* node;
#ifdef TRIE_NEED_PRINT_ONE
    struct TRIE_NODE_TYPE *base, *up;
#endif
    struct TRIE_NODE_TYPE** ptr;
    TRIE_UCHAR_TYPE ch;

#ifdef TRIE_NEED_PRINT_ONE
    up = base = NULL;
#endif
    ptr = &trie->root_node;
    while ((node = *ptr)) {
        ASSERT(!len || *key);
        ch = len > 0 ? *key : 0;
        // ch == 0 <=> len == 0
        if (ch == node->ch) {
#ifdef TRIE_NEED_PRINT_ONE
            if (base == NULL)
                base = node;
#endif
#ifdef TRIE_NEED_STATISTICS
            trie->stats.insert_eq ++;
#endif
            if (ch == 0) {
#ifdef TRIE_NEED_STATISTICS
                trie->stats.dups_node ++;
#endif
                *result = node;
                return false;
            }
            ptr = &node->eq;
            key ++;
            len --;
        }
        else
        if (ch < node->ch) {
#ifdef TRIE_NEED_STATISTICS
            trie->stats.insert_lt ++;
#endif
            ptr = &node->lo;
        }
        else {
#ifdef TRIE_NEED_STATISTICS
            trie->stats.insert_gt ++;
#endif
            ptr = &node->hi;
        }
#ifdef TRIE_NEED_PRINT_ONE
        up = node;
#endif
    }

    while (true) {
        ASSERT(!len || *key);
        ch = len > 0 ? *key : 0;
        // ch == 0 <=> len == 0
        node = *ptr =
#ifndef TRIE_NEED_PRINT_ONE
            TRIE_NEW_NODE(ch);
#else
            TRIE_NEW_NODE(ch, up);
        if (base == NULL)
            base = node;
#endif
        if (ch == 0) {
#ifdef TRIE_NEED_PRINT_ONE
            node->val = base;
#endif
            break;
        }

        ptr = &node->eq;
#ifdef TRIE_NEED_PRINT_ONE
        up = node;
#endif
        key ++;
        len --;
    }

#ifdef TRIE_NEED_STATISTICS
    trie->stats.uniq_node ++;
#endif
    *result = node;
    return true;
}

#ifdef TRIE_NEED_IS_EMPTY

static bool TRIE_IS_EMPTY(
    const struct TRIE_TYPE* trie)
{
    return trie->root_node == NULL;
}

#endif // TRIE_NEED_IS_EMPTY

#if defined(TRIE_NEED_PRINT_ONE) || \
    defined(TRIE_NEED_PRINT)

#undef  STACK_NAME
#define STACK_NAME      TRIE_STACK_NAME
#undef  STACK_ELEM_TYPE
#define STACK_ELEM_TYPE char

#include "stack-impl.h"

struct TRIE_PRINTER_TYPE
{
    struct TRIE_STACK_TYPE str;
    FILE* file;
};

#define TRIE_STACK_INIT(s)    STACK_INIT(&prt->str, s)
#define TRIE_STACK_DONE()     STACK_DONE(&prt->str)
#define TRIE_STACK_CLEAR()    STACK_CLEAR(&prt->str)
#define TRIE_STACK_PUSH(c)    STACK_PUSH(&prt->str, c)
#define TRIE_STACK_POP()      STACK_POP(&prt->str)
#define TRIE_STACK_SIZE()     STACK_SIZE(&prt->str)
#define TRIE_STACK_IS_EMPTY() STACK_IS_EMPTY(&prt->str)
#define TRIE_STACK_BEG_REF()  STACK_BEG_REF(&prt->str)

static void TRIE_PRINTER_INIT(
    struct TRIE_PRINTER_TYPE* prt,
    FILE* file)
{
    memset(prt, 0, sizeof(*prt));

    TRIE_STACK_INIT(0);

    prt->file = file;
}

static void TRIE_PRINTER_DONE(
    struct TRIE_PRINTER_TYPE* prt)
{
    TRIE_STACK_DONE();
}

#endif // defined(TRIE_NEED_PRINT_ONE) ||
       // defined(TRIE_NEED_PRINT)

#ifdef TRIE_NEED_PRINT_ONE

static void TRIE_PRINTER_PRINT_ONE(
    struct TRIE_PRINTER_TYPE* prt,
    const struct TRIE_NODE_TYPE* node)
{
    const struct TRIE_NODE_TYPE *q, *t;
    const char *b, *p;

    ASSERT(node != NULL);
    ASSERT(node->ch == 0);
    ASSERT(node->val != NULL);

    TRIE_STACK_CLEAR();

    q = node;
    do {
        t = q;
        q = q->up;
        ASSERT(q != NULL);
        if (q->ch && q->eq == t)
            TRIE_STACK_PUSH(q->ch);
    } while (q != node->val);

    b = TRIE_STACK_BEG_REF();
    p = b + TRIE_STACK_SIZE();
    while (p > b)
         fputc(*-- p, prt->file);
}

#endif // TRIE_NEED_PRINT_ONE

#ifdef TRIE_NEED_PRINT

static void TRIE_PRINTER_PRINT(
    struct TRIE_PRINTER_TYPE* prt,
    const struct TRIE_NODE_TYPE* node)
{
    if (node->lo != NULL)
        TRIE_PRINTER_PRINT(prt, node->lo);

    if (node->ch == 0) {
        size_t n = TRIE_STACK_SIZE();

        fprintf(prt->file, "%.*s",
            SIZE_AS_INT(n),
            n ? TRIE_STACK_BEG_REF() : "");
        fputc('\n', prt->file);
    }
    else
    if (node->eq != NULL) {
        TRIE_STACK_PUSH((char) node->ch);

        TRIE_PRINTER_PRINT(prt, node->eq);

        TRIE_STACK_POP();
    }

    if (node->hi != NULL)
        TRIE_PRINTER_PRINT(prt, node->hi);
}

static void TRIE_PRINT(
    const struct TRIE_TYPE* trie,
    FILE* file)
{
    struct TRIE_PRINTER_TYPE p;

    if (trie->root_node == NULL)
        return;

    TRIE_PRINTER_INIT(&p, file);
    TRIE_PRINTER_PRINT(&p, trie->root_node);
    TRIE_PRINTER_DONE(&p);
}

#endif // TRIE_NEED_PRINT


