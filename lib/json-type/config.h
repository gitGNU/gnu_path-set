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

#ifndef CONFIG_H
#define CONFIG_H

#ifndef __GNUC__
#error we need a GCC compiler
#endif

#ifndef __GNUC_MINOR__
#error __GNUC_MINOR__ is not defined
#endif

#ifndef __GNUC_PATCHLEVEL__
#error __GNUC_PATCHLEVEL__ is not defined
#endif

#define GCC_VERSION \
    (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)

//
// CONFIG_MEM_ALIGNOF
//

// stev: CONFIG_MEM_ALIGNOF establishes whether the compiler is
// providing an '__alignof__' builtin function which allows to
// inquire about the minimum alignment required by a type or a
// variable. GCC does have such a builtin: I made only a couple
// of investigations for versions upward from v4.0.0 (look into
// http://gcc.gnu.org/onlinedocs/gcc-4.0.0/gcc/Alignment.html).

// Note that as of GCC v4.8.0 the builtin function is no longer
// needed, since ISO/IEC 9899:2011 specifies the existence of
// the '_Alignof()' unary operator which applied to a 'type-id'
// gets back the alignment requirement of the type in question.
#if GCC_VERSION < 40000
#define CONFIG_MEM_ALIGNOF 0
#else
#define CONFIG_MEM_ALIGNOF 1
#endif

//
// CONFIG_PTR_TO_INT_IDENTOP
//

// stev: CONFIG_PTR_TO_INT_IDENTOP establishes whether the compiler
// implements the conversion from pointer types to integer types of
// equal size as an identity operation. That is that the bits which
// result upon casting a pointer to an integer of the same size are
// identical with those of the pointer representation itself.

// stev: the issues implied by CONFIG_PTR_TO_INT_IDENTOP parameter
// are pertaining to specifications of C language: ISO/IEC 9899:TC3,
// 6.3.2.3 Pointers; and to specifications of C++ language: ISO/IEC
// 14882:2003(E), 5.2.10 Reinterpret cast.

// stev: the implementation of GCC at least upward from v4.0.0 does
// precisely this (see http://gcc.gnu.org/onlinedocs/gcc-4.0.0/gcc/
// Arrays-and-pointers-implementation.html):
//
//   The result of converting a pointer to an integer or vice versa
//   (C90 6.3.4, C99 6.3.2.3):
//
//   A cast from pointer to integer discards most-significant bits
//   if the pointer representation is larger than the integer type,
//   sign-extends [1] if the pointer representation is smaller than
//   the integer type, otherwise the bits are unchanged.
//
//   A cast from integer to pointer discards most-significant bits
//   if the pointer representation is smaller than the integer type,
//   extends according to the signedness of the integer type if the
//   pointer representation is larger than the integer type,
//   otherwise the bits are unchanged.
//
//   Footnotes
//
//   [1] Future versions of GCC may zero-extend, or use a target-
//       defined ptr_extend pattern. Do not rely on sign extension.
//
// Note that I only looked into the GCC documentation of versions
// between v4.0.0 and v4.9.0.
#if GCC_VERSION < 40000 || GCC_VERSION > 40900
#define CONFIG_PTR_TO_INT_IDENTOP 0
#else
#define CONFIG_PTR_TO_INT_IDENTOP 1
#endif

//
// CONFIG_PTR_NULL_ZERO_REPRESENTATION
// CONFIG_PTR_NON_C99_C11_RELATIONAL_COMPARISON
// CONFIG_PTR_OUT_OF_BOUNDS_ARITHMETIC
//

// stev: these CONFIG_* parameters subsume to the issues described,
// for example, by the work of Kayvan Memarian and Peter Sewell, of
// University of Cambridge: 'Clarifying the C memory object model'
// (see http://www.open-std.org/jtc1/sc22/wg14/www/docs/n2012.htm
// for the section on null pointers, the one on pointer relational
// comparison and the one on out-of-bounds pointer arithmetic).
//
// Prior to enabling these CONFIG_* parameters, make sure that the
// things implied by them work on the target platform and compiler.
// These issues are to be of concern since they come orthogonally
// against the present-day C language standards (ISO/IEC 9899:TC3
// and ISO/IEC 9899:2011: e.g. 6.5.6 pt. 8 and 6.5.8 pt. 5).

// Joseph Myers, https://gcc.gnu.org/ml/gcc/2015-04/msg00325.html:
//   > Can null pointers be assumed to be represented with 0?
//   For all targets supported by GCC, yes.
#define CONFIG_PTR_NULL_ZERO_REPRESENTATION          1

#define CONFIG_PTR_NON_C99_C11_RELATIONAL_COMPARISON 0
#define CONFIG_PTR_OUT_OF_BOUNDS_ARITHMETIC          0

#define CONFIG_PTR_NON_STANDARD_OPERATIONS              \
    (                                                   \
        CONFIG_PTR_NON_C99_C11_RELATIONAL_COMPARISON && \
        CONFIG_PTR_OUT_OF_BOUNDS_ARITHMETIC             \
    )

// stev: instead of spreading STATIC(...) assertions throughout
// the source files, in places where the config parameter below
// ensures the well-functioning of the code, we choose to check
// for it being enabled at this global level:

#if !CONFIG_PTR_NULL_ZERO_REPRESENTATION
#error we need the NULL pointer to be represented as 0
#endif

#endif /* CONFIG_H */

