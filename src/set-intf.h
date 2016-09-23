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

#ifndef __SET_INTF_H
#define __SET_INTF_H

#include <stdbool.h>

#include "common.h"

struct set_intf_t;

struct set_intf_t* set_intf_create(
    const struct options_t* opt);

void set_intf_destroy(
    struct set_intf_t* intf);

void set_intf_load(
    struct set_intf_t* intf, FILE* file,
    bool verbose);

bool set_intf_is_empty(
    const struct set_intf_t* intf);

void set_intf_print_set(
    const struct set_intf_t* intf,
    FILE* file);

void set_intf_print_elems(
    const struct set_intf_t* intf,
    FILE* file);

void set_intf_print_stats(
    const struct set_intf_t* intf,
    FILE* file);

void print_sizes(
    const struct options_t* opt,
    FILE* file);

void print_names(
    const struct options_t* opt,
    FILE* file);

#endif // __SET_INTF_H

