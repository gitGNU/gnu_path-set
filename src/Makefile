# Copyright (C) 2016  Stefan Vargyas
# 
# This file is part of Path-Set.
# 
# Path-Set is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# Path-Set is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with Path-Set.  If not, see <http://www.gnu.org/licenses/>.

.PHONY: default clean allclean all depend cpp-only path-set

default: all

GCC := gcc
GCC_STD := gnu99
CFLAGS := -Wall -Wextra -std=${GCC_STD} \
          -I. -I../lib/json-type -I../lib/gnulib \
          -DPROGRAM=path-set

SRCS := main.c \
        common.c \
        clocks.c \
        set-intf.c \
        pool-alloc.c \
        trie-path-set.c \
        lhash-path-set.c \
        ghash-path-set.c \
        trie-plain-set.c \
        lhash-plain-set.c \
        ghash-plain-set.c \
        ../lib/gnulib/hash.c

BIN  := path-set

CFGS := PATH_TRIE_ELEM_32BIT_OFFSETS|PATH_TRIE_NODE_32BIT_OFFSETS|PATH_TRIE_UINT_HASH_IDENTOP|TRIE_PATH_SET_PRINT_SET|VALUE_TYPE_SIZE=(8|16|32|64)

ifdef CONFIG
CONFIG_CHECK = $(shell \
	bash -c 'tr -s " \t" "\n" <<< "${CONFIG}"|sed -nr "/^(${CFGS})$$/!{s/^/invalid config parameter: \x22/;s/$$/\x22/;p;q}"' 2>&1)
ifneq (${CONFIG_CHECK},)
$(error ${CONFIG_CHECK})
endif
endif

ifdef CONFIG
CONFIG_CHECK = $(shell \
	bash -c 'tr -s " \t" "\n" <<< "${CONFIG}"|sed -r "s/=.+$$//"|sort|uniq -d|sed -nr "1{s/^/duplicated config parameter: \x22/;s/$$/\x22/;p;q}"' 2>&1)
ifneq (${CONFIG_CHECK},)
$(error ${CONFIG_CHECK})
endif
endif

ifdef CONFIG
CFLAGS += $(addprefix -DCONFIG_, ${CONFIG})
endif

ifdef DEBUG
ifneq (${DEBUG},no)
ifneq (${DEBUG},yes)
$(error invalid DEBUG=${DEBUG})
endif
endif
endif

ifneq (${DEBUG},no)
CFLAGS += -DDEBUG -g
endif

ifdef OPT
ifeq ($(filter ${OPT}, 0 1 2 3 4),)
$(error invalid OPT=${OPT})
endif
endif

ifdef OPT
CFLAGS += -DOPT=${OPT} -O${OPT}
endif

ifdef 32BIT
ifneq (${32BIT},no)
ifneq (${32BIT},yes)
$(error invalid 32BIT=${32BIT})
endif
endif
endif

ifeq (${32BIT},yes)
CFLAGS += -DBIT32 -m32
endif

ifdef CPPS
ifneq (${CPPS},all)
ifeq ($(filter ${CPPS},${SRCS}),)
$(error invalid CPPS=${CPPS})
endif
endif
endif

ifdef CPPS
default: cpp-only
ifneq (${CPPS},all)
SRCS := ${CPPS}
endif
endif

${BIN}: ${SRCS}

# building rules

ifndef CPPS
${BIN}:
	${GCC} ${CFLAGS} ${SRCS} -o $@ -lm
endif

ifdef CPPS
cpp-only:
	${GCC} ${CFLAGS} ${SRCS} -E -o - || true
endif

# main targets

all: ${BIN}

clean:
	rm -f *.o

allclean: clean
	rm -f *~ ${BIN}


