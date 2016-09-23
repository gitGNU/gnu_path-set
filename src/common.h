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

#ifndef __COMMON_H
#define __COMMON_H

#include <stdio.h>
#include <stdbool.h>

#include "int-traits.h"

#ifndef __GNUC__
#error we need a GCC compiler
#endif

#define UNUSED    __attribute__((unused))
#define PRINTF(F) __attribute__((format(printf, F, F + 1)))
#define NORETURN  __attribute__((noreturn))
#define PACKED    __attribute__((packed))

#define STRINGIFY_(s) #s
#define STRINGIFY(s)  STRINGIFY_(s)

#ifndef CONFIG_VALUE_TYPE_SIZE
#ifndef __WORDSIZE
#error __WORDSIZE is not defined
#else // __WORDSIZE
#define CONFIG_VALUE_TYPE_SIZE __WORDSIZE
#endif // __WORDSIZE
#endif // CONFIG_VALUE_TYPE_SIZE

#define SET_VALUE_TYPE__(n) \
    uint ## n ## _t
#define SET_VALUE_TYPE_(n) \
    SET_VALUE_TYPE__(n)
#define SET_VALUE_TYPE \
    SET_VALUE_TYPE_(CONFIG_VALUE_TYPE_SIZE)

// stev: important requirement: VERIFY evaluates E only once!

#define VERIFY(E)             \
    do {                      \
        if (!(E))             \
            UNEXPECT_ERR(#E); \
    }                         \
    while (0)

// stev: important requirement: ENSURE evaluates E only once!

#define ENSURE(E, M, ...)                     \
    do {                                      \
        if (!(E))                             \
            ensure_failed(__FILE__, __LINE__, \
                __func__, M, ## __VA_ARGS__); \
    }                                         \
    while (0)

#define UNEXPECT_ERR(M, ...)               \
    do {                                   \
        unexpect_error(__FILE__, __LINE__, \
            __func__, M, ## __VA_ARGS__);  \
    }                                      \
    while (0)

#define UNEXPECT_VAR(F, N) UNEXPECT_ERR(#N "=" F, N)

#define INVALID_ARG(F, N)                    \
    do {                                     \
        invalid_argument(__FILE__, __LINE__, \
            __func__, #N, F, N);             \
    }                                        \
    while (0)

#define NOT_YET_IMPL()                   \
    do {                                 \
        not_yet_impl(__FILE__, __LINE__, \
            __func__);                   \
    }                                    \
    while (0)

#ifdef DEBUG
# define ASSERT(E)                            \
    do {                                      \
        if (!(E))                             \
            assert_failed(__FILE__, __LINE__, \
                __func__, #E);                \
    }                                         \
    while (0)
#else
# define ASSERT(E) \
    do {} while (0)
#endif

#define STATIC(E)                                   \
    ({                                              \
        extern int __attribute__                    \
            ((error("assertion failed: '" #E "'"))) \
            static_assert();                        \
        (void) ((E) ? 0 : static_assert());         \
    })

void fatal_error(const char* fmt, ...)
    PRINTF(1)
    NORETURN;

void ensure_failed(
    const char* file, int line, const char* func,
    const char* msg, ...)
    PRINTF(4)
    NORETURN;

void assert_failed(
    const char* file, int line, const char* func,
    const char* expr)
    NORETURN;

void unexpect_error(
    const char* file, int line, const char* func,
    const char* msg, ...)
    PRINTF(4)
    NORETURN;

void invalid_argument(
    const char* file, int line, const char* func,
    const char* name, const char* msg, ...)
    PRINTF(5)
    NORETURN;

void not_yet_impl(
    const char* file, int line, const char* func)
    NORETURN;

void error(const char* fmt, ...)
    PRINTF(1)
    NORETURN;

void warning(const char* fmt, ...)
    PRINTF(1);

enum options_action_t
{
    options_print_config_action,
    options_print_sizes_action,
    options_print_names_action,
    options_load_only_action,
    options_print_set_action,
    options_print_elems_action
};

enum options_struct_type_t
{
    options_plain_struct_type,
    options_path_struct_type
};

enum options_set_type_t
{
    options_ternary_tree_set_type,
    options_linear_hash_set_type,
    options_gnulib_hash_set_type,
};

struct options_t
{
    enum options_action_t      action;
    enum options_struct_type_t struct_type;
    enum options_set_type_t    set_type;
    size_t                     pool_size;
    size_t                     hash_size;
#ifdef CONFIG_PATH_TRIE_NODE_32BIT_OFFSETS
    size_t                     node_size;
#endif
    const char*                sep_set;

    bits_t                     print_stats: 1;
    bits_t                     verbose: 1;

    size_t                     argc;
    char* const               *argv;
};

const struct options_t* options(
    int argc, char* argv[]);

extern const char path_trie_default_sep[];

struct size_frac_t
{
    size_t num;
    size_t denom;
    size_t prec;
};

bool compute_size_frac(
    size_t n, size_t m, struct size_frac_t* r);

enum stat_param_type_t
{
    stat_param_size_field_type,
    stat_param_time_field_type,
    stat_param_size_func_type,
    stat_param_frac_func_type
};

enum stat_param_name_t
{
#undef  CASE
#define CASE(type, id, name, help) stat_param_ ## id,
#include "set-stats.def"
};

typedef size_t
    (*stat_param_size_func_t)(const void*);
typedef struct size_frac_t
    (*stat_param_frac_func_t)(const void*);

struct stat_param_info_t
{
    enum stat_param_name_t     name;
    enum stat_param_type_t     type;
    union {
        size_t                 size_field;
        size_t                 time_field;
        stat_param_size_func_t size_func;
        stat_param_frac_func_t frac_func;
    } val;
};

void stat_params_print(
    const struct stat_param_info_t* params,
    size_t len, const void* obj,
    bool only_names, FILE* file);

void print_config(FILE* file);

#endif/*__COMMON_H*/

