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

#ifdef  OBJ_ALLOC_OBJ_SIZE

#ifndef OBJ_ALLOC_OBJ_ALIGN
#error  OBJ_ALLOC_OBJ_ALIGN not defined
#endif

#endif // OBJ_ALLOC_OBJ_SIZE

#ifndef OBJ_ALLOC_NODE_BITS
#error  OBJ_ALLOC_NODE_BITS not defined
#endif

#ifndef OBJ_ALLOC_NAME
#define OBJ_ALLOC_NAME obj
#endif

#define OBJ_ALLOC_MAKE_NAME__(n, s) n ## _alloc ## s
#define OBJ_ALLOC_MAKE_NAME_(n, s)  OBJ_ALLOC_MAKE_NAME__(n, s)
#define OBJ_ALLOC_MAKE_NAME(s)      OBJ_ALLOC_MAKE_NAME_(OBJ_ALLOC_NAME, _ ## s)

#define OBJ_ALLOC_TYPE      OBJ_ALLOC_MAKE_NAME(t)
#define OBJ_ALLOC_NODE_TYPE OBJ_ALLOC_MAKE_NAME(node_t)

#define OBJ_ALLOC_INIT      OBJ_ALLOC_MAKE_NAME(init)
#define OBJ_ALLOC_DONE      OBJ_ALLOC_MAKE_NAME(done)

#define OBJ_ALLOC_ALLOCATE  OBJ_ALLOC_MAKE_NAME(allocate)
#define OBJ_ALLOC_GET_OBJ_COUNT \
                            OBJ_ALLOC_MAKE_NAME(get_obj_count)
#define OBJ_ALLOC_GET_OBJ_MEM \
                            OBJ_ALLOC_MAKE_NAME(get_obj_mem)
#define OBJ_ALLOC_GET_STRUCT_MEM \
                            OBJ_ALLOC_MAKE_NAME(get_struct_mem)

// stev: TODO: promote 'U32' to a global context
#if UINT32_MAX == UINT_MAX
#define U32(x) x ## U
#elif UINT32_MAX == ULONG_MAX
#define U32(x) x ## UL
#else
#error uint32_t is neither unsigned int nor unsigned long
#endif

#define OBJ_ALLOC_NODE_SIZE   (U32(1) << OBJ_ALLOC_NODE_BITS)
#define OBJ_ALLOC_OFFSET_BITS (U32(32) - OBJ_ALLOC_NODE_BITS)

#if OBJ_ALLOC_NODE_BITS > 0
#define OBJ_ALLOC_OFFSET_MAX  ((U32(1) << OBJ_ALLOC_OFFSET_BITS) - U32(1))
#define OBJ_ALLOC_NODE_MASK   ((OBJ_ALLOC_NODE_SIZE - U32(1)) << \
                                OBJ_ALLOC_OFFSET_BITS)
#else // OBJ_ALLOC_NODE_BITS
#define OBJ_ALLOC_OFFSET_MAX  UINT32_MAX
#define OBJ_ALLOC_NODE_MASK   U32(0)
#endif // OBJ_ALLOC_NODE_BITS

#define OBJ_ALLOC_OFFSET_MASK OBJ_ALLOC_OFFSET_MAX

// stev: note that the semantics of 'size' and 'used'
// in the struct 'OBJ_ALLOC_NODE_TYPE' below depends
// on 'OBJ_ALLOC_OBJ_SIZE' being defined or not:
//
// * when defined: 'size' is the number of *objects*
//   of specified size 'OBJ_ALLOC_OBJ_SIZE' currently
//   claimed from the free store; 'used' equals the
//   number of *objects* allocated by our user through
//   the function 'OBJ_ALLOC_ALLOCATE'.
//
// * when not defined: 'size' is the amount of *bytes*
//   of memory currently claimed from the free store;
//   'used' equals the amount of *bytes* allocated by
//   our user via the function 'OBJ_ALLOC_ALLOCATE'.

struct OBJ_ALLOC_NODE_TYPE
{
#ifdef OBJ_ALLOC_OBJ_SIZE
    char (*base)[OBJ_ALLOC_OBJ_SIZE];
#else
    char  *base;
#endif
    size_t size;
    size_t used;
};

struct OBJ_ALLOC_TYPE
{
    struct OBJ_ALLOC_NODE_TYPE nodes[
        OBJ_ALLOC_NODE_SIZE];
    size_t init_size;
    size_t max_size;
};

