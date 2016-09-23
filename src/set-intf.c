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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "set-intf.h"
#include "ptr-traits.h"

enum set_intf_type_t
{
    set_intf_path_trie_type,
    set_intf_path_lhash_type,
    set_intf_path_ghash_type,
    set_intf_plain_trie_type,
    set_intf_plain_lhash_type,
    set_intf_plain_ghash_type,
};

struct set_intf_t
{
    enum set_intf_type_t type;

    union {
        struct trie_path_set_t*   path_trie;
        struct lhash_path_set_t*  path_lhash;
        struct ghash_path_set_t*  path_ghash;
        struct trie_plain_set_t*  plain_trie;
        struct lhash_plain_set_t* plain_lhash;
        struct ghash_plain_set_t* plain_ghash;
    } set;

    void (*destroy)(struct set_intf_t*);
    void (*load)(struct set_intf_t*, FILE*, bool);
    bool (*is_empty)(const struct set_intf_t*);
    void (*print_set)(const struct set_intf_t*, FILE*);
    void (*print_elems)(const struct set_intf_t*, FILE*);
    void (*print_stats)(const struct set_intf_t*, FILE*);
};

#define SET_NAME trie_path
#include "set-decl.h"

#undef  SET_NAME
#define SET_NAME lhash_path
#include "set-decl.h"

#undef  SET_NAME
#define SET_NAME ghash_path
#include "set-decl.h"

#undef  SET_NAME
#define SET_NAME trie_plain
#include "set-decl.h"

#undef  SET_NAME
#define SET_NAME lhash_plain
#include "set-decl.h"

#undef  SET_NAME
#define SET_NAME ghash_plain
#include "set-decl.h"

#undef  SET_NAME
#define SET_NAME(b, n, s)     n ## _ ## b ## _set_ ## s
#undef  SET_CREATE
#define SET_CREATE(b, n)      SET_NAME(b, n, create)
#undef  SET_DESTROY
#define SET_DESTROY(b, n)     SET_NAME(b, n, destroy)
#undef  SET_LOAD
#define SET_LOAD(b, n)        SET_NAME(b, n, load)
#undef  SET_IS_EMPTY
#define SET_IS_EMPTY(b, n)    SET_NAME(b, n, is_empty)
#undef  SET_PRINT_SET
#define SET_PRINT_SET(b, n)   SET_NAME(b, n, print_set)
#undef  SET_PRINT_ELEMS
#define SET_PRINT_ELEMS(b, n) SET_NAME(b, n, print_elems)
#undef  SET_PRINT_STATS
#define SET_PRINT_STATS(b, n) SET_NAME(b, n, print_stats)

#define SET_INTF_NAME(b, n, s) \
    set_intf_ ## b ## _ ## n ## _ ## s
#define SET_INTF_TYPE(b, n) \
    SET_INTF_NAME(b, n, type)

#define SET_INTF_OBJ_IS(b, n) \
    (                         \
        intf->type ==         \
        SET_INTF_TYPE(b, n)   \
    )

#define SET_INTF_OBJ(b, n)             \
    ({                                 \
        ASSERT(SET_INTF_OBJ_IS(b, n)); \
        intf->set.b ## _ ## n;         \
    })

#define SET_INTF_IMPL(b, n)                            \
static void SET_INTF_NAME(b, n, destroy)(              \
    struct set_intf_t* intf)                           \
{                                                      \
    SET_DESTROY(b, n)(SET_INTF_OBJ(b, n));             \
}                                                      \
                                                       \
static void SET_INTF_NAME(b, n, load)(                 \
    struct set_intf_t* intf, FILE* file, bool verbose) \
{                                                      \
    SET_LOAD(b, n)(SET_INTF_OBJ(b, n), file, verbose); \
}                                                      \
                                                       \
static bool SET_INTF_NAME(b, n, is_empty)(             \
    const struct set_intf_t* intf)                     \
{                                                      \
    return SET_IS_EMPTY(b, n)(SET_INTF_OBJ(b, n));     \
}                                                      \
                                                       \
static void SET_INTF_NAME(b, n, print_set)(            \
    const struct set_intf_t* intf, FILE* file)         \
{                                                      \
    SET_PRINT_SET(b, n)(SET_INTF_OBJ(b, n), file);     \
}                                                      \
                                                       \
static void SET_INTF_NAME(b, n, print_elems)(          \
    const struct set_intf_t* intf, FILE* file)         \
{                                                      \
    SET_PRINT_ELEMS(b, n)(SET_INTF_OBJ(b, n), file);   \
}                                                      \
                                                       \
static void SET_INTF_NAME(b, n, print_stats)(          \
    const struct set_intf_t* intf, FILE* file)         \
{                                                      \
    SET_PRINT_STATS(b, n)(SET_INTF_OBJ(b, n), false,   \
        file);                                         \
}

SET_INTF_IMPL(path, trie)
SET_INTF_IMPL(path, lhash)
SET_INTF_IMPL(path, ghash)

SET_INTF_IMPL(plain, trie)
SET_INTF_IMPL(plain, lhash)
SET_INTF_IMPL(plain, ghash)

