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

#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "clocks.h"
#include "ptr-traits.h"
#include "pool-alloc.h"

struct path_trie_node_t;
struct lhash_node_t;

struct lhash_path_set_t;

#ifndef  CONFIG_PATH_TRIE_NODE_32BIT_OFFSETS
static struct path_trie_node_t* lhash_path_set_new_node(
    struct lhash_path_set_t* set);
#endif
static struct lhash_node_t* lhash_path_set_new_elem(
    struct lhash_path_set_t* set, size_t len);

#undef  LHASH_NAME
#define LHASH_ALLOC_NODE_FUNC lhash_path_set_new_elem
#define LHASH_ALLOC_OBJ_TYPE  struct lhash_path_set_t

#ifdef  CONFIG_PATH_TRIE_ELEM_32BIT_OFFSETS
#define LHASH_NEED_32BIT_OFFSETS
#endif
#undef  LHASH_NEED_NULL_TERM_KEY
#define LHASH_NEED_STATISTICS
#define LHASH_NEED_PRINT
#include "lhash-impl.h"

#undef  PATH_TRIE_NAME
#define PATH_TRIE_VAL_TYPE        SET_VALUE_TYPE
#define PATH_TRIE_ELEM_NAME       lhash
#define PATH_TRIE_ALLOC_NODE_FUNC lhash_path_set_new_node
#define PATH_TRIE_ALLOC_OBJ_TYPE  struct lhash_path_set_t
#define PATH_TRIE_DEFAULT_SEP     path_trie_default_sep
#define PATH_TRIE_ELEM_PRINT_ONE  LHASH_PRINT_ONE

#ifdef  CONFIG_PATH_TRIE_NODE_32BIT_OFFSETS
#define PATH_TRIE_NEED_NODE_32BIT_OFFSETS
#endif
#ifdef  CONFIG_PATH_TRIE_ELEM_32BIT_OFFSETS
#define PATH_TRIE_NEED_ELEM_32BIT_OFFSETS
#endif
#define PATH_TRIE_NEED_ELEM_OBJ_ALLOC
#define PATH_TRIE_NEED_STATISTICS
#define PATH_TRIE_NEED_PRINT
#include "path-trie-impl.h"

#undef  SET_STATS_NAME
#define SET_STATS_NEED_CLOCKS
#undef  SET_STATS_NEED_NODE_SIZES
#define SET_STATS_NEED_LINE_SIZES
#include "set-stats-impl.h"

SET_STATS_STRUCT_DECL(
#ifndef CONFIG_PATH_TRIE_NODE_32BIT_OFFSETS
    node_struct,
#endif
    elem_struct,
    elem_mem)

#define SET_NAME lhash_path
#include "set-decl.h"

#define SET_IMPL_NAME path_trie
#define SET_PATH_TRIE_ELEM_NODE_TYPE lhash_node_t
#define SET_NEED_POOL_ALLOC
#define SET_NEED_PRINT_SET
#include "set-impl.h"

#ifndef  CONFIG_PATH_TRIE_NODE_32BIT_OFFSETS
static struct path_trie_node_t*
    lhash_path_set_new_node(
        struct lhash_path_set_t* set)
{
    return SET_ALLOCATE_STRUCT(
        node, path_trie_node_t);
}
#endif

static struct lhash_node_t*
    lhash_path_set_new_elem(
        struct lhash_path_set_t* set,
        size_t len)
{
    return SET_ALLOCATE_VAR_STRUCT(
        elem, lhash_node_t,
        len);
}

void lhash_path_set_print_elems(
    const struct lhash_path_set_t* set,
    FILE* file)
{
    lhash_print(&set->impl.elem_set, file);
}

static struct size_frac_t
    lhash_path_set_get_param_hash_load(
        const struct lhash_path_set_t* set)
{
    struct size_frac_t r;

    if (!compute_size_frac(
            set->impl.elem_set.used,
            set->impl.elem_set.size,
            &r))
        VERIFY(false);

    return r;
}

