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

#ifndef SET_NAME
#error  SET_NAME not defined
#endif

#define SET_MAKE_NAME__(n, s) n ## _set_ ## s
#define SET_MAKE_NAME_(n, s) SET_MAKE_NAME__(n, s)
#define SET_MAKE_NAME(s)    SET_MAKE_NAME_(SET_NAME, s)

#define SET_TYPE            SET_MAKE_NAME(t)
#define SET_CREATE          SET_MAKE_NAME(create)
#define SET_DESTROY         SET_MAKE_NAME(destroy)
#define SET_LOAD            SET_MAKE_NAME(load)
#define SET_IS_EMPTY        SET_MAKE_NAME(is_empty)
#define SET_PRINT_SET       SET_MAKE_NAME(print_set)
#define SET_PRINT_ELEMS     SET_MAKE_NAME(print_elems)
#define SET_PRINT_STATS     SET_MAKE_NAME(print_stats)
#define SET_PRINT_SIZES     SET_MAKE_NAME(print_sizes)

struct SET_TYPE;

struct SET_TYPE* SET_CREATE(
    const struct options_t* opt);

void SET_DESTROY(
    struct SET_TYPE* set);

void SET_LOAD(
    struct SET_TYPE* set,
    FILE* file, bool verbose);

bool SET_IS_EMPTY(
    const struct SET_TYPE* set);

void SET_PRINT_SET(
    const struct SET_TYPE* set,
    FILE* file);

void SET_PRINT_ELEMS(
    const struct SET_TYPE* set,
    FILE* file);

void SET_PRINT_STATS(
    const struct SET_TYPE* set,
    bool only_names,
    FILE* file);

void SET_PRINT_SIZES(
    FILE* file);