#define OBJ_ALLOC_ASSERT_INVARIANTS(a, n)   \
    do {                                    \
        ASSERT((n)->size <= (a)->max_size); \
        ASSERT((n)->used <= (n)->size);     \
        ASSERT((n)->base || !(n)->used);    \
    } while (0)

static void OBJ_ALLOC_INIT(
    struct OBJ_ALLOC_TYPE* alloc,
    size_t init_size, size_t max_size)
{
    // stev: the STATIC assertions below
    // ensure logical consistency of the
    // OBJ_ALLOC_* parameters -- some of
    // which are defined by our user; we
    // do verify them only once in this
    // function!

    STATIC(SIZE_BIT >= 32);

#ifdef OBJ_ALLOC_OBJ_SIZE
    STATIC(TYPEOF_IS_SIZET(OBJ_ALLOC_OBJ_ALIGN));
    STATIC(TYPEOF_IS_SIZET(OBJ_ALLOC_OBJ_SIZE));
    STATIC(SIZE_IS_POW2(OBJ_ALLOC_OBJ_ALIGN));
    STATIC(OBJ_ALLOC_OBJ_SIZE > 0);
#endif

    STATIC(OBJ_ALLOC_NODE_BITS <= 4);
    STATIC((OBJ_ALLOC_NODE_MASK | OBJ_ALLOC_OFFSET_MASK)
        == UINT32_MAX);

#if OBJ_ALLOC_NODE_BITS == 0
    STATIC(OBJ_ALLOC_NODE_SIZE == 1);
    STATIC(OBJ_ALLOC_NODE_MASK == 0);
    STATIC(OBJ_ALLOC_OFFSET_BITS == 32);
    STATIC(OBJ_ALLOC_OFFSET_MAX == UINT32_MAX);
    STATIC(OBJ_ALLOC_OFFSET_MASK == UINT32_MAX);
#endif

    if (init_size > OBJ_ALLOC_OFFSET_MAX)
        INVALID_ARG("%zu", init_size);
    if (max_size > OBJ_ALLOC_OFFSET_MAX)
        INVALID_ARG("%zu", max_size);

    if (max_size == 0)
        max_size = OBJ_ALLOC_OFFSET_MAX;

    if (init_size == 0 ||
        init_size > max_size)
        init_size = max_size;

    memset(alloc, 0, sizeof(*alloc));

    alloc->init_size = init_size;
    alloc->max_size = max_size;
}

static void OBJ_ALLOC_DONE(
    struct OBJ_ALLOC_TYPE* alloc)
{
    const struct OBJ_ALLOC_NODE_TYPE *p, *e;

    for (p = alloc->nodes,
         e = p + OBJ_ALLOC_NODE_SIZE;
         p < e;
         p ++) {
        OBJ_ALLOC_ASSERT_INVARIANTS(
            alloc, p);
        if (p->base)
            free(p->base);
    }
}

