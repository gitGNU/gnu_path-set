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

#include "ptr-traits.h"

#ifndef STACK_NAME
#error  STACK_NAME is not defined
#endif

#ifndef STACK_ELEM_TYPE
#error  STACK_ELEM_TYPE is not defined
#endif

#if defined(STACK_NEED_FIXED_SIZE) && \
    defined(STACK_NEED_MAX_SIZE)
#error cannot have both \
    STACK_NEED_FIXED_SIZE and \
    STACK_NEED_MAX_SIZE defined
#endif

#ifdef  STACK_NEED_FIND_FIRST
#ifndef STACK_ELEM_EQUAL
#define STACK_ELEM_EQUAL(x, y) ((x) == (y))
#endif
#endif

#if  defined(STACK_NEED_PRINT_DEBUG) && \
    !defined(STACK_ELEM_PRINT_DEBUG)
#error STACK_ELEM_PRINT_DEBUG is not defined
#endif

#ifdef STACK_NEED_PRINT_DEBUG
#ifdef STACK_ELEM_PRINT_BY_REF
#define STACK_ELEM_OBJ(p) (p)
#else
#define STACK_ELEM_OBJ(p) (*(p))
#endif
#endif

#define STACK_MAKE_NAME_(n, s) n ## _stack_ ## s
#define STACK_MAKE_NAME(n, s)  STACK_MAKE_NAME_(n, s)

#define STACK_TYPE        STACK_MAKE_NAME(STACK_NAME, t)
#define STACK_INIT        STACK_MAKE_NAME(STACK_NAME, init)
#define STACK_DONE        STACK_MAKE_NAME(STACK_NAME, done)
#define STACK_CREATE      STACK_MAKE_NAME(STACK_NAME, create)
#define STACK_DESTROY     STACK_MAKE_NAME(STACK_NAME, destroy)
#define STACK_ENLARGE     STACK_MAKE_NAME(STACK_NAME, enlarge)
#define STACK_FIND_FIRST  STACK_MAKE_NAME(STACK_NAME, find_first)
#define STACK_PRINT_DEBUG STACK_MAKE_NAME(STACK_NAME, print_debug)

struct STACK_TYPE
{
#ifdef STACK_NEED_MAX_SIZE
    size_t max_size;
#endif
    STACK_ELEM_TYPE* beg;
    STACK_ELEM_TYPE* end;
    STACK_ELEM_TYPE* top;
};

static void STACK_INIT(struct STACK_TYPE* stack,
#ifdef STACK_NEED_MAX_SIZE
    size_t max_size,
#endif
    size_t init_size)
{
    size_t n = sizeof(STACK_ELEM_TYPE);

#ifdef STACK_NEED_MAX_SIZE
    if (max_size == 0)
        max_size = 1024;
    if (init_size == 0 ||
        init_size > max_size)
        init_size = max_size;
#else
    if (init_size == 0)
        init_size = 1024;
#endif

    ASSERT_SIZE_MUL_NO_OVERFLOW(n, init_size);
    n *= init_size;

    stack->beg = malloc(n);
    ENSURE(stack->beg != NULL, "malloc failed");

    memset(stack->beg, 0, n);

    // stev: invariants of stack:
    // (beg < end) &&
    // /* top is inside stack or at the end of it */
    // (top >= beg) && (top <= end)

#ifdef STACK_NEED_MAX_SIZE
    stack->max_size = max_size;
#endif
    stack->end = stack->beg + init_size;
    stack->top = stack->beg;
}

static void STACK_DONE(struct STACK_TYPE* stack)
{
    free(stack->beg);
}

#define STACK_ASSERT_INVARIANTS(stack)        \
    do {                                      \
        ASSERT((stack)->beg < (stack)->end);  \
        ASSERT((stack)->top <= (stack)->end); \
        ASSERT((stack)->top >= (stack)->beg); \
    } while (0)

#ifdef STACK_NEED_CREATE_DESTROY

static struct STACK_TYPE* STACK_CREATE(
#ifdef STACK_NEED_MAX_SIZE
    size_t max_size,
#endif
    size_t init_size)
{
    struct STACK_TYPE* stack;

    stack = malloc(sizeof(struct STACK_TYPE));
    ENSURE(stack != NULL, "malloc failed");

    STACK_INIT(
        stack,
#ifdef STACK_NEED_MAX_SIZE
        max_size,
#endif
        init_size);

    return stack;
}

static void STACK_DESTROY(struct STACK_TYPE* stack)
{
    STACK_DONE(stack);
    free(stack);
}

#endif // STACK_NEED_CREATE_DESTROY

#ifndef STACK_NEED_FIXED_SIZE