#define SET_INTF_INIT_OBJ(b, n, o)                            \
    do {                                                      \
        intf->type = SET_INTF_TYPE(b, n);                     \
        intf->set.b ## _ ## n = SET_CREATE(b, n)(o);          \
        intf->destroy = SET_INTF_NAME(b, n, destroy);         \
        intf->load = SET_INTF_NAME(b, n, load);               \
        intf->is_empty = SET_INTF_NAME(b, n, is_empty);       \
        intf->print_set = SET_INTF_NAME(b, n, print_set);     \
        intf->print_elems = SET_INTF_NAME(b, n, print_elems); \
        intf->print_stats = SET_INTF_NAME(b, n, print_stats); \
    } while (0)

struct set_intf_t* set_intf_create(
    const struct options_t* opt)
{
    struct set_intf_t* intf;

    intf = malloc(sizeof(struct set_intf_t));
    ENSURE(intf != NULL, "malloc failed");

    memset(intf, 0, sizeof(struct set_intf_t));

    if (opt->struct_type == options_plain_struct_type) {
        switch (opt->set_type) {

        case options_ternary_tree_set_type:
            SET_INTF_INIT_OBJ(plain, trie, opt);
            break;
        case options_linear_hash_set_type:
            SET_INTF_INIT_OBJ(plain, lhash, opt);
            break;
        case options_gnulib_hash_set_type:
            SET_INTF_INIT_OBJ(plain, ghash, opt);
            break;

        default:
            UNEXPECT_VAR("%d", opt->set_type);
        }
    }
    else
    if (opt->struct_type == options_path_struct_type) {
        switch (opt->set_type) {

        case options_ternary_tree_set_type:
            SET_INTF_INIT_OBJ(path, trie, opt);
            break;
        case options_linear_hash_set_type:
            SET_INTF_INIT_OBJ(path, lhash, opt);
            break;
        case options_gnulib_hash_set_type:
            SET_INTF_INIT_OBJ(path, ghash, opt);
            break;

        default:
            UNEXPECT_VAR("%d", opt->set_type);
        }
    }
    else
        UNEXPECT_VAR("%d", opt->struct_type);

    return intf;
}

void set_intf_destroy(
    struct set_intf_t* intf)
{
    intf->destroy(intf);
    free(intf);
}

void set_intf_load(
    struct set_intf_t* intf, FILE* file,
    bool verbose)
{ intf->load(intf, file, verbose); }

bool set_intf_is_empty(
    const struct set_intf_t* intf)
{ return intf->is_empty(intf); }

void set_intf_print_set(
    const struct set_intf_t* intf,
    FILE* file)
{ intf->print_set(intf, file); }

void set_intf_print_elems(
    const struct set_intf_t* intf,
    FILE* file)
{ intf->print_elems(intf, file); }

void set_intf_print_stats(
    const struct set_intf_t* intf,
    FILE* file)
{ intf->print_stats(intf, file); }

void print_sizes(
    const struct options_t* opt, FILE* file)
{
#undef  CASE
#define CASE(s, f) \
    [options_ ## s ## _set_type] = f ## _plain_set_print_sizes
    static void (*const plain_funcs[3])(FILE*) = {
        CASE(ternary_tree, trie),
        CASE(linear_hash, lhash),
        CASE(gnulib_hash, ghash),
    };
#undef  CASE
#define CASE(s, f) \
    [options_ ## s ## _set_type] = f ## _path_set_print_sizes
    static void (*const path_funcs[3])(FILE*) = {
        CASE(ternary_tree, trie),
        CASE(linear_hash, lhash),
        CASE(gnulib_hash, ghash),
    };
    void (*f)(FILE*);

    if (opt->struct_type == options_plain_struct_type)
        f = ARRAY_NULL_ELEM(plain_funcs, opt->set_type);
    else
    if (opt->struct_type == options_path_struct_type)
        f = ARRAY_NULL_ELEM(path_funcs, opt->set_type);
    else
        UNEXPECT_VAR("%d", opt->struct_type);

    ASSERT(f != NULL);
    f(file);
}

void print_names(
    const struct options_t* opt, FILE* file)
{
    if (opt->struct_type == options_plain_struct_type) {
        switch (opt->set_type) {

        case options_ternary_tree_set_type:
            trie_plain_set_print_stats(NULL, true, file);
            break;
        case options_linear_hash_set_type:
            lhash_plain_set_print_stats(NULL, true, file);
            break;
        case options_gnulib_hash_set_type:
            ghash_plain_set_print_stats(NULL, true, file);
            break;

        default:
            UNEXPECT_VAR("%d", opt->set_type);
        }
    }
    else
    if (opt->struct_type == options_path_struct_type) {
        switch (opt->set_type) {

        case options_ternary_tree_set_type:
            trie_path_set_print_stats(NULL, true, file);
            break;
        case options_linear_hash_set_type:
            lhash_path_set_print_stats(NULL, true, file);
            break;
        case options_gnulib_hash_set_type:
            ghash_path_set_print_stats(NULL, true, file);
            break;

        default:
            UNEXPECT_VAR("%d", opt->set_type);
        }
    }
    else
        UNEXPECT_VAR("%d", opt->struct_type);
}


