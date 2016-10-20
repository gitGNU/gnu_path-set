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

#ifndef __FLOAT_TRAITS_H
#define __FLOAT_TRAITS_H

#define FLOAT_LT__(x, y) ((x) < (y))
#define FLOAT_LE__(x, y) ((x) <= (y))
#define FLOAT_GT__(x, y) ((x) > (y))
#define FLOAT_GE__(x, y) ((x) >= (y))

#define FLOAT_LT_ABS_DIFF_(x, y) ((y) - (x))
#define FLOAT_GT_ABS_DIFF_(x, y) ((x) - (y))

#define FLOAT_CMP__(op, x, y, e)             \
    (                                        \
        FLOAT_ ## op ## __(x, y) &&          \
        FLOAT_ ## op ## _ABS_DIFF_(x, y) > e \
    )
#define FLOAT_LS_(x, y) FLOAT_CMP__(LT, x, y, FLOAT_EPSILON)
#define FLOAT_LT_       FLOAT_LT__
#define FLOAT_LE_       FLOAT_LE__

#define FLOAT_GS_(x, y) FLOAT_CMP__(GT, x, y, FLOAT_EPSILON)
#define FLOAT_GT_       FLOAT_GT__
#define FLOAT_GE_       FLOAT_GE__

#define FLOAT_CMP_(op, x, y)    \
    (                                 \
        STATIC(TYPEOF_IS(x, float)),  \
        STATIC(TYPEOF_IS(y, float)),  \
        FLOAT_ ## op ## _(x, y) \
    )
#define FLOAT_LS(x, y) FLOAT_CMP_(LS, x, y)
#define FLOAT_LT(x, y) FLOAT_CMP_(LT, x, y)
#define FLOAT_LE(x, y) FLOAT_CMP_(LE, x, y)

#define FLOAT_GS(x, y) FLOAT_CMP_(GS, x, y)
#define FLOAT_GT(x, y) FLOAT_CMP_(GT, x, y)
#define FLOAT_GE(x, y) FLOAT_CMP_(GE, x, y)

#endif // __FLOAT_TRAITS_H


