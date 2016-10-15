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

#ifndef __POOL_ALLOC_H
#define __POOL_ALLOC_H

struct pool_alloc_node_t;

struct pool_alloc_t
{
    struct pool_alloc_node_t* base;
    size_t init_size;
    size_t max_size;
};

void pool_alloc_init(
    struct pool_alloc_t* alloc,
    size_t init_size, size_t max_size);

void pool_alloc_done(
    struct pool_alloc_t* alloc);

void* pool_alloc_allocate(
    struct pool_alloc_t* alloc,
    size_t size, size_t align);

size_t pool_alloc_get_struct_mem(
    const struct pool_alloc_t* alloc);

#endif /*__POOL_ALLOC_H*/