#ifndef STACK_ENSURE_INPLACE_ENLARGE
#undef  STACK_REALLOC
#define STACK_REALLOC(p, n)    \
    ({                         \
        void* __r;             \
        ASSERT(p != NULL);     \
        __r = realloc(p, n);   \
        ENSURE(__r != NULL,    \
            "realloc failed"); \
        __r;                   \
    })
#else // STACK_ENSURE_INPLACE_ENLARGE
#undef  STACK_REALLOC
#define STACK_REALLOC(p, n)            \
    ({                                 \
        void* __r;                     \
        ASSERT(p != NULL);             \
        __r = realloc(p, n);           \
        ENSURE(__r == p,               \
            "inplace realloc failed"); \
        __r;                           \
    })
#endif // STACK_ENSURE_INPLACE_ENLARGE

static void STACK_ENLARGE(struct STACK_TYPE* stack)
{
    const size_t sz = sizeof(STACK_ELEM_TYPE);
    size_t n, t;

    STACK_ASSERT_INVARIANTS(stack);

    n = PTR_DIFF(stack->end, stack->beg);
    t = PTR_DIFF(stack->top, stack->beg);

    ASSERT(n > 0);
    ASSERT(t <= n);
#ifdef STACK_NEED_MAX_SIZE
    ASSERT(stack->max_size > 0);
    ASSERT(n <= stack->max_size);
    ENSURE(n < stack->max_size,
        "stack reached maximum size");
#endif

    ASSERT_SIZE_MUL_NO_OVERFLOW(n, SZ(2));
    n *= SZ(2);

#ifdef STACK_NEED_MAX_SIZE
    // stev: let 's' be the value of 'n' before
    // it was scaled by 2: thus 's' is the current
    // stack size; the assertions above imply:
    //   0 < s < max_size
    if (n > stack->max_size)
        n = stack->max_size;
    // then:
    //   (a) 2 * s >  max_size => s < n == max_size
    //   (b) 2 * s <= max_size => s < n == 2 * s <= max_size
    // hence either way:
    //   s < n <= max_size

    // stev: the double inequality above shows that
    // the new size 'n' will increase strictly over
    // the current size 's', while not exceeding the
    // upper-bound 'max_size'!
#endif

    ASSERT_SIZE_MUL_NO_OVERFLOW(n, sz);
    stack->beg = STACK_REALLOC(stack->beg, n * sz);

    stack->top = stack->beg + t;
    stack->end = stack->beg + n;
}

#undef  STACK_PUSH_ENSURE_SIZE
#define STACK_PUSH_ENSURE_SIZE(stack)        \
    do {                                     \
        if ((stack)->top == (stack)->end)    \
            STACK_ENLARGE(stack);            \
        ASSERT((stack)->top < (stack)->end); \
    } while (0)
#else  // STACK_NEED_FIXED_SIZE
#undef  STACK_PUSH_ENSURE_SIZE
#define STACK_PUSH_ENSURE_SIZE(stack)   \
    ENSURE((stack)->top < (stack)->end, \
        "stack overflow")
#endif // STACK_NEED_FIXED_SIZE

#ifdef STACK_NEED_FIND_FIRST

static bool STACK_FIND_FIRST(struct STACK_TYPE* stack,
    const STACK_ELEM_TYPE* elem, size_t* pos)
{
    STACK_ELEM_TYPE* ptr;

    STACK_ASSERT_INVARIANTS(stack);

    for (ptr = stack->top; ptr > stack->beg; ) {
        ptr --;
        // stev: decremented 'ptr' separately due to
        // the fact that STACK_ELEM_EQUAL is not in
        // our control -- our user supplies it to us
        if (STACK_ELEM_EQUAL(ptr, elem)) {
            *pos = PTR_DIFF(stack->top - 1, ptr);
            return true;
        }
    }

    return false;
}

#endif // STACK_NEED_FIND_FIRST

#ifdef STACK_NEED_PRINT_DEBUG

static void STACK_PRINT_DEBUG(struct STACK_TYPE* stack)
{
    STACK_ELEM_TYPE* ptr;

    STACK_ASSERT_INVARIANTS(stack);

    print_debug_fmt("{.beg=%p .end=%p .top=%p} [",
        stack->beg, stack->end, stack->top);

    for (ptr = stack->beg; ptr < stack->top; ptr ++) {
        if (ptr > stack->beg)
            PRINT_DEBUG_STR(",");
        STACK_ELEM_PRINT_DEBUG(STACK_ELEM_OBJ(ptr));
    }

    print_debug_str("]");
}

#endif // STACK_NEED_PRINT_DEBUG

#ifndef __STACK_IMPL_H
#define __STACK_IMPL_H