static void* OBJ_ALLOC_ALLOCATE(
    struct OBJ_ALLOC_TYPE* alloc,
#ifndef OBJ_ALLOC_OBJ_SIZE
    size_t size, size_t align,
#endif
    uint32_t* result)
{
    struct OBJ_ALLOC_NODE_TYPE* p;
#ifndef OBJ_ALLOC_OBJ_SIZE
    bool f = false;
    size_t a, n;
#endif
    uint32_t r;
    size_t i;

#ifndef OBJ_ALLOC_OBJ_SIZE
    ASSERT(size > 0);
    ASSERT(SIZE_IS_POW2(align));
#endif

    // stev: expecting the compiler to unroll this loop!
    for (i = 0; i < OBJ_ALLOC_NODE_SIZE; i ++) {
        p = &alloc->nodes[i];

        OBJ_ALLOC_ASSERT_INVARIANTS(alloc, p);

        if (p->base == NULL) {
#ifndef OBJ_ALLOC_OBJ_SIZE
            a = align;
            f = true;
#endif
            break;
        }

#ifdef OBJ_ALLOC_OBJ_SIZE
        if (p->used >= OBJ_ALLOC_OFFSET_MAX)
            continue;

        if (p->size > p->used ||
            p->size < alloc->max_size)
            break;
#else
        a = PTR_ALIGN_SIZE(p->base + p->used, align);

        ASSERT_SIZE_ADD_NO_OVERFLOW(p->used, a);
        if (p->used + a >= OBJ_ALLOC_OFFSET_MAX)
            continue;

        ASSERT_SIZE_ADD_NO_OVERFLOW(size, a);
        if (p->size - p->used >= size + a ||
            p->size < alloc->max_size)
            break;
#endif
    }

    if (i >= OBJ_ALLOC_NODE_SIZE)
        return NULL;

    // stev: redundant ASSERT(COND) below since
    // we have an 'if (!COND) continue' statement
    // in the 'for' loop above
#ifdef OBJ_ALLOC_OBJ_SIZE
    ASSERT(p->used < OBJ_ALLOC_OFFSET_MAX);
#else
    ASSERT(p->used + a < OBJ_ALLOC_OFFSET_MAX);

    n = size + a;
#endif

#ifdef OBJ_ALLOC_OBJ_SIZE
    // p->size == p->used =>
    // p->base == NULL || p->size < alloc->max_size

    // p->base == NULL => p->size == 0, thus:
    // p->size == p->used => p->size < alloc->max_size

    if (p->size == p->used) {
#else
    // p->size - p->used < n =>
    // p->base == NULL || p->size < alloc->max_size

    // p->base == NULL => p->size == 0, thus:
    // p->size - p->used < n => p->size < alloc->max_size

    if (p->size - p->used < n) {
#endif
        size_t s = p->size;

        if (s == 0) {
            s = alloc->init_size;
            // => p->size = 0 < s
        }
#ifdef OBJ_ALLOC_OBJ_SIZE
        else {
            SIZE_MUL_EQ(s, SZ(2));
            // => p->size < 2 * p->size = s
        }
        // in both cases have that: p->size < s
#else
        ASSERT(s > 0);
        while (SIZE_SUB(s, p->used) < n)
            SIZE_MUL_EQ(s, SZ(2));
        // => s - p->used >= n

        // also have that p->size < s:
        //   case 1: p->size = 0 =>
        //           p->size = 0 < 2^k * p->init_size = s,
        //           for some k >= 0
        //   case 2: p->size > 0 =>
        //           p->size < 2^k * p->size = s,
        //           for some k >= 1
#endif

        // => p->size < s
        // => p->size < min{ s, alloc->max_size }
        if (s > alloc->max_size)
            s = alloc->max_size;
        // => p->size < s <= alloc->max_size

#ifndef OBJ_ALLOC_OBJ_SIZE
        p->base = realloc(p->base, s);
#else
        ASSERT_SIZE_MUL_NO_OVERFLOW(s, OBJ_ALLOC_OBJ_SIZE);
        p->base = realloc(p->base, s * OBJ_ALLOC_OBJ_SIZE);
#endif
        ENSURE(p->base != NULL, "realloc failed");

#ifdef OBJ_ALLOC_OBJ_SIZE
        ENSURE(PTR_ALIGN_SIZE(
            (void*) p->base, OBJ_ALLOC_OBJ_ALIGN) == 0,
            "realloc alignment failed");
#endif

        p->size = s;
        // => p->size <= alloc->max_size and
        //    p->size increased strictly

#ifdef OBJ_ALLOC_OBJ_SIZE
        // => p->used < p->size
        ASSERT(p->used < p->size);
#else
        // => p->size - p->used >= n
        ASSERT(p->size - p->used >= n);
#endif
    }

#ifndef OBJ_ALLOC_OBJ_SIZE
    if (f) {
        a = PTR_ALIGN_SIZE(p->base + p->used, align);
        ASSERT(a <= align);
    }
    r = p->used + a;
    p->used += n;
    memset(p->base + r, 0, size);
#else
    r = p->used ++;
    memset(p->base + r, 0, OBJ_ALLOC_OBJ_SIZE); 
#endif

#if OBJ_ALLOC_NODE_BITS > 0
    STATIC(OBJ_ALLOC_OFFSET_BITS < SZ(32));
    *result = (i << OBJ_ALLOC_OFFSET_BITS) | (r + 1);
#else
    STATIC(OBJ_ALLOC_NODE_SIZE == 1);
    // => i == 0
    STATIC(OBJ_ALLOC_OFFSET_BITS == 32);
    // => (i << OBJ_ALLOC_OFFSET_BITS) == 0
    *result = r + 1;
#endif

    return p->base + r;
}

#define OBJ_ALLOC_TYPEOF_IS_SELF(a)                \
    (                                              \
        TYPEOF_IS(a, struct OBJ_ALLOC_TYPE*) ||    \
        TYPEOF_IS(a, struct OBJ_ALLOC_TYPE const*) \
    )

// stev: the compiler optimizing the case of
// 'OBJ_ALLOC_NODE_BITS == 0' might render the
// special 'OBJ_ALLOC_DEREF' below useless!

#if OBJ_ALLOC_NODE_BITS == 0
#define OBJ_ALLOC_DEREF(a, r)                      \
    ({                                             \
        void* __r;                                 \
                                                   \
        STATIC(OBJ_ALLOC_TYPEOF_IS_SELF(a));       \
        STATIC(TYPEOF_IS(r, uint32_t));            \
                                                   \
        if ((r) == 0)                              \
            __r = NULL;                            \
        else {                                     \
            const struct OBJ_ALLOC_NODE_TYPE* __p; \
                                                   \
            __p = &(a)->nodes[0];                  \
            OBJ_ALLOC_ASSERT_INVARIANTS(a, __p);   \
                                                   \
            ASSERT((r) > 0);                       \
            ASSERT((r) <= __p->used);              \
            ASSERT(__p->base != NULL);             \
            __r = __p->base + ((r) - 1);           \
        }                                          \
        __r;                                       \
    })
#else // OBJ_ALLOC_NODE_BITS == 0
#define OBJ_ALLOC_DEREF(a, r)                      \
    ({                                             \
        void* __r;                                 \
                                                   \
        STATIC(OBJ_ALLOC_TYPEOF_IS_SELF(a));       \
        STATIC(TYPEOF_IS(r, uint32_t));            \
                                                   \
        if ((r) == 0)                              \
            __r = NULL;                            \
        else {                                     \
            const struct OBJ_ALLOC_NODE_TYPE* __p; \
            uint32_t __i, __o;                     \
                                                   \
            __o = ((r) & OBJ_ALLOC_OFFSET_MASK);   \
            __i = ((r) & OBJ_ALLOC_NODE_MASK) >>   \
                OBJ_ALLOC_OFFSET_BITS;             \
                                                   \
            ASSERT(__i < OBJ_ALLOC_NODE_SIZE);     \
            __p = &(a)->nodes[__i];                \
            OBJ_ALLOC_ASSERT_INVARIANTS(a, __p);   \
                                                   \
            ASSERT(__o > 0);                       \
            ASSERT(__o <= __p->used);              \
            ASSERT(__p->base != NULL);             \
            __r = __p->base + (__o - 1);           \
        }                                          \
        __r;                                       \
    })
#endif // OBJ_ALLOC_NODE_BITS == 0

#if defined(OBJ_ALLOC_OBJ_SIZE) && \
    defined(OBJ_ALLOC_NEED_GET_OBJ_COUNT) || \
    !defined(OBJ_ALLOC_OBJ_SIZE) && \
    defined(OBJ_ALLOC_NEED_GET_OBJ_MEM)

static size_t
#ifdef OBJ_ALLOC_OBJ_SIZE
    OBJ_ALLOC_GET_OBJ_COUNT(
#else
    OBJ_ALLOC_GET_OBJ_MEM(
#endif
        const struct OBJ_ALLOC_TYPE* alloc)
{
    const struct OBJ_ALLOC_NODE_TYPE *p, *e;
    size_t r = 0;

    for (p = alloc->nodes,
         e = p + OBJ_ALLOC_NODE_SIZE;
         p < e;
         p ++) {
        OBJ_ALLOC_ASSERT_INVARIANTS(
            alloc, p);
        SIZE_ADD_EQ(r, p->used);
    }

    return r;
}

#endif // defined(OBJ_ALLOC_OBJ_SIZE) &&
       // defined(OBJ_ALLOC_NEED_GET_OBJ_COUNT) ||
       // !defined(OBJ_ALLOC_OBJ_SIZE) &&
       // defined(OBJ_ALLOC_NEED_GET_OBJ_MEM)

static size_t
    OBJ_ALLOC_GET_STRUCT_MEM(
        const struct OBJ_ALLOC_TYPE* alloc)
{
    const size_t n = sizeof(struct OBJ_ALLOC_TYPE);
    const struct OBJ_ALLOC_NODE_TYPE *p, *e;
    size_t r = 0;

    for (p = alloc->nodes,
         e = p + OBJ_ALLOC_NODE_SIZE;
         p < e;
         p ++) {
        OBJ_ALLOC_ASSERT_INVARIANTS(
            alloc, p);
        SIZE_ADD_EQ(r, p->size);
        SIZE_SUB_EQ(r, p->used);
    }
#ifdef OBJ_ALLOC_OBJ_SIZE
    SIZE_MUL_EQ(r, OBJ_ALLOC_OBJ_SIZE);
#endif
    SIZE_ADD_EQ(r, n);

    return r;
}


