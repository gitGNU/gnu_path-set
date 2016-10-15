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
#include <stdlib.h>
#include <string.h>

#include "pool-alloc.h"
#include "ptr-traits.h"
#include "common.h"

struct pool_alloc_node_t
{
    struct pool_alloc_node_t* next;

    size_t size;
    size_t used;
    char   pool[0];
};

#define ASSERT_POOL_ALLOC_INVARIANTS(p)          \
    do {                                         \
        ASSERT((p)->init_size <= (p)->max_size); \
    } while (0)

#define ASSERT_POOL_ALLOC_NODE_INVARIANTS(n) \
    do {                                     \
        ASSERT((n) != NULL);                 \
        ASSERT((n)->used <= (n)->size);      \
    } while (0)

void pool_alloc_init(
    struct pool_alloc_t* alloc,
    size_t init_size, size_t max_size)
{
    const size_t M = SIZE_MAX / 2;

    if (max_size == 0 ||
        max_size > M)
        max_size = M;

    if (init_size == 0)
        init_size = 1024;

    if (init_size > max_size)
        init_size = max_size;

    memset(alloc, 0, sizeof(struct pool_alloc_t));

    alloc->init_size = init_size;
    alloc->max_size = max_size;
}

void pool_alloc_done(
    struct pool_alloc_t* alloc)
{
    struct pool_alloc_node_t *p, *q;

    ASSERT_POOL_ALLOC_INVARIANTS(alloc);

    for (p = alloc->base; p; p = q) {
        ASSERT_POOL_ALLOC_NODE_INVARIANTS(p);
        q = p->next;
        free(p);
    }
}

void* pool_alloc_allocate(
    struct pool_alloc_t* alloc,
    size_t size, size_t align)
{
    struct pool_alloc_node_t* p;
    size_t a, n, r;

    ASSERT(size > 0);
    ASSERT(SIZE_IS_POW2(align));

    ASSERT_POOL_ALLOC_INVARIANTS(alloc);

    VERIFY_SIZE_ADD_NO_OVERFLOW(size, align);
    if (size + align > alloc->max_size)
        INVALID_ARG("%zu", size + align);

    for (p = alloc->base; p; p = p->next) {
        ASSERT_POOL_ALLOC_NODE_INVARIANTS(p);

        a = PTR_ALIGN_SIZE(p->pool + p->used, align);
        ASSERT_SIZE_ADD_NO_OVERFLOW(size, a);
        if (p->size - p->used >= size + a)
            break;
    }

    if (p == NULL) {
        n = alloc->init_size;
        if (n < size + align)
            n = size + align;
        // => n >= size + align

        SIZE_ADD_EQ(n, sizeof(struct pool_alloc_node_t));
        // => n >= size + align

        p = malloc(n);
        ENSURE(p != NULL, "malloc failed");

        VERIFY_SIZE_MUL_NO_OVERFLOW(n, SZ(2));
        alloc->init_size = n * SZ(2);

        if (alloc->init_size > alloc->max_size)
            alloc->init_size = alloc->max_size;

        p->size = n; // => p->size >= size + align
        p->used = 0;

        p->next = alloc->base;
        alloc->base = p;

        a = PTR_ALIGN_SIZE(p->pool, align);
        // stev: have that:
        //   p->size - p->used >= size + a
        // indeed:
        //   p->size - p->used
        //      = p->size       [p->used == 0]
        //     >= size + align  [p->size >= size + align]
        //     >= size + a      [align >= a]
    }

    n = size + a;
    ASSERT(p->size - p->used >= n);

    r = p->used + a;
    p->used += n;

    return p->pool + r;
}

size_t pool_alloc_get_struct_mem(
    const struct pool_alloc_t* alloc)
{
    const struct pool_alloc_node_t* p;
    size_t r;

    ASSERT_POOL_ALLOC_INVARIANTS(alloc);

    for (p = alloc->base; p; p = p->next) {
        ASSERT_POOL_ALLOC_NODE_INVARIANTS(p);

        SIZE_ADD_EQ(r, p->size - p->used);
        SIZE_ADD_EQ(r, sizeof(struct pool_alloc_t));
    }
    SIZE_ADD_EQ(r, sizeof(struct pool_alloc_t));

    return r;
}


