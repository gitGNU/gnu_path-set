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

#if defined(SET_STATS_NEED_NODE_SIZES) && \
    defined(SET_STATS_NEED_LINE_SIZES)
#error cannot have both \
    SET_STATS_NEED_NODE_SIZES and \
    SET_STATS_NEED_LINE_SIZES defined
#endif

#ifdef  SET_STATS_NAME
#define SET_STATS_MAKE_NAME__(n, s) n ## _set_stats_ ## s
#undef  SET_STATS_MAKE_NAME_
#define SET_STATS_MAKE_NAME_(n, s)  SET_STATS_MAKE_NAME__(n, s)
#undef  SET_STATS_MAKE_NAME
#define SET_STATS_MAKE_NAME(s)      SET_STATS_MAKE_NAME_(SET_STATS_NAME, s)
#else
#undef  SET_STATS_MAKE_NAME_
#define SET_STATS_MAKE_NAME_(s)     set_stats_ ## s
#undef  SET_STATS_MAKE_NAME
#define SET_STATS_MAKE_NAME         SET_STATS_MAKE_NAME_
#endif

#define SET_STATS_STRUCT_TYPE  SET_STATS_MAKE_NAME(t)

// From: Laurent Deniau <laurent...@cern.ch>
// Newsgroups: comp.std.c
// Subject: __VA_NARG__
// Date: Mon, 16 Jan 2006 18:43:40 +0100
// https://groups.google.com/forum/message/raw?msg=comp.std.c/d-6Mj5Lko_s/5R6bMWTEbzQJ

#define SET_STATS_STRUCT_DECL_NARGS___() \
    16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
#define SET_STATS_STRUCT_DECL_NARGS__(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, N, ...) \
    N
#define SET_STATS_STRUCT_DECL_NARGS_(...) \
    SET_STATS_STRUCT_DECL_NARGS__(__VA_ARGS__)
#define SET_STATS_STRUCT_DECL_NARGS(...) \
    SET_STATS_STRUCT_DECL_NARGS_(__VA_ARGS__, \
        SET_STATS_STRUCT_DECL_NARGS___())

#define SET_STATS_STRUCT_DECL_REPEAT_0(m, ...)
#define SET_STATS_STRUCT_DECL_REPEAT_1(m, ...) \
    SET_STATS_STRUCT_DECL_REPEAT_0(m, __VA_ARGS__) m(0, __VA_ARGS__)
#define SET_STATS_STRUCT_DECL_REPEAT_2(m, ...) \
    SET_STATS_STRUCT_DECL_REPEAT_1(m, __VA_ARGS__) m(1, __VA_ARGS__)
#define SET_STATS_STRUCT_DECL_REPEAT_3(m, ...) \
    SET_STATS_STRUCT_DECL_REPEAT_2(m, __VA_ARGS__) m(2, __VA_ARGS__)
#define SET_STATS_STRUCT_DECL_REPEAT_4(m, ...) \
    SET_STATS_STRUCT_DECL_REPEAT_3(m, __VA_ARGS__) m(3, __VA_ARGS__)
#define SET_STATS_STRUCT_DECL_REPEAT_5(m, ...) \
    SET_STATS_STRUCT_DECL_REPEAT_4(m, __VA_ARGS__) m(4, __VA_ARGS__)
#define SET_STATS_STRUCT_DECL_REPEAT_6(m, ...) \
    SET_STATS_STRUCT_DECL_REPEAT_5(m, __VA_ARGS__) m(5, __VA_ARGS__)
#define SET_STATS_STRUCT_DECL_REPEAT_7(m, ...) \
    SET_STATS_STRUCT_DECL_REPEAT_6(m, __VA_ARGS__) m(6, __VA_ARGS__)
#define SET_STATS_STRUCT_DECL_REPEAT_8(m, ...) \
    SET_STATS_STRUCT_DECL_REPEAT_7(m, __VA_ARGS__) m(7, __VA_ARGS__)
#define SET_STATS_STRUCT_DECL_REPEAT_9(m, ...) \
    SET_STATS_STRUCT_DECL_REPEAT_8(m, __VA_ARGS__) m(8, __VA_ARGS__)
#define SET_STATS_STRUCT_DECL_REPEAT_10(m, ...) \
    SET_STATS_STRUCT_DECL_REPEAT_9(m, __VA_ARGS__) m(9, __VA_ARGS__)
#define SET_STATS_STRUCT_DECL_REPEAT_11(m, ...) \
    SET_STATS_STRUCT_DECL_REPEAT_10(m, __VA_ARGS__) m(10, __VA_ARGS__)
#define SET_STATS_STRUCT_DECL_REPEAT_12(m, ...) \
    SET_STATS_STRUCT_DECL_REPEAT_11(m, __VA_ARGS__) m(11, __VA_ARGS__)
#define SET_STATS_STRUCT_DECL_REPEAT_13(m, ...) \
    SET_STATS_STRUCT_DECL_REPEAT_12(m, __VA_ARGS__) m(12, __VA_ARGS__)
#define SET_STATS_STRUCT_DECL_REPEAT_14(m, ...) \
    SET_STATS_STRUCT_DECL_REPEAT_13(m, __VA_ARGS__) m(13, __VA_ARGS__)
#define SET_STATS_STRUCT_DECL_REPEAT_15(m, ...) \
    SET_STATS_STRUCT_DECL_REPEAT_14(m, __VA_ARGS__) m(14, __VA_ARGS__)
