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

#define _GNU_SOURCE
#include <getopt.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <math.h>
#include <fenv.h>

#include "common.h"
#include "clocks.h"
#include "ptr-traits.h"
#include "float-traits.h"

const char path_trie_default_sep[] = "/.-";

extern const char program[];
extern const char verdate[];
extern const char license[];

typedef char fmt_buf_t[512];

enum error_type_t { FATAL, WARNING, ERROR };

static void verror(
    enum error_type_t err, const char* fmt,
    va_list arg)
{
    static const char* const errors[] = {
        [FATAL]   = "fatal error",
        [WARNING] = "warning",
        [ERROR]   = "error",
    };
    fmt_buf_t b;

    vsnprintf(b, sizeof b - 1, fmt, arg);
    b[sizeof b - 1] = 0;

    fprintf(stderr, "%s: %s: %s\n", program, errors[err], b);
}

void fatal_error(const char* fmt, ...)
{
    va_list arg;

    va_start(arg, fmt);
    verror(FATAL, fmt, arg);
    va_end(arg);

    exit(127);
}

void assert_failed(
    const char* file, int line,
    const char* func, const char* expr)
{
    fatal_error(
        "assertion failed: %s:%d:%s: %s",
        file, line, func, expr);
}

void ensure_failed(
    const char* file, int line,
    const char* func, const char* msg, ...)
{
    va_list arg;
    fmt_buf_t b;

    va_start(arg, msg);
    vsnprintf(b, sizeof b - 1, msg, arg);
    va_end(arg);

    b[sizeof b - 1] = 0;

    fatal_error(
        "%s:%d:%s: %s",
        file, line, func, b);
}

void unexpect_error(
    const char* file, int line,
    const char* func, const char* msg, ...)
{
    va_list arg;
    fmt_buf_t b;

    va_start(arg, msg);
    vsnprintf(b, sizeof b - 1, msg, arg);
    va_end(arg);

    b[sizeof b - 1] = 0;

    fatal_error(
        "unexpected error: %s:%d:%s: %s",
        file, line, func, b);
}

void invalid_argument(
    const char* file, int line,
    const char* func, const char* name,
    const char* msg, ...)
{
    va_list arg;
    fmt_buf_t b;

    va_start(arg, msg);
    vsnprintf(b, sizeof b - 1, msg, arg);
    va_end(arg);

    b[sizeof b - 1] = 0;

    fatal_error(
        "invalid argument: %s:%d:%s: %s='%s'",
        file, line, func, name, b);
}

void not_yet_impl(
    const char* file, int line, const char* func)
{
    fatal_error(
        "%s:%d:%s: function not yet implemented",
        file, line, func);
}

void warning(const char* fmt, ...)
{
    va_list arg;

    va_start(arg, fmt);
    verror(WARNING, fmt, arg);
    va_end(arg);
}

void error(const char* fmt, ...)
{
    va_list arg;

    va_start(arg, fmt);
    verror(ERROR, fmt, arg);
    va_end(arg);

    exit(1);
}

static void version()
{
    fprintf(stdout, "%s: version %s\n\n%s", program, verdate, license);
}

static void usage()
{
    fprintf(stdout,
        "usage: %s [ACTION|OPTION]...\n"
        "where actions are specified as:\n"
        "  -C|--print-config        do nothing but print out the config parameters\n"
        "  -I|--print-sizes         do nothing but print out the node struct sizes\n"
        "                             associated to the set structure specified by\n"
        "                             options `-r|--struct-type' and `-s|--set-type'\n"
        "  -N|--print-names         do nothing but print out the statistics parameter\n"
        "                             names associated to the set structure specified\n"
        "                             by options `-r|--struct-type' and `-s|--set-type'\n"
        "  -L|--load-only           only load in the input (default)\n"
        "  -P|--[print-]set         print out the path set\n"
        "  -E|--[print-]elems       print out the path elements\n"
        "the options are:\n"
        "  -p|--pool-size NUM[KM]   memory pool initial size (default 128K)\n"
        "  -h|--hash-size NUM[KM]   by case, the linear hash table or the gnulib\n"
        "                             hash table initial size (default 1K)\n"
#ifdef CONFIG_PATH_TRIE_NODE_32BIT_OFFSETS
        "  -n|--node-size NUM[KM]   the path trie node table initial size (default 1K)\n"
#endif
        "  -z|--rehash-size FLOAT   grow hash table size by specified factor when rehashing;\n"
        "                             FLOAT must be strictly between 1 and 4; default is NAN,\n"
        "                             i.e. use the table's default value\n"
        "  -l|--rehash-load FLOAT   rehash hash table when its load factor reached given value;\n"
        "                             FLOAT must be strictly between 0 and 1; default is NAN,\n"
        "                             i.e. use the table's default value\n"
        "  -r|--struct-type TYPE    use the specified structure type: any of l|plain-set\n"
        "     --plain-set             or p[ath-trie]; the default is path-trie\n"
        "     --path-trie\n"
        "  -s|--[set-]type TYPE     use the specified set type: any of t[ernary-tree],\n"
        "     --ternary-tree          l[inear]-hash or g[nulib-hash]; the default\n"
        "     --linear-hash           is ternary-tree\n"
        "     --gnulib-hash\n"
        "  -t|--sep[arator]s STR    use the given chars as separators of path elements\n"
        "                             in path tries; the default char set is '%s'\n"
        "  -S|--[no-][print-]stats  print out some statistics information or\n"
        "                             otherwise do not (default not)\n"
        "     --dump-options        print options and exit\n"
        "  -V|--[no-]verbose        be verbose or otherwise do not (default not)\n"
        "     --help-stats          print info about the statistics parameters and exit\n"
        "  -v|--version             print version numbers and exit\n"
        "  -?|--help                display this help info and exit\n",
        program, path_trie_default_sep);
}

