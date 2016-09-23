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

struct plain_lhash_node_t;
struct lhash_plain_set_t;

static struct plain_lhash_node_t* lhash_plain_set_new_node(
    struct lhash_plain_set_t* set, size_t len);

#define LHASH_NAME            plain
#define LHASH_VAL_TYPE        SET_VALUE_TYPE
#define LHASH_ALLOC_NODE_FUNC lhash_plain_set_new_node
#define LHASH_ALLOC_OBJ_TYPE  struct lhash_plain_set_t

#define LHASH_NEED_NULL_TERM_KEY
#define LHASH_NEED_STATISTICS
#define LHASH_NEED_IS_EMPTY
#define LHASH_NEED_PRINT
#include "lhash-impl.h"

#undef  SET_STATS_NAME
#define SET_STATS_NEED_CLOCKS
#undef  SET_STATS_NEED_NODE_SIZES
#define SET_STATS_NEED_LINE_SIZES
#include "set-stats-impl.h"

SET_STATS_STRUCT_DECL(
    node_struct,
    node_mem)

#define SET_NAME lhash_plain
#include "set-decl.h"

#define SET_IMPL_NAME plain_lhash
#define SET_NEED_PRINT_SET
#include "set-impl.h"

static struct plain_lhash_node_t*
    lhash_plain_set_new_node(
        struct lhash_plain_set_t* set,
        size_t len)
{
    return SET_ALLOCATE_VAR_STRUCT(
        node, plain_lhash_node_t,
        len);
}

void lhash_plain_set_print_elems(
    const struct lhash_plain_set_t* set,
    FILE* file)
{
    lhash_plain_set_print_set(set, file);
}

static struct size_frac_t
    lhash_plain_set_get_param_hash_load(
        const struct lhash_plain_set_t* set)
{
    struct size_frac_t r;

    if (!compute_size_frac(
            set->impl.used,
            set->impl.size,
            &r))
        VERIFY(false);

    return r;
}

static size_t lhash_plain_set_get_param_node_mem(
    const struct lhash_plain_set_t* set)
{
    size_t n;

    n = SIZE_MUL(
        set->stats.node_struct,
        sizeof(struct plain_lhash_node_t));

    return SIZE_ADD(n, set->stats.node_mem);
}

static size_t lhash_plain_set_get_param_total_node_mem(
    const struct lhash_plain_set_t* set)
{
    return lhash_plain_set_get_param_node_mem(set);
}

#define FSIZE SET_STAT_PARAM_SIZE_FIELD
#define FTIME SET_STAT_PARAM_TIME_FIELD
#define SFUNC SET_STAT_PARAM_SIZE_FUNC
#define FFUNC SET_STAT_PARAM_FRAC_FUNC

void lhash_plain_set_print_stats(
    const struct lhash_plain_set_t* set, bool only_names, FILE* file)
{
    static const struct stat_param_info_t params[] = {
        FSIZE(dups_line,        stats.dups_line),
        FSIZE(uniq_line,        stats.uniq_line),
        FSIZE(line_mem,         stats.line_mem),
        FSIZE(dups_node,        impl.stats.dups_node),
        FSIZE(uniq_node,        impl.stats.uniq_node),
        FSIZE(node_insert_eq,   impl.stats.insert_eq),
        FSIZE(node_insert_ne,   impl.stats.insert_ne),
        FSIZE(hash_size,        impl.size),
        FSIZE(hash_used,        impl.used),
        FFUNC(hash_load,        lhash_plain_set_get_param_hash_load),
        SFUNC(node_struct_size, lhash_plain_set_get_node_size),
        FSIZE(node_struct,      stats.node_struct),
        SFUNC(node_mem,         lhash_plain_set_get_param_node_mem),
        SFUNC(total_node_mem,   lhash_plain_set_get_param_total_node_mem),
        FTIME(insert_time,      stats.insert_time),
        //FTIME(lookup_time,      stats.lookup_time),
    };

    stat_params_print(
        params, ARRAY_SIZE(params), set, only_names, file);
}

void lhash_plain_set_print_sizes(FILE* file)
{
    static const struct stat_param_info_t params[] = {
        SFUNC(node_struct_size, lhash_plain_set_get_node_size),
    };

    stat_params_print(
        params, ARRAY_SIZE(params), NULL, false, file);
}