#define TYPEOF_IS_STACK(x)               \
    TYPEOF_IS(x, struct STACK_TYPE*)

#define STACK_ENSURE_NOT_EMPTY(stack)    \
    ENSURE((stack)->top > (stack)->beg,  \
        "stack is empty")

#define STACK_ENSURE_SIZE(stack)         \
    ENSURE((stack)->top > (stack)->beg,  \
        "stack underflow")

#define STACK_ENSURE_SIZE_N(stack, n)    \
    do {                                 \
        ASSERT_SIZE_INC_NO_OVERFLOW(n);  \
        ENSURE(PTR_DIFF(                 \
            (stack)->top,                \
            (stack)->beg) >= (n) + 1,    \
            "stack underflow");          \
    } while (0)

#define STACK_IS_INIT(stack)             \
    (                                    \
        STATIC(TYPEOF_IS_STACK(stack)),  \
        (stack)->beg != NULL             \
    )

#define STACK_IS_EMPTY(stack)            \
    ({                                   \
        STATIC(TYPEOF_IS_STACK(stack));  \
        STACK_ASSERT_INVARIANTS(stack);  \
        (stack)->top == (stack)->beg;    \
    })

#define STACK_CLEAR(stack)               \
    do {                                 \
        STATIC(TYPEOF_IS_STACK(stack));  \
        STACK_ASSERT_INVARIANTS(stack);  \
        (stack)->top = (stack)->beg;     \
    } while (0)

#define STACK_SIZE(stack)                \
    ({                                   \
        STATIC(TYPEOF_IS_STACK(stack));  \
        STACK_ASSERT_INVARIANTS(stack);  \
        PTR_DIFF(                        \
            (stack)->top,                \
            (stack)->beg);               \
    })

#define STACK_PUSH(stack, v)             \
    do {                                 \
        STATIC(TYPEOF_IS_STACK(stack));  \
        STACK_ASSERT_INVARIANTS(stack);  \
        STACK_PUSH_ENSURE_SIZE(stack);   \
        *(stack)->top ++ = (v);          \
    } while (0)

#define STACK_POP(stack)                 \
    ({                                   \
        STATIC(TYPEOF_IS_STACK(stack));  \
        STACK_ASSERT_INVARIANTS(stack);  \
        STACK_ENSURE_SIZE(stack);        \
        *-- (stack)->top;                \
    })

#define STACK_TOP(stack)                 \
    ({                                   \
        STATIC(TYPEOF_IS_STACK(stack));  \
        STACK_ASSERT_INVARIANTS(stack);  \
        STACK_ENSURE_NOT_EMPTY(stack);   \
        (stack)->top[-1];                \
    })

#define STACK_BEG_REF(stack)             \
    ({                                   \
        STATIC(TYPEOF_IS_STACK(stack));  \
        STACK_ASSERT_INVARIANTS(stack);  \
        STACK_ENSURE_NOT_EMPTY(stack);   \
        (stack)->beg;                    \
    })

#define STACK_TOP_REF(stack)             \
    ({                                   \
        STATIC(TYPEOF_IS_STACK(stack));  \
        STACK_ASSERT_INVARIANTS(stack);  \
        STACK_ENSURE_NOT_EMPTY(stack);   \
        (stack)->top - 1;                \
    })

#define STACK_TOP_N(stack, n)            \
    ({                                   \
        STATIC(TYPEOF_IS_SIZET(n));      \
        STATIC(TYPEOF_IS_STACK(stack));  \
        STACK_ASSERT_INVARIANTS(stack);  \
        STACK_ENSURE_SIZE_N(stack, n);   \
        *((stack)->top - ((n) + 1));     \
    })

// stev: note that POP_N(n) means popping 'n + 1' elements
// off the stack; this eventually imply emptying the stack:
// top - beg >= n + 1 <=> top - (n + 1) >= beg

#define STACK_POP_N(stack, n)            \
    do {                                 \
        STATIC(TYPEOF_IS_SIZET(n));      \
        STATIC(TYPEOF_IS_STACK(stack));  \
        STACK_ASSERT_INVARIANTS(stack);  \
        STACK_ENSURE_SIZE_N(stack, n);   \
        (stack)->top -= (n) + 1;         \
    } while (0)

#define STACK_SHIFT_TOP(stack, f, t)     \
    do {                                 \
        STATIC(TYPEOF_IS_STACK(stack));  \
        STACK_ASSERT_INVARIANTS(stack);  \
        STACK_ENSURE_NOT_EMPTY(stack);   \
        ASSERT((stack)->top[-1] == (f)); \
        (stack)->top[-1] = (t);          \
    } while (0)

#endif/*__STACK_IMPL_H*/


