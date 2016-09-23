// Copyright (C) 2016  Stefan Vargyas
// 
// This file is part of Json-Type.
// 
// Json-Type is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// Json-Type is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with Json-Type.  If not, see <http://www.gnu.org/licenses/>.

#include "config.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "pool-alloc.h"
#include "ptr-traits.h"
#include "common.h"

void pool_alloc_init(
    struct pool_alloc_t* alloc, size_t size)
{
    ASSERT(size > 0);

    memset(alloc, 0, sizeof(struct pool_alloc_t));

    alloc->pool = malloc(size);
    ENSURE(alloc->pool != NULL, "malloc failed");

    alloc->end = alloc->pool + size;
    alloc->ptr = alloc->pool;
}

#define ASSERT_POOL_ALLOC_INVARIANTS(pool) \
    do {                                   \
        ASSERT(alloc->end > alloc->pool);  \
        ASSERT(alloc->ptr >= alloc->pool); \
        ASSERT(alloc->ptr <= alloc->end);  \
    } while (0)

void pool_alloc_done(
    struct pool_alloc_t* alloc)
{
    free(alloc->pool);
}

size_t pool_alloc_get_size(
    const struct pool_alloc_t* alloc)
{
    ASSERT_POOL_ALLOC_INVARIANTS(pool);
    return PTR_DIFF(alloc->end, alloc->pool);
}

void* pool_alloc_allocate(
    struct pool_alloc_t* alloc,
    size_t n_bytes, size_t align)
{
#if !CONFIG_PTR_OUT_OF_BOUNDS_ARITHMETIC
    size_t a, s;
#endif
    char* r;

    // stev: 'align' need to be a power of 2
    ASSERT(SIZE_IS_POW2(align));

    // stev: assert the invariants of the pool
    ASSERT_POOL_ALLOC_INVARIANTS(pool);

    if (n_bytes == 0)
        n_bytes = 1;

#if CONFIG_PTR_OUT_OF_BOUNDS_ARITHMETIC
    r = PTR_ALIGN(alloc->ptr, align);
    if (r + n_bytes > alloc->end)
        return NULL;
#else
    // stev: the amount bytes of alignment
    a = PTR_ALIGN_SIZE(alloc->ptr, align);

    // stev: the free space in the pool
    s = PTR_DIFF(alloc->end, alloc->ptr);

    // stev: assume:
    //   r == alloc->ptr + a
    // we have that:
    //   n_bytes + a > s <=>
    //   r + n_bytes > alloc->end
    // proof:
    //   n_bytes + a > s <=>
    //   alloc->ptr + a + n_bytes >
    //   alloc->ptr + (alloc->end - alloc->ptr) <=>
    //   r + n_bytes > alloc->end
    ASSERT_SIZE_ADD_NO_OVERFLOW(n_bytes, a);
    if (n_bytes + a > s)
        return NULL;

    r = alloc->ptr + a;
#endif
    alloc->ptr = r + n_bytes;

    return r;
}