#define SET_STATS_STRUCT_DECL_REPEAT_16(m, ...) \
    SET_STATS_STRUCT_DECL_REPEAT_15(m, __VA_ARGS__) m(15, __VA_ARGS__)

#define SET_STATS_STRUCT_DECL_REPEAT__(n, m, ...) \
    SET_STATS_STRUCT_DECL_REPEAT_ ## n(m, __VA_ARGS__)
#define SET_STATS_STRUCT_DECL_REPEAT_(n, m, ...) \
    SET_STATS_STRUCT_DECL_REPEAT__(n, m, __VA_ARGS__)

#define SET_STATS_STRUCT_DECL_REPEAT(m, ...)      \
    SET_STATS_STRUCT_DECL_REPEAT_(                \
        SET_STATS_STRUCT_DECL_NARGS(__VA_ARGS__), \
        m, __VA_ARGS__)

#define SET_STATS_STRUCT_DECL_SELECT_0(_0, ...) _0
#define SET_STATS_STRUCT_DECL_SELECT_1(_0, _1, ...) _1
#define SET_STATS_STRUCT_DECL_SELECT_2(_0, _1, _2, ...) _2
#define SET_STATS_STRUCT_DECL_SELECT_3(_0, _1, _2, _3, ...) _3
#define SET_STATS_STRUCT_DECL_SELECT_4(_0, _1, _2, _3, _4, ...) _4
#define SET_STATS_STRUCT_DECL_SELECT_5(_0, _1, _2, _3, _4, _5, ...) _5
#define SET_STATS_STRUCT_DECL_SELECT_6(_0, _1, _2, _3, _4, _5, _6, ...) _6
#define SET_STATS_STRUCT_DECL_SELECT_7(_0, _1, _2, _3, _4, _5, _6, _7, ...) _7
#define SET_STATS_STRUCT_DECL_SELECT_8(_0, _1, _2, _3, _4, _5, _6, _7, _8, ...) _8
#define SET_STATS_STRUCT_DECL_SELECT_9(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, ...) _9
#define SET_STATS_STRUCT_DECL_SELECT_10(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, ...) _10
#define SET_STATS_STRUCT_DECL_SELECT_11(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, ...) _11
#define SET_STATS_STRUCT_DECL_SELECT_12(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, ...) _12
#define SET_STATS_STRUCT_DECL_SELECT_13(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, ...) _13
#define SET_STATS_STRUCT_DECL_SELECT_14(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, ...) _14
#define SET_STATS_STRUCT_DECL_SELECT_15(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, ...) _15
#define SET_STATS_STRUCT_DECL_SELECT_16(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, ...) _16

#define SET_STATS_STRUCT_DECL_SELECT(i, ...) \
    SET_STATS_STRUCT_DECL_SELECT_ ## i(__VA_ARGS__)

#define SET_STATS_STRUCT_DECL_CLOCKST(i, ...) \
    struct clocks_t SET_STATS_STRUCT_DECL_SELECT(i, __VA_ARGS__);
#define SET_STATS_STRUCT_DECL_SIZET(i, ...) \
    size_t SET_STATS_STRUCT_DECL_SELECT(i, __VA_ARGS__);

#ifndef SET_STATS_STRUCT_DECL_CLOCKS
#if 0
#define SET_STATS_STRUCT_DECL_CLOCKS() \
    insert_time, lookup_time
#else
#define SET_STATS_STRUCT_DECL_CLOCKS() \
    insert_time
#endif
#endif

#ifdef  SET_STATS_NEED_NODE_SIZES
#undef  SET_STATS_STRUCT_DECL_SIZES
#define SET_STATS_STRUCT_DECL_SIZES(...) \
    dups_node, uniq_node, insert_eq, ## __VA_ARGS__
#endif

#ifdef  SET_STATS_NEED_LINE_SIZES
#undef  SET_STATS_STRUCT_DECL_SIZES
#define SET_STATS_STRUCT_DECL_SIZES(...) \
    dups_line, uniq_line, line_mem, ## __VA_ARGS__
#endif

#ifndef SET_STATS_NEED_CLOCKS
#undef  SET_STATS_STRUCT_DECL
#define SET_STATS_STRUCT_DECL(...)                \
struct SET_STATS_STRUCT_TYPE                      \
{                                                 \
    SET_STATS_STRUCT_DECL_REPEAT(                 \
        SET_STATS_STRUCT_DECL_SIZET,              \
        SET_STATS_STRUCT_DECL_SIZES(__VA_ARGS__)) \
}; 
#else // SET_STATS_NEED_CLOCKS
#undef  SET_STATS_STRUCT_DECL
#define SET_STATS_STRUCT_DECL(...)                \
struct SET_STATS_STRUCT_TYPE                      \
{                                                 \
    SET_STATS_STRUCT_DECL_REPEAT(                 \
        SET_STATS_STRUCT_DECL_CLOCKST,            \
        SET_STATS_STRUCT_DECL_CLOCKS())           \
    SET_STATS_STRUCT_DECL_REPEAT(                 \
        SET_STATS_STRUCT_DECL_SIZET,              \
        SET_STATS_STRUCT_DECL_SIZES(__VA_ARGS__)) \
}; 
#endif // SET_STATS_NEED_CLOCKS