#define KB(X) (SZ(X) * SZ(1024))
#define MB(X) (KB(X) * SZ(1024))

struct su_size_t
{
    size_t sz;
    const char* su;
};

static struct su_size_t su_size(size_t sz)
{
    struct su_size_t r;

    r.sz = sz;
    r.su = "";
    if (r.sz  % MB(1) == 0) {
        r.sz /= MB(1);
        r.su = "M";
    }
    else
    if (r.sz  % KB(1) == 0) {
        r.sz /= KB(1);
        r.su = "K";
    }
    return r;
}

enum stat_param_desc_type_t
{
    stat_param_desc_type_size,
    stat_param_desc_type_frac,
    stat_param_desc_type_clocks,
};

struct stat_param_desc_t
{
    enum stat_param_desc_type_t type;
    const char* name;
    const char* help;
};

static const struct stat_param_desc_t stat_params[] = {
#undef  CASE
#define CASE(_type, _id, _name, _help) \
    [stat_param_ ## _id] = {           \
        .type = stat_param_desc_type_  \
             ## _type,                 \
        .name = _name,                 \
        .help = _help                  \
    },
#include "set-stats.def"
};

static void usage_stats()
{
    struct clock_t
    {
        const char* label;
        const char* name;
    };
    static const struct clock_t clocks[3] = {
        { "real", "real" },
        { "user", "user CPU" },
        { "sys",  "system CPU" },
    };
#define CLOCKS(k) ARRAY_NULL_ELEM_REF(clocks, k)
    const struct stat_param_desc_t *p, *e;
    size_t m = 0;

    for (p = stat_params,
         e = p + ARRAY_SIZE(stat_params);
         p < e;
         p ++) {
        size_t l;

        l = strlen(p->name);
        if (p->type ==
            stat_param_desc_type_clocks)
            SIZE_ADD_EQ(l, SZ(5));
        if (m < l)
            m = l;
    }

    for (p = stat_params;
         p < e;
         p ++) {
        const char *q, *r, *e;
        bool c, n = false;
        size_t k = 0, l;
        char s[512];

        c = p->type ==
            stat_param_desc_type_clocks;
        if (!c)
            q = p->help;
        else {
        clocks:
            snprintf(s, sizeof s, p->help, CLOCKS(k)->name);
            q = s;
            n = 0;
        }

        l = strlen(p->name);
        if (c) {
            size_t n = strlen(CLOCKS(k)->label);
            SIZE_ADD_EQ(l, n);
            SIZE_POST_INC(l);
        }
        l = SIZE_SUB(m, l);

        if (c)
            fprintf(stdout, "%s-%s:%-*s ",
                CLOCKS(k)->label, p->name, SIZE_AS_INT(l), "");
        else
            fprintf(stdout, "%s:%-*s ",
                p->name, SIZE_AS_INT(l), "");

#define PRINT_OUT(f, ...)                   \
    do {                                    \
        fprintf(stdout, f, ## __VA_ARGS__); \
        n = true;                           \
    } while (0)

        for (e = q + strlen(q);
             q < e;
             q = r) {
            size_t d;

            r = strchr(q, '\n');
            if (r == NULL) {
                r = e;
                // => q < r == e
            }
            else {
                ASSERT(r >= q);
                ASSERT(r < e);
                r ++;
                // => q < r <= e
            }
            // q < r <= e
            // => q <= r - 1 < e
            // => r[-1] is valid

            d = PTR_DIFF(r, q);
            ASSERT(d > 0);
            if (d == 1 && r[-1] == '\n')
                continue;

            if (n)
                PRINT_OUT("%-*s   ",
                    SIZE_AS_INT(m + 1), "");
            if (true)
                PRINT_OUT("%.*s",
                    SIZE_AS_INT(d), q);
            if (r[-1] != '\n')
                PRINT_OUT("\n");
        }
        if (!n)
            fputc('\n', stdout);

        if (c && ++ k < 3)
            goto clocks;
    }
}

static void dump_options(const struct options_t* opts)
{
#define CASE2(n0, n1) \
    [options_ ## n0 ## _ ## n1 ## _action] = #n0 "-" #n1
    static const char* const actions[] = {
        CASE2(print, config),
        CASE2(print, sizes),
        CASE2(print, names),
        CASE2(load, only),
        CASE2(print, set),
        CASE2(print, elems),
    };
#undef  CASE2
#define CASE2(n0, n1) \
    [options_ ## n0 ## _struct_type] = #n0 "-" #n1
    static const char* const struct_types[] = {
        CASE2(plain, set),
        CASE2(path, trie),
    };
#undef  CASE2
#define CASE2(n0, n1) \
    [options_ ## n0 ## _ ## n1 ## _set_type] = #n0 "-" #n1
    static const char* const set_types[] = {
        CASE2(ternary, tree),
        CASE2(linear, hash),
        CASE2(gnulib, hash)
    };
    static const char* const noyes[] = {
        [0] = "no",
        [1] = "yes"
    };
    struct su_size_t pool_su = su_size(opts->pool_size);
    struct su_size_t hash_su = su_size(opts->hash_size);
#ifdef CONFIG_PATH_TRIE_NODE_32BIT_OFFSETS
    struct su_size_t node_su = su_size(opts->node_size);
#endif
    char* const* p;
    size_t i;

    fprintf(stdout,
        "action:      %s\n"
        "struct-type: %s\n"
        "set-type:    %s\n"
        "pool-size:   %zu%s\n"
        "hash-size:   %zu%s\n"
#ifdef CONFIG_PATH_TRIE_NODE_32BIT_OFFSETS
        "node-size:   %zu%s\n"
#endif
        "rehash-size: %.04f\n"
        "rehash-load: %.04f\n"
        "sep-set:     %s\n"
        "print-stats: %s\n"
        "verbose:     %s\n"
        "argc:        %zu\n",
#define NAME0(X, T)                  \
    ({                               \
        size_t __v = opts->X;        \
        ARRAY_NON_NULL_ELEM(T, __v); \
    })
#define NAME(X)  (NAME0(X, X ## s))
#define NNUL(X)  (opts->X ? opts->X : "-")
#define NOYES(X) (NAME0(X, noyes))
        NAME(action),
        NAME(struct_type),
        NAME(set_type),
        pool_su.sz,
        pool_su.su,
        hash_su.sz,
        hash_su.su,
#ifdef CONFIG_PATH_TRIE_NODE_32BIT_OFFSETS
        node_su.sz,
        node_su.su,
#endif
        opts->rehash_size,
        opts->rehash_load,
        opts->sep_set
        ? opts->sep_set
        : path_trie_default_sep,
        NOYES(print_stats),
        NOYES(verbose),
        opts->argc);

    for (i = 0, p = opts->argv; i < opts->argc; i ++, p ++)
        fprintf(stdout, "argv[%zu]:     %s\n", i, *p);
}

static void invalid_opt_arg(const char* opt_name, const char* opt_arg)
{
    error("invalid argument for '%s' option: '%s'", opt_name, opt_arg);
}

static void illegal_opt_arg(const char* opt_name, const char* opt_arg)
{
    error("illegal argument for '%s' option: '%s'", opt_name, opt_arg);
}

static void missing_opt_arg_str(const char* opt_name)
{
    error("argument for option '%s' not found", opt_name);
}

static void missing_opt_arg_ch(char opt_name)
{
    error("argument for option '-%c' not found", opt_name);
}

static void not_allowed_opt_arg(const char* opt_name)
{
    error("option '%s' does not allow an argument", opt_name);
}

static void invalid_opt_str(const char* opt_name)
{
    error("invalid command line option '%s'", opt_name);
}

static void invalid_opt_ch(char opt_name)
{
    error("invalid command line option '-%c'", opt_name);
}

static size_t parse_num(
    const char* p, const char** q, int b)
{
    if (!isdigit(*p)) {
        *q = p;
        errno = EINVAL;
        return 0;
    }
    errno = 0;
    STATIC(SIZE_MAX == ULONG_MAX);
    return strtoul(p, (char**) q, b);
}

static float parse_float(const char* p)
{
    char* q;
    float r;

    if (isspace(*p)) {
        errno = EINVAL;
        return NAN;
    }

    errno = 0;
    r = strtof(p, &q);
    ASSERT(q >= p);

    if (q == p || *q) {
        errno = EINVAL;
        return NAN;
    }
    return r;
}

// $ . ~/trie-gen/commands.sh
// $ print() { printf '%s\n' "$@"; }
// $ adjust-func() { ssed -R '1s/^/static /;1s/\&/*/;s/(,\s+)/\1enum /;s/(?=t =)/*/;1s/(?<=\()/\n\t/'${1:+";s/($1)_t::([a-z0-9_]+)/\2_\1/"}; }

// $ print 'l plain-set =plain' 'p path-trie =path' |gen-func -f lookup_struct_type -r options_struct_type_t|adjust-func struct_type

static bool lookup_struct_type(
    const char* n, enum options_struct_type_t* t)
{
    // pattern: l|p[ath-trie|lain-set]
    switch (*n ++) {
    case 'l':
        if (*n == 0) {
            *t = options_plain_struct_type;
            return true;
        }
        return false;
    case 'p':
        if (*n == 0) {
            *t = options_path_struct_type;
            return true;
        }
        switch (*n ++) {
        case 'a':
            if (*n ++ == 't' &&
                *n ++ == 'h' &&
                *n ++ == '-' &&
                *n ++ == 't' &&
                *n ++ == 'r' &&
                *n ++ == 'i' &&
                *n ++ == 'e' &&
                *n == 0) {
                *t = options_path_struct_type;
                return true;
            }
            return false;
        case 'l':
            if (*n ++ == 'a' &&
                *n ++ == 'i' &&
                *n ++ == 'n' &&
                *n ++ == '-' &&
                *n ++ == 's' &&
                *n ++ == 'e' &&
                *n ++ == 't' &&
                *n == 0) {
                *t = options_plain_struct_type;
                return true;
            }
        }
    }
    return false;
}

// $ print 't ternary-tree =ternary_tree' 'l linear-hash =linear_hash' 'g gnulib-hash =gnulib_hash'|gen-func -f lookup_set_type -r options_set_type_t|adjust-func set_type

static bool lookup_set_type(
    const char* n, enum options_set_type_t* t)
{
    // pattern: g[nulib-hash]|l[inear-hash]|t[ernary-tree]
    switch (*n ++) {
    case 'g':
        if (*n == 0) {
            *t = options_gnulib_hash_set_type;
            return true;
        }
        if (*n ++ == 'n' &&
            *n ++ == 'u' &&
            *n ++ == 'l' &&
            *n ++ == 'i' &&
            *n ++ == 'b' &&
            *n ++ == '-' &&
            *n ++ == 'h' &&
            *n ++ == 'a' &&
            *n ++ == 's' &&
            *n ++ == 'h' &&
            *n == 0) {
            *t = options_gnulib_hash_set_type;
            return true;
        }
        return false;
    case 'l':
        if (*n == 0) {
            *t = options_linear_hash_set_type;
            return true;
        }
        if (*n ++ == 'i' &&
            *n ++ == 'n' &&
            *n ++ == 'e' &&
            *n ++ == 'a' &&
            *n ++ == 'r' &&
            *n ++ == '-' &&
            *n ++ == 'h' &&
            *n ++ == 'a' &&
            *n ++ == 's' &&
            *n ++ == 'h' &&
            *n == 0) {
            *t = options_linear_hash_set_type;
            return true;
        }
        return false;
    case 't':
        if (*n == 0) {
            *t = options_ternary_tree_set_type;
            return true;
        }
        if (*n ++ == 'e' &&
            *n ++ == 'r' &&
            *n ++ == 'n' &&
            *n ++ == 'a' &&
            *n ++ == 'r' &&
            *n ++ == 'y' &&
            *n ++ == '-' &&
            *n ++ == 't' &&
            *n ++ == 'r' &&
            *n ++ == 'e' &&
            *n ++ == 'e' &&
            *n == 0) {
            *t = options_ternary_tree_set_type;
            return true;
        }
    }
    return false;
}

static enum options_struct_type_t parse_struct_type_optarg(
    const char* opt_arg)
{
    enum options_struct_type_t t = options_path_struct_type;
    if (opt_arg && !lookup_struct_type(opt_arg, &t))
        invalid_opt_arg("struct-type", opt_arg);
    return t;
}

static enum options_set_type_t parse_set_type_optarg(
    const char* opt_arg)
{
    enum options_set_type_t t = options_ternary_tree_set_type;
    if (opt_arg && !lookup_set_type(opt_arg, &t))
        invalid_opt_arg("set-type", opt_arg);
    return t;
}

#if 0
static size_t parse_size_optarg(
    const char* opt_name, const char* opt_arg)
{
    const char *p, *q = NULL;
    size_t n, v, d;

    if (!(n = strlen(opt_arg)))
        invalid_opt_arg(opt_name, opt_arg);
    v = parse_num(p = opt_arg, &q, 10);
    d = PTR_OFFSET(q, p, n);
    if (errno || (d != n))
        invalid_opt_arg(opt_name, opt_arg);
    //if (v == 0)
    //	illegal_opt_arg(opt_name, opt_arg);
    return v;
}
#endif

static size_t parse_su_size_optarg(
    const char* opt_name, const char* opt_arg,
    size_t max_size)
{
    const char *p, *q = NULL;
    size_t n, v, d;

    if (!(n = strlen(opt_arg)))
        invalid_opt_arg(opt_name, opt_arg);
    v = parse_num(p = opt_arg, &q, 10);
    d = PTR_OFFSET(q, p, n);
    if (errno ||
        (d == 0) ||
        (d < n - 1) ||
        (d == n - 1 && *q != 'k' && *q != 'K' &&
            *q != 'm' && *q != 'M'))
        invalid_opt_arg(opt_name, opt_arg);
    switch (*q) {
    case '\0':
        break;
    case 'k':
    case 'K':
        STATIC(KB(1) <= SIZE_MAX);
        if (v > SIZE_MAX / KB(1))
            illegal_opt_arg(opt_name, opt_arg);
        v *= KB(1);
        break;
    case 'm':
    case 'M':
        STATIC(MB(1) <= SIZE_MAX);
        if (v > SIZE_MAX / MB(1))
            illegal_opt_arg(opt_name, opt_arg);
        v *= MB(1);
        break;
    }
    if (v == 0 || v > max_size)
        illegal_opt_arg(opt_name, opt_arg);
    return v;
}

static float parse_float_optarg(
    const char* opt_name, const char* opt_arg,
    float lower, float upper)
{
    float v;

    if (!strlen(opt_arg))
        invalid_opt_arg(opt_name, opt_arg);
    v = parse_float(opt_arg);
    if (errno)
        invalid_opt_arg(opt_name, opt_arg);
    if (isnan(v))
        return v;
    ASSERT(FLOAT_LT(lower, upper));
#define FLOAT_EPSILON 0.11f
    if (!FLOAT_GS(v, lower) || !FLOAT_LS(v, upper))
        illegal_opt_arg(opt_name, opt_arg);
#undef  FLOAT_EPSILON
    return v;
}

static const char* parse_sep_set_optarg(
    const char* opt_name, const char* opt_arg)
{
    char *t, *p, *e, *q;
    size_t n;
    char c;
    bool b;

    if (opt_arg == NULL)
        return NULL;

    if (!(n = strlen(opt_arg)) ||
        !memchr(opt_arg, '/', n))
        invalid_opt_arg(opt_name, opt_arg);

    if (n == 1)
        return opt_arg;

    t = malloc(n);
    ENSURE(t != NULL, "malloc failed");

    memcpy(t, opt_arg, n);
    // stev: insertion sort of 't'
    for (p = t + 1, e = t + n; p < e; p ++) {
        c = *p;
        for (q = p; q > t && c < q[-1]; q --)
            *q = q[-1];
        *q = c;
    }
    // stev: check for uniqueness
    for (p = t, q = p ++; p < e; p ++, q ++) {
        if (*p == *q)
            break;
    }
    b = p < e;

    free(t);

    if (b)
        illegal_opt_arg(opt_name, opt_arg);

    return opt_arg;
}

const struct options_t* options(
    int argc, char* argv[])
{
    static struct options_t opts = {
        .action      = options_load_only_action,
        .struct_type = options_path_struct_type,
        .set_type    = options_ternary_tree_set_type,
        .pool_size   = KB(128),
        .hash_size   = KB(1),
#ifdef CONFIG_PATH_TRIE_NODE_32BIT_OFFSETS
        .node_size   = KB(1),
#endif
        .rehash_size = NAN,
        .rehash_load = NAN,
        .sep_set     = NULL,
        .verbose     = false,
        .argc        = 0,
        .argv        = NULL
    };

    enum {
        // stev: actions:
        print_config_act   = 'C',
        print_sizes_act    = 'I',
        print_names_act    = 'N',
        load_only_act      = 'L',
        print_set_act      = 'P',
        print_elems_act    = 'E',

        // stev: options:
        pool_size_opt      = 'p',
        hash_size_opt      = 'h',
#ifdef CONFIG_PATH_TRIE_NODE_32BIT_OFFSETS
        node_size_opt      = 'n',
#endif
        struct_type_opt    = 'r',
        set_type_opt       = 's',
        rehash_size_opt    = 'z',
        rehash_load_opt    = 'l',
        sep_set_opt        = 't',
        print_stats_opt    = 'S',
        verbose_opt        = 'V',
        version_opt        = 'v',
        help_opt           = '?',

        dump_opts_opt      = 128,
        no_verbose_opt,
        no_print_stats_opt,
        plain_set_opt,
        path_trie_opt,
        ternary_tree_opt,
        linear_hash_opt,
        gnulib_hash_opt,

        help_stats_opt,
    };

    static struct option long_opts[] = {
        { "print-config",   0,       0, print_config_act },
        { "print-sizes",    0,       0, print_sizes_act },
        { "print-names",    0,       0, print_names_act },
        { "load-only",      0,       0, load_only_act },
        { "print-set",      0,       0, print_set_act },
        { "set",            0,       0, print_set_act },
        { "print-elems",    0,       0, print_elems_act },
        { "elems",          0,       0, print_elems_act },
        { "struct-type",    1,       0, struct_type_opt },
        { "plain-set",      0,       0, plain_set_opt },
        { "path-trie",      0,       0, path_trie_opt },
        { "set-type",       1,       0, set_type_opt },
        { "type",           1,       0, set_type_opt },
        { "ternary-tree",   0,       0, ternary_tree_opt },
        { "linear-hash",    0,       0, linear_hash_opt },
        { "gnulib-hash",    0,       0, gnulib_hash_opt },
        { "pool-size",      1,       0, pool_size_opt },
        { "hash-size",      1,       0, hash_size_opt },
#ifdef CONFIG_PATH_TRIE_NODE_32BIT_OFFSETS
        { "node-size",      1,       0, node_size_opt },
#endif
        { "rehash-size",    1,       0, rehash_size_opt },
        { "rehash-load",    1,       0, rehash_load_opt },
        { "separators",     1,       0, sep_set_opt },
        { "seps",           1,       0, sep_set_opt },
        { "print-stats",    0,       0, print_stats_opt },
        { "stats",          0,       0, print_stats_opt },
        { "no-print-stats", 0,       0, no_print_stats_opt },
        { "no-stats",       0,       0, no_print_stats_opt },
        { "dump-options",   0,       0, dump_opts_opt },
        { "verbose",        0,       0, verbose_opt },
        { "no-verbose",     0,       0, no_verbose_opt },
        { "version",        0,       0, version_opt },
        { "help-stats",     0,       0, help_stats_opt },
        { "help",           0, &optopt, help_opt },
        { 0,                0,       0, 0 }
    };
    static const char short_opts[] =
        ":"
        "CINLPE"
#ifdef CONFIG_PATH_TRIE_NODE_32BIT_OFFSETS
        "n:"
#endif
        "h:l:p:r:s:St:Vvz:";

    struct bits_opts_t {
        bits_t dump: 1;
        bits_t usage: 1;
        bits_t usage_stats: 1;
        bits_t version: 1;
    };
    struct bits_opts_t bits = {
        .dump        = false,
        .usage       = false,
        .usage_stats = false,
        .version     = false
    };

    int opt;

#define argv_optind()                      \
    ({                                     \
        size_t i = INT_AS_SIZE(optind);    \
        ASSERT_SIZE_DEC_NO_OVERFLOW(i);    \
        ASSERT(i - 1 < INT_AS_SIZE(argc)); \
        argv[i - 1];                       \
    })

#define optopt_char()            \
    ({                           \
        ASSERT(isascii(optopt)); \
        (char) optopt;           \
    })

    opterr = 0;
    optind = 1;
    while ((opt = getopt_long(argc, argv, short_opts,
        long_opts, 0)) != EOF) {
        switch (opt) {
        case print_config_act:
            opts.action = options_print_config_action;
            break;
        case print_sizes_act:
            opts.action = options_print_sizes_action;
            break;
        case print_names_act:
            opts.action = options_print_names_action;
            break;
        case load_only_act:
            opts.action = options_load_only_action;
            break;
        case print_set_act:
            opts.action = options_print_set_action;
            break;
        case print_elems_act:
            opts.action = options_print_elems_action;
            break;
        case struct_type_opt:
            opts.struct_type = parse_struct_type_optarg(optarg);
            break;
        case plain_set_opt:
            opts.struct_type = options_plain_struct_type;
            break;
        case path_trie_opt:
            opts.struct_type = options_path_struct_type;
            break;
        case set_type_opt:
            opts.set_type = parse_set_type_optarg(optarg);
            break;
        case ternary_tree_opt:
            opts.set_type = options_ternary_tree_set_type;
            break;
        case linear_hash_opt:
            opts.set_type = options_linear_hash_set_type;
            break;
        case gnulib_hash_opt:
            opts.set_type = options_gnulib_hash_set_type;
            break;
        case pool_size_opt:
            opts.pool_size = 
                parse_su_size_optarg("pool-size", optarg,
                    MB(512));
            break;
        case hash_size_opt:
            opts.hash_size = 
                parse_su_size_optarg("hash-size", optarg,
                    MB(32));
            break;
#ifdef CONFIG_PATH_TRIE_NODE_32BIT_OFFSETS
        case node_size_opt:
            opts.node_size = 
                parse_su_size_optarg("node-size", optarg,
                    MB(32));
            break;
#endif
        case rehash_size_opt:
            opts.rehash_size = 
                parse_float_optarg("rehash-size", optarg,
                    1.0f, 4.0f);
            break;
        case rehash_load_opt:
            opts.rehash_load = 
                parse_float_optarg("rehash-load", optarg,
                    0.0f, 1.0f);
            break;
        case sep_set_opt:
            opts.sep_set = 
                parse_sep_set_optarg("sep-set", optarg);
            break;
        case print_stats_opt:
            opts.print_stats = true;
            break;
        case no_print_stats_opt:
            opts.print_stats = false;
            break;
        case dump_opts_opt:
            bits.dump = true;
            break;
        case verbose_opt:
            opts.verbose = true;
            break;
        case no_verbose_opt:
            opts.verbose = false;
            break;
        case version_opt:
            bits.version = true;
            break;
        case help_stats_opt:
            bits.usage_stats = true;
            break;
        case 0:
            bits.usage = true;
            break;
        case ':': {
            const char* opt = argv_optind();
            if (opt[0] == '-' && opt[1] == '-')
                missing_opt_arg_str(opt);
            else
                missing_opt_arg_ch(optopt_char());
            break;
        }
        case '?':
        default:
            if (optopt == 0)
                invalid_opt_str(argv_optind());
            else
            if (optopt != '?') {
                char* opt = argv_optind();
                if (opt[0] == '-' && opt[1] == '-') {
                    char* end = strchr(opt, '=');
                    if (end) *end = '\0';
                    not_allowed_opt_arg(opt);
                }
                else
                    invalid_opt_ch(optopt_char());
            }
            else
                bits.usage = true;
            break;
        }
    }

    ASSERT(argc >= optind);

    argc -= optind;
    argv += optind;

    opts.argc = argc;
    opts.argv = argv;

    if (bits.version)
        version();
    if (bits.dump)
        dump_options(&opts);
    if (bits.usage_stats)
        usage_stats();
    if (bits.usage)
        usage();

    if (bits.dump ||
        bits.version ||
        bits.usage_stats ||
        bits.usage)
        exit(0);

    return &opts;
}

#define MATH_IS_ERROR()    \
    (                      \
        errno ||           \
        fetestexcept(      \
            FE_INVALID |   \
            FE_DIVBYZERO | \
            FE_OVERFLOW |  \
            FE_UNDERFLOW)  \
    )
#define MATH_RESET_ERROR()  \
    do {                    \
        errno = 0;          \
        feclearexcept(      \
            FE_ALL_EXCEPT); \
    } while (0)
#define MATH_EVAL(e)        \
    ({                      \
        MATH_RESET_ERROR(); \
        e;                  \
        !MATH_IS_ERROR();   \
    })

bool compute_size_frac(
    size_t n, size_t m, struct size_frac_t* r)
{
    const size_t p = 100000;
    double f;

    STATIC(SIZE_MAX == ULONG_MAX);

    if (!MATH_EVAL(f = (double) n / (double) m) ||
        !MATH_EVAL(r->num = (size_t) f) ||
        !MATH_EVAL(f *= (double) p) ||
        !MATH_EVAL(r->denom = lround(f)))
        return false;

    if (!SIZE_MUL_NO_OVERFLOW(r->num, p) ||
        !SIZE_SUB_NO_OVERFLOW(r->denom, r->num * p))
        return false;

    r->denom -= r->num * p;
    ASSERT(r->denom < p);
    r->prec = 5;

    return true;
}

#define FIELD_STAT_PARAM_NAME(p) \
    (                            \
        ARRAY_NULL_ELEM_REF(     \
            stat_params,         \
            p->name              \
        )->name                  \
    )
#define FIELD_STAT_PARAM_(p, n, t)             \
    ({                                         \
        ENSURE(p->val.n < SIZE_MAX,            \
            "field '%s' is not of type '%s'",  \
            FIELD_STAT_PARAM_NAME(p), #t);     \
        (t const*) (((char*) obj) + p->val.n); \
    })
#define SIZE_STAT_PARAM(p) \
    FIELD_STAT_PARAM_(p, size_field, size_t)
#define TIME_STAT_PARAM(p) \
    FIELD_STAT_PARAM_(p, time_field, struct clocks_t)

#define FUNC_STAT_PARAM_(p, n, t)                \
    ({                                           \
        ENSURE(p->val.n != NULL,                 \
            "field '%s' is not of %s-func type", \
            FIELD_STAT_PARAM_NAME(p), #t);       \
        p->val.n(obj);                           \
    })
#define SIZE_FUNC_STAT_PARAM(p) \
    FUNC_STAT_PARAM_(p, size_func, size)
#define FRAC_FUNC_STAT_PARAM(p) \
    FUNC_STAT_PARAM_(p, frac_func, frac)

void stat_params_print(
    const struct stat_param_info_t* params,
    size_t len, const void* obj,
    bool only_names, FILE* file)
{
    const struct stat_param_info_t *p, *e;
    const struct stat_param_desc_t* d;
    const size_t M = obj ? 22 : 0;
    size_t m = M, l, w;

    for (p = params, e = p + len; p < e; p ++) {
        d = ARRAY_NULL_ELEM_REF(stat_params, p->name);
        ASSERT(d != NULL);

        if (only_names) {
            fprintf(file, "%s\n", d->name);
            continue;
        }

        l = strlen(d->name);
        if (m < l)
            m = l;
    }

    if (only_names)
        return;

    for (p = params; p < e; p ++) {
        d = ARRAY_NULL_ELEM_REF(stat_params, p->name);
        ASSERT(d != NULL);

        if (p->type != stat_param_time_field_type) {
            l = strlen(d->name);
            w = SIZE_SUB(m, l);
            fprintf(file, "%s:%-*s ",
                d->name, SIZE_AS_INT(w), "");
        }

        switch (p->type) {

        case stat_param_size_field_type:
            fprintf(file, "%zu\n", *SIZE_STAT_PARAM(p));
            break;

        case stat_param_time_field_type:
            clocks_print(TIME_STAT_PARAM(p), d->name, M, true, file);
            break;

        case stat_param_size_func_type:
            fprintf(file, "%zu\n", SIZE_FUNC_STAT_PARAM(p));
            break;

        case stat_param_frac_func_type: {
            struct size_frac_t v;
            const size_t b = 10;
            size_t k, d;

            v = FRAC_FUNC_STAT_PARAM(p);

            for (k = v.prec, d = 1; k; k --)
                 SIZE_MUL_EQ(d, b);
            ASSERT(v.denom < d);

            fprintf(file, "%zu.%0*zu\n",
                v.num, SIZE_AS_INT(v.prec), v.denom);
            break;
        }

        default:
            UNEXPECT_VAR("%d", p->type);
        }
    }
}

void print_config(FILE* file)
{
    struct config_param_t
    {
        const char* name;
        const char* val;
    };

#define EVAL_CONFIG_(v)     #v
#define EVAL_CONFIG(v)      EVAL_CONFIG_(v)
#define DECL_CONFIG_B(n, v) { .name = #n, .val = EVAL_CONFIG(v) }
#define DECL_CONFIG_V(n)    DECL_CONFIG_B(CONFIG_ ## n, CONFIG_ ## n)
#define DECL_CONFIG_D(n)    DECL_CONFIG_B(CONFIG_ ## n, yes)
#define DECL_CONFIG_U(n)    DECL_CONFIG_B(CONFIG_ ## n, no)

    static const struct config_param_t params[] = {
#ifndef CONFIG_PATH_TRIE_NODE_32BIT_OFFSETS
        DECL_CONFIG_U(PATH_TRIE_NODE_32BIT_OFFSETS),
#else
        DECL_CONFIG_D(PATH_TRIE_NODE_32BIT_OFFSETS),
#endif
#ifndef CONFIG_PATH_TRIE_ELEM_32BIT_OFFSETS
        DECL_CONFIG_U(PATH_TRIE_ELEM_32BIT_OFFSETS),
#else
        DECL_CONFIG_D(PATH_TRIE_ELEM_32BIT_OFFSETS),
#endif
#ifndef CONFIG_PATH_TRIE_UINT_HASH_IDENTOP
        DECL_CONFIG_U(PATH_TRIE_UINT_HASH_IDENTOP),
#else
        DECL_CONFIG_D(PATH_TRIE_UINT_HASH_IDENTOP),
#endif
#ifndef CONFIG_TRIE_PATH_SET_PRINT_SET
        DECL_CONFIG_U(TRIE_PATH_SET_PRINT_SET),
#else
        DECL_CONFIG_D(TRIE_PATH_SET_PRINT_SET),
#endif
        DECL_CONFIG_V(VALUE_TYPE_SIZE),
#if defined(BIT32) && __WORDSIZE != 32
#error unexpected BIT32 to be defined and __WORDSIZE != 32
#endif
#ifndef BIT32
        DECL_CONFIG_B(32BIT, no),
#else
        DECL_CONFIG_B(32BIT, yes),
#endif
#ifndef DEBUG
        DECL_CONFIG_B(DEBUG, no),
#else
        DECL_CONFIG_B(DEBUG, yes),
#endif
#ifndef OPT
        DECL_CONFIG_B(OPT, -)
#else
        DECL_CONFIG_B(OPT, OPT)
#endif
    };

    const struct config_param_t *p, *e;
    size_t m = 0, l, w;

    for (p = params,
         e = params + ARRAY_SIZE(params);
         p < e;
         p ++) {
        l = strlen(p->name);
        if (m < l)
            m = l;
    }
    for (p = params; p < e; p ++) {
        l = strlen(p->name);
        w = SIZE_SUB(m, l);
        fprintf(file, "%s:%-*s %s\n",
            p->name, SIZE_AS_INT(w), "",
            p->val);
    }
}