static size_t lhash_path_set_get_param_node_struct(
    const struct lhash_path_set_t* set)
{
#ifndef CONFIG_PATH_TRIE_NODE_32BIT_OFFSETS
    return set->stats.node_struct;
#else
    return path_trie_get_node_count(&set->impl);
#endif
}

static size_t lhash_path_set_get_param_node_mem(
    const struct lhash_path_set_t* set)
{
    size_t n;

    n = lhash_path_set_get_param_node_struct(set);
    return SIZE_MUL(n, sizeof(struct path_trie_node_t));
}

static size_t lhash_path_set_get_param_elem_mem(
    const struct lhash_path_set_t* set)
{
    size_t n;

    n = SIZE_MUL(
        set->stats.elem_struct,
        sizeof(struct lhash_node_t));

    return SIZE_ADD(n, set->stats.elem_mem);
}

static size_t lhash_path_set_get_param_total_node_mem(
    const struct lhash_path_set_t* set)
{
    size_t n, e;

    n = lhash_path_set_get_param_node_mem(set);
    e = lhash_path_set_get_param_elem_mem(set);

    return SIZE_ADD(n, e);
}

static size_t lhash_path_set_get_param_total_mem(
    const struct lhash_path_set_t* set)
{
    size_t n, s;

    n = lhash_path_set_get_param_total_node_mem(set);
    s = path_trie_get_struct_mem(&set->impl);

    return SIZE_ADD(n, s);
}

#define FSIZE SET_STAT_PARAM_SIZE_FIELD
#define FTIME SET_STAT_PARAM_TIME_FIELD
#define SFUNC SET_STAT_PARAM_SIZE_FUNC
#define FFUNC SET_STAT_PARAM_FRAC_FUNC

void lhash_path_set_print_stats(
    const struct lhash_path_set_t* set, bool only_names, FILE* file)
{
    static const struct stat_param_info_t params[] = {
        FSIZE(dups_line,        stats.dups_line),
        FSIZE(uniq_line,        stats.uniq_line),
        FSIZE(line_mem,         stats.line_mem),
        FSIZE(dups_node,        impl.stats.dups_node),
        FSIZE(uniq_node,        impl.stats.uniq_node),
        FSIZE(dups_elem,        impl.elem_set.stats.dups_node),
        FSIZE(uniq_elem,        impl.elem_set.stats.uniq_node),
        FSIZE(node_insert_eq,   impl.stats.insert_eq),
        FSIZE(node_insert_lt,   impl.stats.insert_lt),
        FSIZE(node_insert_gt,   impl.stats.insert_gt),
        FSIZE(elem_insert_eq,   impl.elem_set.stats.insert_eq),
        FSIZE(elem_insert_ne,   impl.elem_set.stats.insert_ne),
        FSIZE(hash_size,        impl.elem_set.size),
        FSIZE(hash_used,        impl.elem_set.used),
        FFUNC(hash_load,        lhash_path_set_get_param_hash_load),
        SFUNC(node_struct_size, lhash_path_set_get_node_size),
        SFUNC(elem_struct_size, lhash_path_set_get_elem_size),
        SFUNC(node_struct,      lhash_path_set_get_param_node_struct),
        FSIZE(elem_struct,      stats.elem_struct),
        SFUNC(node_mem,         lhash_path_set_get_param_node_mem),
        SFUNC(elem_mem,         lhash_path_set_get_param_elem_mem),
        SFUNC(total_node_mem,   lhash_path_set_get_param_total_node_mem),
        SFUNC(total_mem,        lhash_path_set_get_param_total_mem),
        FTIME(insert_time,      stats.insert_time),
        //FTIME(lookup_time,      stats.lookup_time),
    };

    stat_params_print(
        params, ARRAY_SIZE(params), set, only_names, file);
}

void lhash_path_set_print_sizes(FILE* file)
{
    static const struct stat_param_info_t params[] = {
        SFUNC(node_struct_size, lhash_path_set_get_node_size),
        SFUNC(elem_struct_size, lhash_path_set_get_elem_size),
    };

    stat_params_print(
        params, ARRAY_SIZE(params), NULL, false, file);
}


