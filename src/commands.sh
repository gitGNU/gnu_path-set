#!/bin/bash

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

usage()
{
    echo "usage: $1 [$(sed 's/^://;s/-:$/\x0/;s/[^:]/|-\0/g;s/:/ <arg>/g;s/^|//;s/\x0/-<long>/' <<< "$2")]"
}

quote()
{
    local __n__
    local __v__

    [ -z "$1" -o "$1" == "__n__" -o "$1" == "__v__" ] &&
    return 1

    printf -v __n__ '%q' "$1"
    eval __v__="\"\$$__n__\""
    #!!! echo "!!! 0 __v__='$__v__'"
    test -z "$__v__" && return 0
    printf -v __v__ '%q' "$__v__"
    #!!! echo "!!! 1 __v__='$__v__'"
    printf -v __v__ '%q' "$__v__"  # double quote
    #!!! echo "!!! 2 __v__='$__v__'"
    test -z "$SHELL_BASH_QUOTE_TILDE" &&
    __v__="${__v__//\~/\\~}"
    eval "$__n__=$__v__"
}

quote2()
{
    local __n__
    local __v__

    local __q__='q'
    [ "$1" == '-i' ] && {
        __q__=''
        shift
    }

    [ -z "$1" -o "$1" == "__n__" -o "$1" == "__v__" -o "$1" == "__q__" ] &&
    return 1

    printf -v __n__ '%q' "$1"
    eval __v__="\"\$$__n__\""
    __v__="$(sed -nr '
        H
        $!b
        g
        s/^\n//
        s/\x27/\0\\\0\0/g'${__q__:+'
        s/^/\x27/
        s/$/\x27/'}'
        p
    ' <<< "$__v__")"
    test -n "$__v__" &&
    printf -v __v__ '%q' "$__v__"
    eval "$__n__=$__v__"
}

assign2()
{
    local __n__
    local __v__

    [ -z "$1" -o "$1" == "__n__" -o "$1" == "__v__" ] && return 1
    [ -z "$2" -o "$2" == "__n__" -o "$2" == "__v__" ] && return 1
    printf -v __n__ '%q' "$2"
    eval __v__="\"\$$__n__\""
    test -n "$__v__" &&
    printf -v __v__ '%q' "$__v__"
    printf -v __n__ '%q' "$1"
    test -z "$SHELL_BASH_QUOTE_TILDE" &&
    __v__="${__v__//\~/\\~}"
    eval "$__n__=$__v__"
}

optopt()
{
    local __n__="${1:-$opt}"       #!!!NONLOCAL
    local __v__=''
    test -n "$__n__" &&
    printf -v __v__ '%q' "$__n__"  # paranoia
    test -z "$SHELL_BASH_QUOTE_TILDE" &&
    __v__="${__v__//\~/\\~}"
    eval "$__n__=$__v__"
}

optarg()
{
    local __n__="${1:-$opt}"       #!!!NONLOCAL
    local __v__=''
    test -n "$OPTARG" &&
    printf -v __v__ '%q' "$OPTARG" #!!!NONLOCAL
    test -z "$SHELL_BASH_QUOTE_TILDE" &&
    __v__="${__v__//\~/\\~}"
    eval "$__n__=$__v__"
}

optact()
{
    local __v__="${1:-$opt}"       #!!!NONLOCAL
    printf -v __v__ '%q' "$__v__"  # paranoia
    test -z "$SHELL_BASH_QUOTE_TILDE" &&
    __v__="${__v__//\~/\\~}"
    eval "act=$__v__"
}

optactarg()
{
    optact ${1:+"$1"}
    local __v__=''
    test -n "$OPTARG" &&
    printf -v __v__ '%q' "$OPTARG" #!!!NONLOCAL
    test -z "$SHELL_BASH_QUOTE_TILDE" &&
    __v__="${__v__//\~/\\~}"
    eval "arg=$__v__"
}

optlong()
{
    local a="$1"

    if [ "$a" == '-' ]; then
        if [ -z "$OPT" ]; then                                      #!!!NONLOCAL
            local A="${OPTARG%%=*}"                                 #!!!NONLOCAL
            OPT="-$opt$A"                                           #!!!NONLOCAL
            OPTN="${OPTARG:$((${#A})):1}"                           #!!!NONLOCAL
            OPTARG="${OPTARG:$((${#A} + 1))}"                       #!!!NONLOCAL
        else
            OPT="--$OPT"                                            #!!!NONLOCAL
        fi
    elif [ "$opt" == '-' -o \( -n "$a" -a -z "$OPT" \) ]; then      #!!!NONLOCAL
        OPT="${OPTARG%%=*}"                                         #!!!NONLOCAL
        OPTN="${OPTARG:$((${#OPT})):1}"                             #!!!NONLOCAL
        OPTARG="${OPTARG:$((${#OPT} + 1))}"                         #!!!NONLOCAL
        [ -n "$a" ] && OPT="$a-$OPT"                                #!!!NONLOCAL
    elif [ -z "$a" ]; then                                          #!!!NONLOCAL
        OPT=''                                                      #!!!NONLOCAL
        OPTN=''                                                     #!!!NONLOCAL
    fi
}

optlongchkarg()
{
    test -z "$OPT" &&                               #!!!NONLOCAL
    return 0

    [[ "$opt" == [a-zA-Z] ]] || {                   #!!!NONLOCAL
        error "internal: invalid opt name '$opt'"   #!!!NONLOCAL
        return 1
    }

    local r="^:[^$opt]*$opt(.)"
    [[ "$opts" =~ $r ]]
    local m="$?"

    local s
    if [ "$m" -eq 0 ]; then
        s="${BASH_REMATCH[1]}"
    elif [ "$m" -eq 1 ]; then
        error "internal: opt '$opt' not in '$opts'" #!!!NONLOCAL
        return 1
    elif [ "$m" -eq "2" ]; then
        error "internal: invalid regex: $r"
        return 1
    else
        error "internal: unexpected regex match result: $m: ${BASH_REMATCH[@]}"
        return 1
    fi

    if [ "$s" == ':' ]; then
        test -z "$OPTN" && {                        #!!!NONLOCAL
            error --long -a
            return 1
        }
    else
        test -n "$OPTN" && {                        #!!!NONLOCAL
            error --long -d
            return 1
        }
    fi
    return 0
}

error()
{
    local __self__="$self"     #!!!NONLOCAL
    local __help__="$help"     #!!!NONLOCAL
    local __OPTARG__="$OPTARG" #!!!NONLOCAL
    local __opts__="$opts"     #!!!NONLOCAL
    local __opt__="$opt"       #!!!NONLOCAL
    local __OPT__="$OPT"       #!!!NONLOCAL

    local self="error"

    # actions: \
    #  a:argument for option -$OPTARG not found|
    #  o:when $OPTARG != '?': invalid command line option -$OPTARG, or, \
    #    otherwise, usage|
    #  i:invalid argument '$OPTARG' for option -$opt|
    #  d:option '$OPTARG' does not take arguments|
    #  e:error message|
    #  w:warning message|
    #  u:unexpected option -$opt|
    #  g:when $opt == ':': equivalent with 'a', \
    #    when $opt == '?': equivalent with 'o', \
    #    when $opt is anything else: equivalent with 'u'

    local act="e"
    local A="$__OPTARG__" # $OPTARG
    local h="$__help__"   # $help
    local m=""            # error msg
    local O="$__opts__"   # $opts
    local P="$__opt__"    # $opt
    local L="$__OPT__"    # $OPT
    local S="$__self__"   # $self

    local long=''         # short/long opts (default)

    #!!! echo "!!! A='$A'"
    #!!! echo "!!! O='$O'"
    #!!! echo "!!! P='$P'"
    #!!! echo "!!! L='$L'"
    #!!! echo "!!! S='$S'"

    local opt
    local opts=":aA:degh:iL:m:oO:P:S:uw-:"
    local OPTARG
    local OPTERR=0
    local OPTIND=1
    while getopts "$opts" opt; do
        case "$opt" in
            [adeiouwg])
                act="$opt"
                ;;
            #[])
            #	optopt
            #	;;
            [AhLmOPS])
                optarg
                ;;
            \:)	echo "$self: error: argument for option -$OPTARG not found" >&2
                return 1
                ;;
            \?)	if [ "$OPTARG" != "?" ]; then
                    echo "$self: error: invalid command line option -$OPTARG" >&2
                else
                    echo "$self: $(usage $self $opts)"
                fi
                return 1
                ;;
            -)	case "$OPTARG" in
                    long|long-opts)
                        long='l' ;;
                    short|short-opts)
                        long='' ;;
                    *)	echo "$self: error: invalid command line option --$OPTARG" >&2
                        return 1
                        ;;
                esac
                ;;
            *)	echo "$self: error: unexpected option -$OPTARG" >&2
                return 1
                ;;
        esac
    done
    #!!! echo "!!! A='$A'"
    #!!! echo "!!! O='$O'"
    #!!! echo "!!! P='$P'"
    #!!! echo "!!! L='$L'"
    #!!! echo "!!! S='$S'"
    shift $((OPTIND - 1))
    test -n "$1" && m="$1"
    local f="2"
    if [ "$act" == "g" ]; then
        if [ "$P" == ":" ]; then
            act="a"
        elif [ "$P" == "?" ]; then
            act="o"
        else 
            act="u"
        fi
    fi
    local o=''
    if [ -n "$long" -a -n "$L" ]; then
        test "${L:0:1}" != '-' && o+='--'
        o+="$L"
    elif [[ "$act" == [aod] ]]; then
        o="-$A"
    elif [[ "$act" == [iu] ]]; then
        o="-$P"
    fi
    case "$act" in
        a)	m="argument for option $o not found"
            ;;
        o)	if [ "$A" != "?" ]; then
                m="invalid command line option $o"
            else
                act="h"
                m="$(usage $S $O)"
                f="1"
            fi
            ;;
        i)	m="invalid argument for $o: '$A'"
            ;;
        u)	m="unexpected option $o"
            ;;
        d)	m="option $o does not take arguments"
            ;;
        *)	# [ew]
            if [ "$#" -ge "2" ]; then
                S="$1"
                m="$2"
            elif [ "$#" -ge "1" ]; then
                m="$1"
            fi
            ;;
    esac
    if [ "$act" == "w" ]; then
        m="warning${m:+: $m}"
    elif [ "$act" != "h" ]; then
        m="error${m:+: $m}"
    fi
    if [ -z "$S" -o "$S" == "-" ]; then
        printf "%s\n" "$m" >&$f
    else
        printf "%s: %s\n" "$S" "$m" >&$f
    fi
    if [ "$act" == "h" ]; then
        test -n "$1" && h="$1"
        test -n "$h" &&
        printf "%s\n" "$h" >&$f
    fi
    return $f
}

export PATH_SET_TEST_INPUT='+'
export PATH_SET_TEST_EXPR='+'
export PATH_SET_TEST_LIMITS='+'
export PATH_SET_TEST_POOL_SIZE='+'
export PATH_SET_TEST_HASH_SIZE='+'
export PATH_SET_TEST_NODE_SIZE='+'
export PATH_SET_TEST_REHASH_SIZE='+'
export PATH_SET_TEST_REHASH_LOAD='+'
export PATH_SET_TEST_STRUCT_TYPE='+'
export PATH_SET_TEST_SET_TYPE='+'
export PATH_SET_TEST_SEPARATORS='+'

path-set-test()
{
    local self="path-set-test"
    local prog="path-set"
    local cfg0='path-trie-32bit-offsets path-trie-node-32bit-offsets path-trie-elem-32bit-offsets path-trie-uint-hash-identop trie-path-set-print-set value-type-size'
    local cfgl="$cfg0 32bit debug optimize"
    local cfgx="@(${cfgl// /|})"
    local optl='pool-size hash-size node-size rehash-size rehash-load struct-type set-type separators plain-set path-trie ternary-tree linear-hash gnulib-hash'
    local opnx='@(plain-set|path-trie|ternary-tree|linear-hash|gnulib-hash)'
    local optx="@(${optl// /|})"
    # $ ssed -nR '/^CASE/!b;n;n;s/^\s*"//;s/"\s*,\s*$//;s/^[a-z0-9-]+?-time$/real-\0\nuser-\0\nsys-\0/;p' set-stats.def|ssed -nR 'H;$!b;g;s/^\n//;s/\n/ /g;p'
    local stal='dups-line uniq-line line-mem dups-elem uniq-elem dups-node uniq-node elem-insert-eq elem-insert-lt elem-insert-gt elem-insert-ne elem-insert-hit elem-realloc-op real-elem-realloc-time user-elem-realloc-time sys-elem-realloc-time node-insert-eq node-insert-lt node-insert-gt node-insert-ne node-insert-hit node-realloc-op real-node-realloc-time user-node-realloc-time sys-node-realloc-time rehash-eq rehash-ne rehash-op rehash-hit real-rehash-time user-rehash-time sys-rehash-time hash-size hash-used hash-load elem-struct-size node-struct-size elem-struct node-struct elem-mem node-mem total-node-mem total-mem real-insert-time user-insert-time sys-insert-time real-lookup-time user-lookup-time sys-lookup-time'
    local stax="@(${stal// /|})"
    local nodl='trie-plain trie-path lhash-plain lhash-path ghash-plain ghash-path'
    local nodx="${nodl// /-set|}-set"
    local rngx='^\+$|^[1-9][0-9]*(-[1-9][0-9]*)?$'
    local sizx='^[1-9][0-9]*[kKmM]?$'
    local flox='^[0-9]+(\.[0-9]+)?$|^\.[0-9]+$|^NAN$'
    local strx='@(plain-set|path-trie)'
    local setx='@(ternary-tree|@(linear|gnulib)-hash)'
    local numx='^\+$|^[1-9][0-9]*$'
    local sha1='[0-9a-f]{40}'
    local tmpf="/tmp/$self.XXX"
    local defe='10 ** x'
    local defi='files.txt'
    local defl='1-6'
    local defp='total-node-mem'
    local defr="10"
    local defs="100"
    local defB="1000"
    local deft="1000"

    #
    # 'path-set' config parameters
    #
    local path_trie_node_32bit_offsets='' # null or '+'
    local path_trie_elem_32bit_offsets='' # null or '+'
    local path_trie_uint_hash_identop=''  # null or '+'
    local trie_path_set_print_set=''      # null or '+'
    local value_type_size=''              # null, '8', '16', '32' or '64'
    local bit32=''                        # null or '+'
    local debug=''                        # null or '+'
    local optimize=''                     # null or '+'

    #
    # 'path-set' command line options
    #
    local pool_size=''
    local hash_size=''
    local node_size=''
    local rehash_size=''
    local rehash_load=''
    local struct_type=''
    local set_type=''
    local separators=''

    local def_pool_size='128M'
    local def_hash_size='2M'
    local def_node_size='4M'
    local def_rehash_size='NAN'
    local def_rehash_load='NAN'
    local def_struct_type='plain-set'
    local def_set_type='gnulib-hash'
    local def_separators='/'

    local x="eval"
    local act="B"   # actions: \
                    #  A:averages results (--averages[-results]=FILE)|
                    #  b:build binary (--[build-]binary)|
                    #  B:test base (default) (--[test-]base=NUM)|
                    #  D:print node struct def (`-D+' gets all defs; `-D?' prints out all names and exit) (--node-[struct-]def[=NAME])|
                    #  N:print stat names (--[print-]stat-names)|
                    #  R:test results (--[test-]results=FILE)|
                    #  S:collect statistics (--stat[istic]s)|
                    #  t:run test (--[run-]test=NUM)|
                    #  T:run tests (--[run-]tests)|
                    #  E:run series (--[run-]series)
    local C=""      # define the named 'path-set' config parameter (`-C?' prints out all names and exit) (--config-NAME|--config-NAME=VALUE)
    local e=""      # tests range expression (function of 'x') (default: '10 ** x') (--[range-]expr=EXPR)
    local i=""      # input file (default: 'files.txt') (--input=FILE)
    local l=""      # tests range limits (default: '1-6') (--[range-]limits=NUM[-NUM]) 
    local o="-"     # output file when action is '-E' or '-T' ('+' means to generate a SHA1 name) (default: '-' aka stdout) (--output=FILE)
    local O=""      # define the named 'path-set' command line option (`-O?' prints out all names and exit) (--NAME|--NAME=VALUE)
    local p="+"     # account for the named 'path-set' stat parameter (default: 'total-node-mem') (`-p?' prints out all names and exit) (--stat-NAME)
    local q=""      # be quiet -- suppress normal output when action is '-E' or '-T' and output doesn't go to stdout (--quiet)
    local r="+"     # number of times to repeat the 'path-set' command (default: 10) (--repeat=NUM)
    local s="+"     # microseconds to sleep between repeats (default: 100) (--sleep=NUM)

    local arg='+'

    local opt
    local OPT
    local OPTN
    local opts=":A:bB:C:dD:e:Ei:l:No:O:p:qr:R:s:St:Tx-:"
    local OPTARG
    local OPTERR=0
    local OPTIND=1
    while getopts "$opts" opt; do
        # discriminate long options
        optlong

        # translate long options to short ones
        test -n "$OPT" &&
        case "$OPT" in
            averages-results|averages)
                opt='A' ;;
            build-binary|binary)
                opt='b' ;;
            test-base|base)
                opt='B' ;;
            config-$cfgx)
                opt='C' ;;
            node-struct-def|node-def)
                opt='D' ;;
            range-expr|expr)
                opt='e' ;;
            run-series|series)
                opt='E' ;;
            input)
                opt='i' ;;
            range-limits|limits)
                opt='l' ;;
            print-stat-names|stat-names)
                opt='N' ;;
            output)
                opt='o' ;;
            $optx)
                opt='O' ;;
            stat-$stax)
                opt='p' ;;
            quiet)
                opt='q' ;;
            repeat)
                opt='r' ;;
            test-results|results)
                opt='R' ;;
            sleep)
                opt='s' ;;
            statistics|stats)
                opt='S' ;;
            run-test|test)
                opt='t' ;;
            run-tests|tests)
                opt='T' ;;
            *)	error --long -o
                return 1
                ;;
        esac

        # check long option argument
        [[ "$opt" == [COp] ]] ||
        optlongchkarg ||
        return 1

        # handle short options
        case "$opt" in
            d)	x="echo"
                ;;
            x)	x="eval"
                ;;
            [AbENRST])
                optactarg
                ;;
            [Bt])
                [[ "$OPTARG" =~ $numx ]] || {
                    error --long -i
                    return 1
                }
                optactarg
                ;;
            D)	[ "$OPTARG" == '?' ] && {
                    echo -e "${nodl// /-set\\n}-set"
                    return 0
                }
                [[ "$OPTARG" =~ ^\+$|^($nodx)$ ]] || {
                    error --long -i
                    return 1
                }
                optactarg
                ;;
            l)	[[ "$OPTARG" =~ $rngx ]] || {
                    error --long -i
                    return 1
                }
                optarg
                ;;
            [ei])
                [ -n "$OPTARG" ] || {
                    error --long -i
                    return 1
                }
                optarg
                ;;
            [o])
                optarg
                ;;
            [q])
                optopt
                ;;
            C)	[[ -z "$OPT" && "$OPTARG" == '?' ]] && {
                    echo -e "${cfgl// /\\n}"|ssed -R '
                        s/(?<=^value-type-size)$/=8|16|32|64/'
                    return 0
                }
                [[ -n "$OPT" || "${OPTARG%%=*}" == $cfgx ]] || {
                    error -i
                    return 1
                }
                optlong "config"

                [[ "${OPT:7}" == $cfgx ]] || {
                    error --long -o
                    return 1
                }
                [ "${OPT:7}" == 'value-type-size' ] ||
                [ -z "$OPTN" ] || {
                    error --long -d
                    return 1
                }
                case "${OPT:7}" in
                    path-trie-32bit-offsets)
                        path_trie_node_32bit_offsets='+'
                        path_trie_elem_32bit_offsets='+'
                        continue
                        ;;
                    value-type-size)
                        [ -n "$OPTN" ] || {
                            error --long -a
                            return 1
                        }
                        [[ "$OPTARG" == @(8|16|32|64) ]] || {
                            error --long -i
                            return 1
                        }
                        ;;
                    32bit)
                        OPT="${OPT/32bit/bit32}"
                        OPTARG='+'
                        ;;
                    *)	OPTARG='+'
                        ;;
                esac
                OPT="${OPT:7}"
                optarg "${OPT//-/_}"
                ;;
            O)	[[ -z "$OPT" && "$OPTARG" == '?' ]] && {
                    echo -e "${optl// /\\n}"|ssed -R '
                        /^rehash-size$/!s/(?<=-size)$/=NUM[KM]/
                        /^rehash-/s/$/=FLOAT/
                        s/(?<=-type)$/=TYPE/
                        s/(?<=^separators)$/=STR/'
                    return 0
                }
                [[ -n "$OPT" || "${OPTARG%%=*}" == $optx ]] || {
                    error -i
                    return 1
                }
                optlong -

                [[ "${OPT:2}" == $optx ]] || {
                    error --long -o
                    return 1
                }
                [[ "${OPT:2}" == $opnx ]] ||
                [ -n "$OPTN" ] || {
                    error --long -a
                    return 1
                }
                [[ "${OPT:2}" != $opnx ]] ||
                [ -z "$OPTN" ] || {
                    error --long -d
                    return 1
                }
                case "${OPT:2}" in
                    pool-size|hash-size|node-size)
                        [[ "$OPTARG" =~ $sizx ]] || {
                            error --long -i
                            return 1
                        }
                        ;;
                    rehash-size|rehash-load)
                        [[ "$OPTARG" =~ $flox ]] || {
                            error --long -i
                            return 1
                        }
                        ;;
                    struct-type)
                        [[ "$OPTARG" == $strx ]] || {
                            error --long -i
                            return 1
                        }
                        ;;
                    set-type)
                        [[ "$OPTARG" == $setx ]] || {
                            error --long -i
                            return 1
                        }
                        ;;
                    plain-set|path-trie)
                        OPTARG="${OPT:2}"
                        OPT='--struct-type'
                        ;;
                    ternary-tree|linear-hash|gnulib-hash)
                        OPTARG="${OPT:2}"
                        OPT='--set-type'
                        ;;
                    separators)
                        [ -n "$OPTARG" ] || {
                            error --long -i
                            return 1
                        }
                        ;;
                    *)	error "internal: unexpected OPT='$OPT'"
                        return 1
                        ;;
                esac
                OPT="${OPT:2}"
                optarg "${OPT//-/_}"
                ;;
            p)	[[ -z "$OPT" && "$OPTARG" == '?' ]] && {
                    echo -e "${stal// /\\n}"
                    return 0
                }
                [[ -n "$OPT" || "$OPTARG" == $stax ]] || {
                    error -i
                    return 1
                }
                optlong "stat"

                [[ "${OPT:5}" == $stax ]] || {
                    error --long -o
                    return 1
                }
                [ -z "$OPTN" ] || {
                    error --long -d
                    return 1
                }
                p="${OPT:5}"
                ;;
            [rs])
                [[ "$OPTARG" =~ $numx ]] || {
                    error --long -i
                    return 1
                }
                optarg
                ;;
            *)	error --long -g
                return 1
                ;;
        esac
    done
    shift $((OPTIND - 1))

    local i2="$i"
    if [[ "$act" == [BETt] ]]; then
        [ -z "$i2" ] && {
            i="$PATH_SET_TEST_INPUT"
            i2="$i"
            [ -n "$i2" ] || {
                error "\$PATH_SET_TEST_INPUT is null"
                return 1
            }
        }
        [ "$i2" == '+' ] && {
            i2="$defi"
            i=''
        }
        [ -f "$i2" ] || {
            error "input file '$i2' not found"
            test "$x" == "eval" && return 1
        }
    fi
    quote i2

    local e2="$e"
    if [[ "$act" == [ET] ]]; then
        [ -z "$e2" ] && {
            e="$PATH_SET_TEST_EXPR"
            e2="$e"
            [ -n "$e2" ] || {
                error "\$PATH_SET_TEST_EXPR is null"
                return 1
            }
        }
        [ "$e2" == '+' ] && {
            e2="$defe"
            e=''
        }
    fi
    # stev: not quoting $e2 because we need it so when $act == '-T'

    local l2="$l"
    if [[ "$act" == [ET] ]]; then
        [ -z "$l2" ] && {
            l="$PATH_SET_TEST_LIMITS"
            l2="$l"
            [[ "$l2" =~ $rngx ]] || {
                error "invalid \$PATH_SET_TEST_LIMITS: '$PATH_SET_TEST_LIMITS'"
                return 1
            }
        }
        [ "$l2" == '+' ] && {
            l2="$defl"
            l=''
        }
    fi
    # stev: need not quote $l2

    if [[ "$o" != [-+] ]]; then
        [ -n "$o" ] || {
            error "output file cannot be null"
            return 1
        }
        [ ! -e "$o" ] || {
            error "output file '$o' already exists"
            test "$x" == "eval" && return 1
        }
    fi
    quote o

    local p2="$p"
    test "$p2" == '+' && p2="$defp"
    # stev: need not quote $p2

    local r2="$r"
    if [ "$r2" != '+' ]; then
        [ "$r2" -gt 0 ] || {
            error "repeat count cannot be 0"
            return 1
        }
    else
        r2="$defr"
    fi
    # stev: need not quote $r2

    local s2="$s"
    test "$s2" == '+' && s2="$defs"
    # stev: need not quote $s2

    if [ "$act" == 'A' -a "$arg" != '-' -a ! -f "$arg" ]; then
        error "averages results input file '$arg' not found"
        test "$x" == "eval" && return 1
    fi
    if [ "$act" == 'R' -a "$arg" != '-' -a ! -f "$arg" ]; then
        error "test results input file '$arg' not found"
        test "$x" == "eval" && return 1
    fi

    local n
    local n2
    local v
    local v2

    for n in \
BETt:pool_size \
BETt:hash_size \
BETt:node_size \
BETt:rehash_size \
BETt:rehash_load \
BNSTt:struct_type \
BNSTt:set_type \
BETt:separators;
    do
        [[ "$act" == [${n%%:*}] ]] || continue
        n="${n##*:}"
        assign2 v "$n"
        test "$v" == '+' && {
            assign2 "$n" "def_$n"
            continue
        }
        test -n "$v" && continue
        n2="$(tr '[a-z]' '[A-Z]' <<< "$n")" && [ -n "$n2" ] || {
            error "inner command failed: tr [0]"
            return 1
        }
        assign2 v "PATH_SET_TEST_$n2"
        test -n "$v" || {
            error "\$PATH_SET_TEST_$n2 is null"
            return 1
        }
        test "$v" == '+' && {
            assign2 "$n" "def_$n"
            continue
        }
        case "$n" in
            pool_size|hash_size|node_size)
                [[ "$v" =~ $sizx ]] || n=''
                ;;
            rehash_size|rehash_load)
                [[ "$v" =~ $flox ]] || n=''
                ;;
            struct_type)
                [[ "$v" == $strx ]] || n=''
                ;;
            set_type)
                [[ "$v" == $setx ]] || n=''
                ;;
        esac
        test -n "$n" || {
            error "invalid \$PATH_SET_TEST_$n2: $v"
            return 1
        }
        assign2 "$n" v
    done
    # $pool_size, $hash_size, $node_size, $rehash_size, $rehash_load,
    # $struct_type and $set_type need not be quoted, only $separators
    quote2 separators

    local S

    if [[ "$act" == [ST] ]]; then
        S=($(eval "$self --$struct_type --$set_type -N|
                ssed -R 's/^[a-z0-9-]+?-time$/real-\0\nuser-\0\nsys-\0/'")) &&
        [ "${#S[@]}" -gt 0 ] || {
            error "inner command failed: $self -N|ssed ..."
            return 1
        }
    fi

    local a
    local c
    local c2
    local c3
    local f
    local k
    local s3
    local s4
    local t

    if [ "$act" == "N" ]; then
        c="\
./$prog --$struct_type --$set_type -N"
    elif [ "$act" == "S" ]; then
        a='
            BEGIN {
                #
                # struct-type: '"$struct_type"'
                # set-type:    '"$set_type
                #"
        n2=0
        for ((k=0; k<${#S[@]}; k++)); do
            v="${S[k]}"
            a+='
                N['"$k"'] = "'"$v"'"'
            test "$n2" -lt "${#v}" && n2="${#v}"
        done
        for ((k=0; k<${#S[@]}; k++)); do
            [[ "${S[k]}" =~ hash-load ]] && a+='
                T['"$k"'] = 1'
        done
        a+='
            }
            function set(k,	t)
            {
                t = T[k] \
                    ? float($2) \
                    : int($2)
                if (n == 0 ||
                    m[k] > t)
                    m[k] = t
                if (n == 0 ||
                    M[k] < t)
                    M[k] = t
                s[k] += t
                v[k] += t ** 2
            }
            function println(name, min, max, avg, var, dev)
            { printf("%-'"$n2"'s  %-11s  %-11s  %-14s  %-16s  %-15s\n", \
                name, min, max, avg, var, dev) }
            function float(v)
            { return sprintf("%.2f", v) }
            function header()
            { println("param", "min", "max", "avg", "var", "dev") }
            function abs(v)
            { return v >= 0 ? v : -v }
            function result(k,	A, V, D)
            {
                # average
                A = s[k] / n

                # variance
                V = v[k] / n - A ** 2

                # standard deviation
                D = sqrt(abs(V)) 

                println(N[k], m[k], M[k], float(A), float(V), float(D))
            }
            function error(s)
            {
                printf( \
                    "'"$self"': statistics: input error: %s\n", s) \
                    > "/dev/stderr"
                input_error = 1
                exit 1
            }
            function invalid_line(s)
            { error(sprintf("line #%d: %s: \x27%s\x27", FNR, s, $0)) }
            function empty_input()
            { error("empty input") }
            {
                i = (FNR - 1) % '"$((${#S[@]} + 1))"'
                if (i < '"${#S[@]}"') {
                    if (NF != 2)
                        invalid_line("unexpected line")
                    if ($1 != N[i] ":")
                        invalid_line("unexpected stat param")
                    set(i)
                }
                else {
                    if (NF != 0)
                        invalid_line("expected empty line")
                    n ++
                }
            }
            END {
                if (!input_error) {
                    if (FNR == 0)
                        empty_input()
                    i = (FNR - 1) % '"$((${#S[@]} + 1))"'
                    if (i != '"$((${#S[@]} - 1))"')
                        invalid_line("unexpected end line")
                    n ++
                    header()
                    for (i = 0; i < '"${#S[@]}"'; i ++)
                        result(i)
                }
            }'
        c="\
awk '$a'"
    elif [ "$act" == "A" ]; then
        s3='
            /^\$\s*'$self'\s+-p\s*'"$p2"'\s+-R\s*\.\.\.\s*$/!b
            n
            :0
            n
            /^\$/q
            p
            b0'
        a='
            function abs(v)
            { return v >= 0 ? v : -v }
            {
                t = int($4)
                if (n == 0 ||
                    m > t)
                    m = t
                if (n == 0 ||
                    M < t)
                    M = t
                s += t
                v += t ** 2
                n ++
            }
            END {
                if (n > 0) {
                    a = s / n
                    v = v / n - a ** 2
                    d = sqrt(abs(v))
                    printf( \
                        "min: %d\nmax: %d\navg: %.2f\nvar: %.2f\ndev: %.2f\n", \
                        m, M, a, v, d)
                }
            }'
        test "$arg" == '-' && arg=''
        quote arg
        c="\
ssed -nR '$s3'${arg:+ \\
$arg}|
awk '$a'"
    elif [ "$act" == "R" ]; then
        s3='
            1i\
files       min          max          avg             var               dev
            :0
            /^\$\s*time\s+'"$self"'\s*.*?\s-t\s*(\d+)\s*$/!b
            s//\1/
            h
            :1
            n
            /^\$/b0
            /^'"$p2"'\s+(.*?)\s*$/!b1
            s//\1/
            G
            s/^(.*?)\n(.*)$/\2          \1/
            s/^(.{10}) */\1  /
            p
            b1'
        test "$arg" == '-' && arg=''
        quote arg
        c="\
ssed -nR '$s3'${arg:+ \\
$arg}"
    elif [[ "$act" == [bD] ]]; then
        c="\
make"
        [ "$act" == 'b' ] &&
        c+=" -B"
        [ "$act" == 'D' -a "$arg" == '+' ] &&
        c+=" CPPS=all"
        [ "$act" == 'D' -a "$arg" != '+' ] &&
        c+=" CPPS=$arg.c"
        for n in ${cfg0//-/_}; do # do not quote $cfg0
            n2="$(tr '[a-z]' '[A-Z]' <<< "$n")" &&
            [ -n "$n2" ] || {
                error "inner command failed: tr [1]"
                return 1
            }
            assign2 v "$n"
            [ -z "$v" ] && continue
            [ "$v" == '+' ] && v=''
            c+=" \
CONFIG+=$n2${v:+=$v}"
        done
        [ -n "$bit32" ] && c+=" \
32BIT=yes"
        [ -z "$bit32" ] && c+=" \
32BIT=no"
        [ -n "$debug" ] && c+=" \
DEBUG=yes"
        [ -z "$debug" ] && c+=" \
DEBUG=no"
        [ -n "$optimize" ] && c+=" \
OPT=4"
        [ "$act" == 'D' -a "$arg" == '+' ] && s3='
            /^#\s+1\s+\x22((?:'"$nodx"')\.c)\x22\s*$/!b0
            s||// \1|
            h
            b
            :0'
        [ "$act" == 'D' ] && s3+='
            /^struct\s+path_trie_node_t\s*$/b1
            /^struct\s+[a-z0-9_]+_node_t\s*$/!b
            /_alloc_node_t\s*$/b
            x
            /^$/!p
            x
            :1
            p
            :2
            n
            /^\}\s*;\s*$/!b3
            p
            b
            :3
            /^\s*$/b2
            :4
            s/^(\t*) /\1\t/
            t4
            :5
            s/^( *)\t/\1    /
            t5
            p
            b2'
        [ "$act" == 'D' -a "$arg" == '+' ] && s4='
            s|^//.*$|//\n\0\n//|
            1!s|^(?=//)|\n|'
        [ "$act" == 'D' ] && c+="|
ssed -nR '$s3'${s4:+|
ssed -R '$s4'}"
    elif [ "$act" == "B" ]; then
        v="$(eval "./$prog -C|
                ssed -nR 's/^CONFIG_PATH_TRIE_NODE_32BIT_OFFSETS:\s*(no|yes)\s*$/\1/p'")" || {
            error "inner command failed: ./$prog -C|ssed ..."
            return 1
        }
        test "$arg" == '+' && arg="$defB"
         c="\
shuf $i2|head -$arg|
./$prog --$struct_type --$set_type -p $pool_size -h $hash_size"
        [ "$v" == 'yes' ] && c+=" \
-n $node_size"
        c+=" -z $rehash_size -l $rehash_load -t $separators \
-LS"
    elif [ "$act" == "t" ]; then
        test "$s" == '+' && s="$defs"
        test "$arg" != '+' && arg=" $arg"
        c="\
(
set +o errexit
set -o pipefail
${i:+
PATH_SET_TEST_INPUT=$i2}"
        for n in \
pool_size \
hash_size \
node_size \
rehash_size \
rehash_load \
struct_type \
set_type \
separators;
        do
            n2="$(tr '[a-z]' '[A-Z]' <<< "$n")" &&
            [ -n "$n2" ] || {
                error "inner command failed: tr [2]"
                return 1
            }
            assign2 v "$n"
            #!!! assign2 v2 "def_$n"
            #!!! [ "$v" == "$v2" ] && continue
            c+=${c:+$'\n'}"\
PATH_SET_TEST_$n2=$v"
        done
        c+=$'\n'"
for ((k=0; k<$r2; k++)); do
    $self -B$arg|
    ssed -R '/^\s*$/d'
    r=\"\$?\"
    [ \"\$r\" -eq 0 -o \"\$r\" -eq 141 ] || exit \"\$r\"
    [ \"\$k\" -lt $((r2 - 1)) ] && echo"
        [ "$s" -gt 0 ] && {
            # stev: assume $s doesn't have heading 0's
            n="${#s}"
            [ "$n" -gt 6 ] && (( n-= 6 )) || n=0
            # stev: compute $s3 = $s / 10^6
            printf -v s3 '%d.%.6d' "${s:0:$n}" "${s:$n}"
            c+="
    sleep $s3"
        }
        c+="
done|
$self -S

r=\"\$?\"
test \"\$r\" -eq 141 && r=0
exit \"\$r\"
)"
    elif [ "$act" == "T" ]; then
        local N=0
        local -a C2
        C2[((N ++))]="\
date +'%Y-%m-%d %H:%M:%S.%N %z'"
        C2[((N ++))]="\
./$prog -C"
        C2[((N ++))]="\
./$prog --$struct_type --$set_type -I"
        C2[((N ++))]="\
./$prog --$struct_type --$set_type -N"
        local N2="$N"
        C2[((N ++))]="\
PATH_SET_TEST_INPUT=$i2"
        c=''
        for n in \
pool_size \
hash_size \
node_size \
rehash_size \
rehash_load \
struct_type \
set_type \
separators;
        do
            n2="$(tr '[a-z]' '[A-Z]' <<< "$n")" &&
            [ -n "$n2" ] || {
                error "inner command failed: tr [3]"
                return 1
            }
            assign2 v "$n"
            #!!! assign2 v2 "def_$n"
            #!!! [ "$v" == "$v2" ] && continue
            C2[((N ++))]="\
PATH_SET_TEST_$n2=$v"
            [[ "$n" == @(struct|set)_type ]] && continue
            [ "${v:0:1}" == "'" ] &&
            quote2 -i v
            c+=$'\n'"\
# --${n//_/-}=$v \\"
        done
        local e3="$e2"
        quote2 e3
        quote2 -i e3
        c="\
echo '#
# # meta command:
# $ $self \\
# --input=$i2 \\
# --expr=$e3 \\
# --limits=$l2 \\\
$c
# --repeat=$r2 \\
# --sleep=$s2 \\
# --$struct_type \\
# --$set_type \\
# --run-tests
#
'"
        for ((k=0;k<N;k++)); do
            c2="${C2[k]}"
            c3="\$ $c2"
            quote2 c3
            [ "$k" -le "$N2" ] &&
            c+=$'\n'
            c+=$'\n'"\
echo $c3
$c2 2>&1"
            [ "$k" -lt "$N2" ] && c+="
echo -e '$\n'"
        done
        c+="
echo -e '$\n'"
        test "$r" == '+' && r2=''
        test "$s" == '+' && s2=''
        local l0="${l2%%-*}"
        local l1="${l2:$((${#l0} + 1))}"
        test -z "$l1" && l1="$l0"
        local l3
        local x0="$x" # save "$x"
        for ((l3=$l0; l3<=l1; l3++)); do
            x="$l3"
            eval "t=\"\$(($e2))\""
            c2="\
time $self ${r2:+-r$r2 }${s2:+-s$s2 }-t $t"
            c3="$c2"
            quote2 -i c3
            c+=$'\n\n'"\
echo '\$ $c3'
$c2 2>&1 || \\
echo 'command failed: $c3'
echo -e '$\n'"
        done
        x="$x0" # restore $x
        t="$tmpf.0"
        if [ "$x" == "eval" ]; then
            t="$(mktemp $t)" && [ -n "$t" ] || {
                error "inner command failed: mktemp [0]"
                return 1
            }
            quote t
        fi
        c2=''
        for n in ${S[@]}; do
            c2+=${c2:+$'\n\n'}"\
echo '$ $self -p $n -R ...'
$self -p $n -R $t 2>&1
echo -e '$\n'"
        done
        c="\
(
trap 'rm -f $t' EXIT

(
$c
) 2>&1|"
        [ "$o" == '-' -o -z "$q" ] && c+="
stdbuf -i0 -o0 -e0 \\"
        c+="
tee $t

$c2
)"
        t="$tmpf.1"
        if [ "$x" == "eval" ]; then
            t="$(mktemp $t)" && [ -n "$t" ] || {
                error "inner command failed: mktemp [1]"
                return 1
            }
            quote t
        fi
        c2=''
        for n in ${S[@]}; do
            c2+=${c2:+$'\n\n'}"\
echo '$ $self -p $n -A ...'
$self -p $n -A $t 2>&1
echo -e '$\n'"
        done
        c3="\
("
        [ "$o" == '-' ] && c3+="
set -o errexit
set -o pipefail"
        c3+="
trap 'rm -f $t' EXIT

$c 2>&1|"
        [ "$o" == '-' -o -z "$q" ] && c3+="
stdbuf -i0 -o0 -e0 \\"
        c3+="
tee $t

$c2"
        [ "$o" == '-' ] && c3+=$'\n'"
! grep $t -qe '^command failed:'"
        c3+="
)"
        c="$c3"
        t="$tmpf.2"
        [ "$o" != '-' ] &&
        if [ "$x" == "eval" ]; then
            t="$(mktemp $t)" && [ -n "$t" ] || {
                error "inner command failed: mktemp [2]"
                return 1
            }
            quote t
        fi
        c2=''
        [ "$o" == '+' ] && f='$f' || f="$o"
        [ "$o" == '+' ] && c2+="\
f=\"\$(sha1sum $t|ssed -nr '1{s/^($sha1)\s+.*$/\1/p;q}')\"
test ! -e $f || cmp -s $t $f || {
    error \"file $f already exists with different content\"
    exit 1"\
$'\n}'
        [ "$o" != '-' ] && c2+=${c2:+$'\n'}"\
cp $t $f"
        [ "$o" == '+' ] && c2+="
echo $f"
        [ "$o" != '-' ] && c="\
(
set -o errexit
set -o pipefail
trap 'rm -f $t' EXIT

$c"
        [ "$o" != '-' -a -z "$q" ] && c+=" 2>&1|
stdbuf -i0 -o0 -e0 \\
tee $t"
        [ "$o" != '-' -a -n "$q" ] && c+=" \
>$t 2>&1
r=\"\$?\""
        [ "$o" != '-' ] && c+="

$c2"$'\n'
        [ "$o" != '-' -a -n "$q" ] && c+="
test \"\$r\" -ne 0 && exit \"\$r\""
        [ "$o" != '-' ] && c+="
! grep $t -qe '^command failed:'
)"
    elif [ "$act" == 'E' ]; then
        c="\
$self -d -b"
        c2=''
        for n in $cfgl; do # do not quote $cfgl
            [ "$n" == '32bit' ] &&
            n2='bit32' ||
            n2="$n"
            assign2 v "${n2//-/_}"
            [ -z "$v" ] && continue
            [ "$v" == '+' ] && v=''
            c+=" \
--config-$n${v:+=$v}"
            c2+=$'\n'"\
# --config-$n${v:+=$v} \\"
        done
        c3="$(eval "$c")" || {
            error "inner command failed: $c"
            return 1
        }
        local N=0
        local -a C2
        C2[((N ++))]="\
date"
        C2[((N ++))]="\
$c3"
        C2[((N ++))]="\
./$prog -C"
        local N2="$N"
        C2[((N ++))]="\
PATH_SET_TEST_INPUT=$i2"
        quote2 e2
        C2[((N ++))]="\
PATH_SET_TEST_EXPR=$e2"
        C2[((N ++))]="\
PATH_SET_TEST_LIMITS=$l2"
        c=''
        for n in \
pool_size \
hash_size \
node_size \
rehash_size \
rehash_load \
separators;
        do
            n2="$(tr '[a-z]' '[A-Z]' <<< "$n")" &&
            [ -n "$n2" ] || {
                error "inner command failed: tr [4]"
                return 1
            }
            assign2 v "$n"
            #!!! assign2 v2 "def_$n"
            #!!! [ "$v" == "$v2" ] && continue
            C2[((N ++))]="\
PATH_SET_TEST_$n2=$v"
            [ "${v:0:1}" == "'" ] &&
            quote2 -i v
            c+=$'\n'"\
# --${n//_/-}=$v \\"
        done
        quote2 -i e2
        c="\
echo '#
# # meta command:
# $ $self \\\
$c2
# --input=$i2 \\
# --expr=$e2 \\
# --limits=$l2 \\\
$c
# --repeat=$r2 \\
# --sleep=$s2 \\
# --run-series
#
'"
        for ((k=0;k<N;k++)); do
            c2="${C2[k]}"
            c3="\$ $c2"
            quote2 c3
            [ "$k" -le "$N2" ] &&
            c+=$'\n'
            c+=$'\n'"\
echo $c3
$c2 2>&1"
            [ "$k" -lt "$N2" ] && c+="
echo -e '$\n'"
        done
        c+="
echo -e '$\n'"
        c2="\
time $self"
        [ "$r" != '+' ] && c2+=" \
--repeat=$r"
        [ "$s" != '+' ] && c2+=" \
--sleep=$s"
        c2+=" \
--quiet --output=+ --run-tests"
        for n in plain-set path-trie; do
            for n2 in ternary-tree linear-hash gnulib-hash; do
                c3="$c2 --$n --$n2"
                quote2 -i c3
                c+=$'\n\n'"\
echo '\$ $c3'
$c2 --$n --$n2 2>&1 || \\
echo 'command failed: $c3'
echo -e '$\n'"
            done
        done
        t="$tmpf.3"
        [ "$o" != '-' ] &&
        if [ "$x" == "eval" ]; then
            t="$(mktemp $t)" && [ -n "$t" ] || {
                error "inner command failed: mktemp [3]"
                return 1
            }
            quote t
        fi
        c2=''
        [ "$o" == '+' ] && f='$f' || f="$o"
        [ "$o" == '+' ] && c2+="\
f=\"\$(sha1sum $t|ssed -nr '1{s/^($sha1)\s+.*$/\1/p;q}')\"
test ! -e $f || cmp -s $t $f || {
    error \"file $f already exists with different content\"
    exit 1"\
$'\n}'
        [ "$o" != '-' ] && c2+=${c2:+$'\n'}"\
cp $t $f"
        [ "$o" == '+' ] && c2+="
echo $f"
        c3="\
(
set -o errexit
set -o pipefail"
        [ "$o" == '-' ] && c3+=$'\n'
        [ "$o" != '-' ] && c3+="
trap 'rm -f $t' EXIT

("
        c3+="
$c"
        [ "$o" != '-' ] && c3+="
)"
        [ "$o" != '-' -a -z "$q" ] && c3+=" 2>&1|
stdbuf -i0 -o0 -e0 \\
tee $t"
        [ "$o" != '-' -a -n "$q" ] && c3+=" \
>$t 2>&1
r=\"\$?\""
        [ "$o" != '-' ] && c3+="

$c2"$'\n'
        [ "$o" != '-' -a -n "$q" ] && c3+="
test \"\$r\" -ne 0 && exit \"\$r\""
        [ "$o" != '-' ] && c3+="
! grep $t -qe '^command failed:'"
        c3+="
)"
        c="$c3"
    else
        error "internal: unexpected act='$act'"
        return 1
    fi

    $x "$c"
}

path-set-test-grep()
{
    local self="path-set-test-grep"
    local cfnl='path-trie-node-32bit-offsets path-trie-elem-32bit-offsets path-trie-uint-hash-identop trie-path-set-print-set 32bit debug optimize'
    local cfnx="config-${cfnl// /|config-}"
    local cfal='value-type-size'
    local cfax="config-${cfal// /|config-}"
    local cfg0="config-${cfnl// / config-} config-${cfal// / config-}"
    local cfgl="config-path-trie-32bit-offsets $cfg0"
    local base='input expr limits'
    local basx="${base// /|}"
    # stev: 'struct-type' and 'set-type' are required to be at the end of $opta
    local opta='pool-size hash-size node-size rehash-size rehash-load separators struct-type set-type'
    local optx="${opta// /|}"
    local optn='plain-set path-trie ternary-tree linear-hash gnulib-hash'
    local coll="$base ${opta/ struct-type set-type}"
    local colx="${coll// /|}"
    local tstl="$cfgl $base $opta $optn"
    local tstx="@(${tstl// /|})"
    # stev: $stal below is a duplicate of $stal in 'path-set-test' above
    local stal='dups-line uniq-line line-mem dups-elem uniq-elem dups-node uniq-node elem-insert-eq elem-insert-lt elem-insert-gt elem-insert-ne elem-insert-hit elem-realloc-op real-elem-realloc-time user-elem-realloc-time sys-elem-realloc-time node-insert-eq node-insert-lt node-insert-gt node-insert-ne node-insert-hit node-realloc-op real-node-realloc-time user-node-realloc-time sys-node-realloc-time rehash-eq rehash-ne rehash-op rehash-hit real-rehash-time user-rehash-time sys-rehash-time hash-size hash-used hash-load elem-struct-size node-struct-size elem-struct node-struct elem-mem node-mem total-node-mem total-mem real-insert-time user-insert-time sys-insert-time real-lookup-time user-lookup-time sys-lookup-time'
    local perx='[0-9]+(\.[0-9]+)?[kKmMgG]?'
    local stax="@(${stal// /|})"
    local meta='[][(){}^$|.?*+\\]'
    local sha1='[0-9a-f]{40}'

    #
    # 'path-set-test' options
    #
    local config_path_trie_node_32bit_offsets='' # boolean
    local config_path_trie_elem_32bit_offsets='' # boolean
    local config_path_trie_uint_hash_identop=''  # boolean
    local config_trie_path_set_print_set=''      # boolean
    local config_value_type_size=''              # string
    local config_32bit=''                        # boolean
    local config_debug=''                        # boolean
    local config_optimize=''                     # boolean
    local input=''                               # string
    local expr=''                                # string
    local limits=''                              # string
    local pool_size=''                           # string
    local hash_size=''                           # string
    local node_size=''                           # string
    local rehash_size=''                         # string
    local rehash_load=''                         # string
    local struct_type=''                         # string
    local set_type=''                            # string
    local separators=''                          # string

    local P0=() # stev: '-p|--stat-NAME=ARG' names
    local P1=() # stev: '-p|--stat-NAME=ARG' args
    local P2=0  # stev: '-p|--stat-NAME-time=+' count
    local P3=0  # stev: '-p|--stat-NAME-mem=+' count
    local p2=''

    local v

    local x="eval"
    local act="T"   # actions: \
                    #  E:grep series (--[grep-]series)|
                    #  T:grep tests (default) (--[grep-]tests)|
                    #  G:print raw grepped text (--raw-text)|
                    #  L:print raw text list (--raw-list)|
                    #  M:print formatted text list (--[formatted-]list)|
                    #  R:print raw text table (--raw-table)|
                    #  F:print formatted table (--[formatted-]table)
    local e=''      # exclude all the 'config-*' parameters from output (--exclude-config)
    local H=''      # print out the test file name for each match (--[with-]filename)
    local n="="     # negate the next 'path-set-test' command line option (--not)
    local O=""      # define the named 'path-set-test' command line option (`-O?' prints out all names and exit) (--NAME|--NAME=VALUE)
    local p=""      # account for the named 'path-set' stat parameter (cumulative); the value is of form NUM[.NUM][KMG], or is equal to '+' in case NAME is of form '*-mem' (`-p?' prints out all names and exit) (--stat-NAME|--stat-NAME=VALUE)
    local t="+"     # filter for the output of commands of form 'path-set-test --run-test=NUM' -- when action is '-[FLMR]' (default: '1000000') (--[ran-]test=NUM)
    local V=""      # be verbose when action is '-[FLMR]' (--verbose)

    local arg='+'

    local opt
    local OPT
    local OPTN
    local opts=":deEFGHLMnO:p:Rt:TVx-:"
    local OPTARG
    local OPTERR=0
    local OPTIND=1
    while getopts "$opts" opt; do
        # discriminate long options
        optlong

        # translate long options to short ones
        test -n "$OPT" &&
        case "$OPT" in
            exclude-config)
                opt='e' ;;
            grep-series|series)
                opt='E' ;;
            formatted-table|table)
                opt='F' ;;
            raw-text)
                opt='G' ;;
            with-filename|filename)
                opt='H' ;;
            raw-list)
                opt='L' ;;
            formatted-list|list)
                opt='M' ;;
            not)
                opt='n' ;;
            $tstx)
                opt='O' ;;
            stat-$stax)
                opt='p' ;;
            raw-table)
                opt='R' ;;
            ran-test|test)
                opt='t' ;;
            grep-tests|tests)
                opt='T' ;;
            verbose)
                opt='V' ;;
            *)	error --long -o
                return 1
                ;;
        esac

        # check long option argument
        [[ "$opt" == [Op] ]] ||
        optlongchkarg ||
        return 1

        # handle short options
        case "$opt" in
            d)	x="echo"
                ;;
            x)	x="eval"
                ;;
            [EFGLMRT])
                optactarg
                ;;
            [t])
                optarg
                ;;
            [HeV])
                optopt
                ;;
            n)	[ "$n" != '=' ] && n='=' || n='!'
                ;;
            O)	[[ -z "$OPT" && "$OPTARG" == '?' ]] && {
                    echo -e "${tstl// /\\n}"|ssed -R '
                        /^(?:config-value-type-size|input|expr|limits|[a-z-]+-(?:size|load|type)|separators)$/s/$/=STR/'
                    return 0
                }
                [[ -n "$OPT" || "${OPTARG%%=*}" == $tstx ]] || {
                    error -i
                    return 1
                }
                optlong -

                [[ "${OPT:2}" == $tstx ]] || {
                    error --long -o
                    return 1
                }
                if [[ "${OPT:2}" == @($cfax|$basx|$optx) ]]; then
                    [ -n "$OPTN" ] || {
                        error --long -a
                        return 1
                    }
                    [ -n "$OPTARG" ] || {
                        error --long -i
                        return 1
                    }
                else
                    [ -z "$OPTN" ] || {
                        error --long -d
                        return 1
                    }
                fi
                case "${OPT:2}" in
                    config-path-trie-32bit-offsets)
                        config_path_trie_node_32bit_offsets="$n"
                        config_path_trie_elem_32bit_offsets="$n"
                        n='='
                        continue
                        ;;
                    @($cfax|$basx|$optx))
                        # stev: do not allow embedded newlines in args
                        [[ "$OPTARG" =~ $'\n' ]] && {
                            error --long -i
                            return 1
                        }
                        ;;
                    @($cfnx))
                        ;;
                    plain-set|path-trie)
                        OPTARG="${OPT:2}"
                        OPT='--struct-type'
                        ;;
                    ternary-tree|linear-hash|gnulib-hash)
                        OPTARG="${OPT:2}"
                        OPT='--set-type'
                        ;;
                    *)	error "internal: unexpected OPT='$OPT'"
                        return 1
                        ;;
                esac
                OPTARG="$n$OPTARG"
                OPT="${OPT:2}"
                optarg "${OPT//-/_}"
                n='='
                ;;
            p)	[[ -z "$OPT" && "$OPTARG" == '?' ]] && {
                    echo -e "${stal// /\\n}"
                    return 0
                }
                [[ -n "$OPT" || \
                    "$OPTARG" =~ ^(${stal// /|})(=$perx)?$ || \
                    "$OPTARG" =~ ^(${stal// /|})=\+$ && \
                    "$OPTARG" =~ -(mem|time)=\+$ ]] || {
                    error -i
                    return 1
                }
                optlong "stat"

                [[ "${OPT:5}" == $stax ]] || {
                    error --long -o
                    return 1
                }
                [[ -z "$OPTN" || "$OPTARG" =~ ^$perx$ || \
                    "$OPT" =~ -(mem|time)$ && \
                    "$OPTARG" == '+' ]] || {
                    error --long -i
                    return 1
                }
                v="${P0[@]}"
                [[ "${v// /|}" =~ ^(${p// /|})${p:+(%*)}$ ]] &&
                v="${BASH_REMATCH[2]}" ||
                v=''
                [ -n "$OPTN" ] && {
                    P0[${#P0[@]}]="${OPT:5}$v%"
                    if [[ "$OPTARG" == '+' && "$OPT" =~ -time$ ]]; then
                        P1[${#P1[@]}]="+${OPT:5}"
                        (( P2 ++ ))
                    else
                        P1[${#P1[@]}]="$OPTARG"
                    fi
                }
                p+="${p:+ }${OPT:5}"
                [[ -n "$OPTN" && "$OPTARG" == '+' && "$OPT" =~ -mem$ ]] && {
                    P0[${#P0[@]}]="${OPT:5}$v%%"
                    P1[${#P1[@]}]="+${OPT:5}"
                    p+="${p:+ }${OPT:5}"
                    (( P3 ++ ))
                }
                ;;
            *)	error --long -g
                return 1
                ;;
        esac
    done
    shift $((OPTIND - 1))

    local n
    local n2
    local v2

    if [[ "$act" == [FGLMR] ]]; then
        [ -z "$t" ] && {
            error "argument of option '-t|--ran-tests' cannot be null"
            return 1
        }
        [ "$t" == '+' ] && t='1000000'

        [ -n "$e" -a -n "$V" ] &&
        error -w "option '-V|--verbose' overrides option 'e|--exclude-config'"

        if [ -n "$p" ]; then
            # stev: do not quote $stal and $p below
            p="$(printf '%s\n' $stal $p|sort|uniq -d|tr '\n' ' ')" || {
                error "inner command failed: printf|sort|uniq|tr"
                return 1
            }
            p="${p%%+( )}"
            # stev: do not quote $p and $P0 below
            p2="$(printf '%s\n' $p ${P0[@]}|sort|tr '\n' ' ')" || {
                error "inner command failed: printf|sort|tr"
                return 1
            }
            v="$p2 "
            p2=''
            for n in $stal; do # stev: do not quote $stal
                v2="$v"
                while [ -n "$v2" ]; do
                    n2="${v2%% *}"
                    v2="${v2#* }"
                    [[ "$n2" =~ ^$n%*$ ]] &&
                    p2+="${p2:+ }$n2"
                done
            done
        fi
    fi

    local sn='
            BEGIN {
                U[0] = ""
                U[1] = "K"
                U[2] = "M"
                U[3] = "G"
                U[4] = "T"
            }
            function round(x)
            { return int(x + 0.5) }
            function num(n,	i, q)
            {
                n = round(n)
                for (i = 4; i >= 0; i --) {
                    q = n / (1024 ** i)
                    if (int(q) > 0)
                        break
                }
                return sprintf( \
                    n > 0 && n % (1024 ** i) != 0 \
                    ? "%.1f%s" : "%lu%s", \
                    q, U[i])
            }'
    local sf='
            function float(x)
            { return sprintf("%.02f", x) }'

    local c
    local k

    if [ "$P2" -gt 0 -o "$P3" -gt 0 ]; then
        local a="$sn$sf"'
            BEGIN {
                n = split(col, C, ",")
                for (i = 1; i <= n; i ++) {
                    F[i] = C[i] !~ /-mem$/
                }
            }
            {
                if (FNR == 1) {
                    for (i = 1; i <= n; i ++) {
                        for (k = 1; k <= NF; k ++) {
                            if ($k == C[i])
                                break 
                        }
                        if (k > NF) {
                            printf("'$self': error: internal: " \
                                "column \x27%s\x27 not found\n", C[i]) \
                                > "/dev/stderr"
                            error = 1
                            exit 1
                        }
                        C[i] = k
                    }
                }
                else {
                    for (i = 1; i <= n; i ++) {
                        S[i] += $(C[i])
                        N[i] ++
                    }
                }
            }
            END {
                if (!error) {
                    for (i = 1; i <= n; i ++) {
                        v = N[i] ? S[i] / N[i] : 0
                        print F[i] ? float(v) : num(v)
                    }
                }
            }'

        local A=''
        # do not quote $base, $cfgl, $opta below
        for n in $cfgl $base $opta; do
            assign2 v "${n//-/_}"
            [ -z "$v" ] && continue
            case "${v:0:1}" in
                =)	;;
                !)	A+="${A:+ }--not" ;;
                *)	error "internal: unexpected ${n//-/_}='$v'"
                    return 1
                    ;;
            esac
            v2="${v:1}"
            if [[ "$n" == @(expr|separators) ]]; then
                quote2 v2
            else
                quote v2
            fi
            [[ "$n" == @(struct|set)-type ]] && n=''
            A+="${A:+ }--${n:+$n}${v2:+${n:+=}$v2}"
        done
        local t2="$t"
        quote t2
        A+="${A:+ }-t $t2"

        local c0="\
set -o pipefail &&
$self -Re${A:+ $A}"

        if [ "$P3" -gt 0 ]; then
            c="\
$c0 --stat-line-mem|
awk -F '\t' -v col=line-mem '$a'"
            [ "$x" == 'echo' ] && echo "$c"
            v="$(eval "$c")"
            local r="$?"
            [ "$r" -eq 0 -o "$r" -eq 141 -o "$r" -eq 123 ] || { #!!!HACKISH
                error "inner command failed: $self [0]: $r"
                return 1
            }
        else
            v=''
        fi

        local i
        local P4=()
        local P5=()

        for ((k=0; k<${#P1[@]}; k++)); do
            n="${P1[k]}"
            [ "${n:0:1}" != '+' ] && continue
            if [ "${#n}" -eq 1 ]; then
                P1[$k]="$v"
            else
                i="${#P4[@]}"
                P4[$i]="${n:1}"
                P5[$i]="$k"
            fi
        done

        if [ "${#P4[@]}" -gt 0 ]; then
            v="${P4[@]}"
            c="\
$c0 --stat-${v// / --stat-} --plain-set --gnulib-hash|
awk -F '\t' -v col='${v// /,}' '$a'"
            [ "$x" == 'echo' ] && echo "$c"
            v2="$(eval "$c")"
            r="$?"
            [ "$r" -eq 0 -o "$r" -eq 141 -o "$r" -eq 123 ] || { #!!!HACKISH
                error "inner command failed: $self [1]: $r"
                return 1
            }
            P4=($v2) # stev: do not quote $v3
            if [ "${#P4[@]}" -ne "${#P5[@]}" ]; then
                error "inner command failed: $self [1]: unexpected result"
                return 1
            fi
            for ((i=0; i<${#P4[@]}; i++)); do
                P1[${P5[$i]}]="${P4[$i]}"
            done
        fi
    fi

    if [[ "$act" == [EFGLMRT] ]]; then
        c="\
find \\
-maxdepth 1 \\
-regextype posix-extended \\
-type f -regex '\./$sha1' -printf '%P\n'|
xargs -r grep -lE '^# --run-series$'|sort"

        local o
        local s=''
        # do not quote $base, $cfgl, $opta below
        for n in $cfgl $base $opta; do
            [[ "$n" == @(struct|set)-type ]] && {
                [ "$act" == 'E' ] && break
                [ -z "$s" ] && c+="|
xargs -r grep -hE '^$sha1$'"
                s='+'
            }
            assign2 v "${n//-/_}"
            [ -z "$v" ] && continue
            case "${v:0:1}" in
                =)	o='l' ;;
                !)	o='L' ;;
                *)	error "internal: unexpected ${n//-/_}='$v'"
                    return 1
                    ;;
            esac
            v2="$(ssed -R 's/'"$meta"'/\\\0/g' <<< "${v:1}")" || {
                error "inner command failed: ssed [0]"
                return 1
            }
            [[ "$n" == @(expr|separators) ]] && quote2 v2
            quote2 -i v2
            [[ "$n" == @(struct|set)-type ]] && n=''
            c+="|
xargs -r grep -${o}E '^# --${n:+$n}${v2:+${n:+=}$v2} '"
        done
        [ "$act" != 'E' -a -z "$s" ] && c+="|
xargs -r grep -hE '^$sha1$'"
    fi

    if [[ "$act" == [FGLMR] ]]; then
        v="$(ssed -R '
                s/-/_/g
                s/ {2,}/ /g
                s/\b[a-z0-9_]+\b/config_\0/g
                s/\bconfig_(32bit|debug)\b/\1/g
                s/\bconfig_(opt)imize\b/\1/' \
                <<< "${cfnl/ 32bit/ ${cfal} 32bit}")" &&
            [ -n "$v" ] || {
            error "inner command failed: ssed [1]"
            return 1
        }
        v2="$(tr 'a-z' 'A-Z' <<< "$v")" && [ -n "$v2" ] || {
            error "inner command failed: tr"
            return 1
        }
        local t2
        t2="$(ssed -R 's/'"$meta"'/\\\0/g' <<< "$t")" || {
            error "inner command failed: ssed [2]"
            return 1
        }
        local s2=''
        [ -n "$H" ] && s2+=$'
            1itest\t$$'
        s2+='
            /^# # meta command:\s*$/!b

            :0
            n
            $ba
            /^#/!b4
            /^# --(input|separators)=(.*?)\s+\\\s*$/b1
            /^# --(plain-set|path-trie)\s+\\\s*$/b2
            /^# --(ternary-tree|linear-hash|gnulib-hash)\s+\\\s*$/b3
            b0
            :1
            s//\1\t\2/p
            b0
            :2
            s//struct-type\t\1/p
            b0
            :3
            s//set-type\t\1/p
            b0

            :4
            /^\$\s*\.\/path-set\s+-C\s*$/b5
            n
            $ba
            b4

            :5
            n
            $ba
            /^\$/'
        [ -z "$p" ] && s2+='ba'
        [ -n "$p" ] && s2+='b6'
        s2+='
            /^('"${v2// /|}"'):\s*(.*?)\s*$/!b5
            s//\L\1\E\t\2/
            s/_/-/g
            p
            b5'$'\n'
        [ -n "$p" ] && s2+='
            :6
            /^\$\s*time\s+path-set-test\s*.*?\s-t\s*'"$t2"'\s*$/b7
            n
            $ba
            b6

            :7
            n
            $ba
            /^\$/ba
            /^(?:'"${p// /|}"')\s/!b7
            s/^(.*?)\s+.*?\s+.*?\s+(.*?)\s.*$/\1\t\2/p
            b7'$'\n'
        s2+='
            :a
            a'$'\n'
        c+="${c:+|}
xargs -r ${H:+-I '\$\$' }ssed -nRs '$s2'${H:+ '\$\$'}"
    fi

    if [[ "$act" == [FLMR] ]]; then
        local s2='
            function su_size(v,	n, s)
            {
                n = length(v)
                s = n > 0 ? S[toupper(substr(v, n))] : 0
                if (s > 0)
                    v = substr(v, 1, n - 1)
                return v * 1024 ** s
            }
            BEGIN {
                S["K"] = 1
                S["M"] = 2
                S["G"] = 3
                S["T"] = 4'
        local s3=''
        local s4=''

        local N=()
        local S=()
        local R0=()
        local R=()

        k=0
        # stev: do not quote $H, $v and $p2 below
        for n in ${H:+test} ${v//_/-} input separators struct-type set-type $p2; do
            v='+'
            [[ -z "$V" && "$n" != @(${H:+test${p:+|}}${p// /|}) ]] && {
                n2="${n//-/_}"
                [ "$n2" == 'opt' ] && n2='optimize'
                [[ "$n2" == @(32bit|debug|optimize) ]] && n2="config_$n2"
                if [ -z "$e" -o "${n2:0:7}" != 'config_' ]; then
                    assign2 v "$n2"
                    [ "${v:0:1}" != '=' ] &&
                    v='+' ||
                    v=''
                else
                    v=''
                fi
            }
            N[$k]="$n"
            S[$k]="$v"
            (( k++ ))
        done
        local i
        for ((k=0; k<${#N[@]}; k++)); do
            n="${N[k]}"
            [[ "$n" =~ ^([^%]+)%+$ ]] || continue
            v="${BASH_REMATCH[1]}"
            for ((i=0; i<${#P0[@]}; i++)); do
                [ "${P0[i]}" == "$n" ] && {
                    R0[$k]=$i
                    break
                }
            done
            for ((i=0; i<${#N[@]}; i++)); do
                [ "${N[i]}" == "$v" ] && {
                    R[$k]=$i
                    break
                }
            done
        done
        for ((k=0; k<${#S[@]}; k++)); do
            [ -z "${S[k]}" ] && continue
            i="${R0[k]}"
            [ -z "$i" ] &&
            n="${N[k]}" ||
            n="${P1[i]}"
            s2+='
                N['"$k"'] = "'"$n"'"'
            if [[ "$act" == [LM] ]]; then
                s3+=${s3:+$' \\'}$'\n\t\t\t\t\t''"%s\t%s\n"'
                s4+=${s4:+','}$'\n\t\t\t\t\t'"N[$k], v[$k]"
            else
                s3+=${s3:+'\t'}'%s'
                s4+=${s4:+', '}"v[$k]"
            fi
        done
        [[ "$act" != [LM] ]] && s3="\"$s3\\n\", \\"$'\n\t\t\t\t\t'
        [[ "$act" == [LM] ]] && s3=" \\$s3,"
        local j
        for ((k=0; k<${#N[@]}; k++)); do
            i="${R[k]}"
            [ -z "$i" ] && continue
            s2+='
                P['"$k"'] = '"$((i + 1))"
            j="${R0[k]}"
            [ -z "$j" ] && {
                error "internal: unexpected k='$k' i='$i'"
                return 1
            }
            s2+='
                R['"$k"'] = su_size("'"${P1[j]}"'")'
        done
        for ((k=0; k<${#N[@]}; k++)); do
            n="${N[k]}"
            [ "${n:$((${#n} - 1))}" != '%' ] && s2+='
                T["'"$n"'"] = '"$((k + 1))"
        done
        local a3
        [ "$act" == 'F' ] && a3='formatted' || a3='raw'
        [ "$act" == 'M' ] && a3+='-list' || a3+='-table'
        [[ "$act" != [LM] ]] && s2+='
                print_array(N)'
        s2+='
                reset_array(V)
            }
            function print_array(v)
            {'
        [[ "$act" == [LM] ]] && s2+='
                if (l ++) printf("\n")'
        s2+='
                printf('"$s3$s4"')
            }
            function reset_array(v,	i)
            {
                for (i = 0; i < '"${#N[@]}"'; i ++)
                    v[i] = "-"
            }
            function update_deps(v,	i, j)
            {
                for (i = 0; i < '"${#N[@]}"'; i ++) {
                    j = P[i]
                    if (j -- > 0 && A[j]) {
                        if (A[i])
                            error(sprintf( \
                                "internal: duplicated dep #%d", i))
                        V[i] = percents(R[i], V[j])
                        A[i] = 1
                    }
                }
            }
            function error(s)
            {
                printf( \
                    "'"$self"': '"$a3"': input error: %s\n", s) \
                    > "/dev/stderr"
                input_error = 1
                exit 1
            }
            function invalid_line(s)
            { error(sprintf("line #%d: %s: \x27%s\x27", FNR, s, $0)) }
            function percents(t, v,	r)
            {
                if (t == 0)
                    return "-"
                r = sprintf("%.0f", (t - v) / t * 100)
                if (r == "-0")
                    r = "0"
                return r "%"
            }
            {
                if (NF == 0) {
                    update_deps(V)
                    print_array(V)
                    reset_array(V)
                    delete A
                }
                else {
                    if (NF != 2)
                        invalid_line("unexpected line")
                    k = T[$1]
                    if (k -- == 0)
                        invalid_line("unexpected param")
                    if (A[k])
                        invalid_line("duplicated param")
                    V[k] = $2
                    A[k] = 1
                }
            }
            END {
                if (!input_error) {
                    if (FNR > 0 && NF > 0)
                        invalid_line("unexpected end line")
                }
            }'
        c+="|
awk -F '\t' '$s2'"
    fi

    if [[ "$act" == [FR] ]]; then
        local i
        local j=0
        local K=''
        local K2=''
        for ((k=0; k<${#S[@]}; k++)); do
            [ -z "$V" -a -z "${S[k]}" ] && continue
            (( j++ ))
            n="${N[k]}"
            [ "$n" == 'separators' ] && K2="$j"
            [[ "$n" != @(${p// /|}) || "${n:$((${#n} - 4))}" != '-mem' ]] && continue
            for ((i=0; i<${#P0[@]}; i++)); do
                [ "${P0[i]}" == "$n" ] && break
            done
            [ "$i" -ge "${#P0[@]}" ] && i="${#S[@]}"
            K+="${K:+ }$i:$j"
        done
        # stev: do not quote $K below
        [ -z "$K" ] ||
        K="$(printf '%s\n' $K|sort -t: -k1n,1 -s|ssed -nR '
                s/^\d+://
                s/^.*$/-k\0g,\0/
                H
                $!b
                g
                s/^\n//
                s/\n/ /g
                p')" || {
            error "internal: command failed: printf|sort|ssed"
            return 1
        }
        # stev: do not quote $K below
        [ -n "$K" ] && c+="|
ssed -R '1s/(?=[^\s])/\x1/g'|
sort -t $'\t' $K${K2:+ -k$K2,$K2} -s|
ssed -R '1s/\x1//g'"
    fi

    if [[ "$act" == [FM] ]]; then
        s2="$sn"'
            function num_fmt(n)
            { return (n ~ /^[0-9]+(\.[0-9]+)?$/) ? num(n) : n }'
        [ "$act" == "F" ] && s2+='
            function is_stat(n)
            { return match(n, /^('"${p// /|}"')$/) != 0 }'
        s2+='
            function is_mem(n)
            { return match(n, /-mem$|^'"$perx"'$/) != 0 }
            BEGIN {
                OFS = FS
            }'
        [ "$act" == "M" ] && s2+='
            function format_line()
            {
                if (NF > 0 && is_mem($1))
                    $2 = num_fmt($2)
            }'
        [ "$act" == "F" ] && s2+='
            function format_line(	i)
            {
                if (FNR == 1) {
                    for (i = 1; i <= NF; i ++) {
                        S[i] = is_stat($i)
                        M[i] = is_mem($i)
                    }
                }
                else {
                    for (i = 1; i <= NF; i ++) {
                        if (M[i])
                            $i = num_fmt($i)
                    }
                }
            }'
        [ "$act" == 'M' ] && s2+='
            function record_line(	n)
            {
                if (NF >= 1) {
                    n = length($1)
                    if (W < n)
                        W = n'
        [ "$act" == 'F' ] && s2+='
            function record_line(	i, n)
            {
                for (i = 1; i <= NF; i ++) {
                    n = length($i)
                    if (W[i] < n)
                        W[i] = n'
        s2+='
                }
                T[N ++] = $0
            }
            function print_table(	k, n, a, i)
            {
                for (k = 0; k < N; k ++) {'
        [ "$act" == 'M' ] && s2+='
                    if (length(T[k]) == 0) {
                        printf("\n")
                        continue
                    }'
        s2+='
                    n = split(T[k], a)
                    for (i = 1; i <= n; i ++) {
                        printf( \'
        [ "$act" == 'M' ] && s2+='
                            i == 1 ? "%s:%*s%s" : "%s%.*s%s", \
                            a[i], i == 1 ? W - length(a[1]) : 0, "", \'
        [ "$act" == 'F' ] && s2+='
                            S[i] || M[i] ? "%*s%s" : "%-*s%s", \
                            S[i] || M[i] || i < n ? W[i] : 0, a[i], \'
        s2+='
                            i < n ? "  " : "\n")
                    }
                }
            }
            {
                format_line()
                record_line()
            }
            END {
                print_table()
            }'

        c+="|
awk -F '\t' '$s2'"
    fi
    $x "$c"
}

git-repo-diff()
{
    local self="git-repo-diff"
    local trgx='@(json-type|gnulib)'
    local defs='README'
    local deft='json-type'

    local x="eval"
    local b=""          # pass '-b|--ignore-space-change' to diff (--ignore-space-change)
    local g="+"         # 'git' repo directory (default: '$HOME/$target') (--git-dir=DIR)
    local s="+"         # sha1 hashes file name ('-' means stdin, the default is 'README') (--sha1-hashes=FILE)
    local t="+"         # target name: 'json-type' or 'gnulib' (default: 'json-type') (--target=NAME|--json-type|--gnulib)
    local u=""          # pass '-u|--unified=NUM' to diff (--unified=NUM)

    local opt
    local OPT
    local OPTN
    local opts=":bdg:s:t:u:x-:"
    local OPTARG
    local OPTERR=0
    local OPTIND=1
    while getopts "$opts" opt; do
        # discriminate long options
        optlong

        # translate long options to short ones
        test -n "$OPT" &&
        case "$OPT" in
            ignore-space-change)
                opt='b' ;;
            git-dir)
                opt='g' ;;
            sha1-hashes)
                opt='s' ;;
            target|$trgx)
                opt='t' ;;
            unified)
                opt='u' ;;
            *)	error --long -o
                return 1
                ;;
        esac

        # check long option argument
        [[ "$opt" == [t] ]] ||
        optlongchkarg ||
        return 1

        # handle short options
        case "$opt" in
            d)	x="echo"
                ;;
            x)	x="eval"
                ;;
            [gs])
                optarg
                ;;
            [b])
                optopt
                ;;
            t)	[[ -n "$OPT" || "$OPTARG" == $trgx ]] || {
                    error -i
                    return 1
                }
                optlong -

                [[ "${OPT:2}" == @(target|$trgx) ]] || {
                    error --long -o
                    return 1
                }
                case "${OPT:2}" in
                    target)
                        [ -n "$OPTN" ] || {
                            error --long -a
                            return 1
                        }
                        [[ "$OPTARG" == $trgx ]] || {
                            error --long -i
                            return 1
                        }
                        t="$OPTARG"
                        ;;
                    $trgx)
                        [ -z "$OPTN" ] || {
                            error --long -d
                            return 1
                        }
                        t="${OPT:2}"
                        ;;
                    *)	error "internal: unexpected OPT='$OPT'"
                        return 1
                        ;;
                esac
                ;;
            u)	[[ "$OPTARG" == +([0-9]) ]] || {
                    error --long -i
                    return 1
                }
                optarg
                ;;
            *)	error --long -g
                return 1
                ;;
        esac
    done
    shift $((OPTIND - 1))

    [ "$t" == '+' ] && t="$deft"

    [ -n "$g" ] || {
        error "git directory cannot be null"
        return 1
    }
    [ "$g" == '+' ] && g="$HOME/$t"
    [ -d "$g" ] || {
        error "git directory '$g' not found"
        return 1
    }
    quote g

    [ -n "$s" ] || {
        error "sha1 hashes file name cannot be null"
        return 1
    }
    [ "$s" == '-' ] && s=""
    [ "$s" == '+' ] && s="$defs"
    [ -z "$s" -o -f "$s" ] || {
        error "sha1 hashes file '$s' not found"
        return 1
    }
    quote s

    local c="\
git \\
--git-dir=${g%%+(/)?(.git*(/))}/.git rev-parse \\
--git-dir"
    [ "$x" == "echo" ] && echo "$c"

    local g2
    g2="$(eval "$c")" && [ -n "$g2" ] || {
        error "inner command failed: git" #!!! ${c//@(\\|$'\n')/}"
        return 1
    }
    quote g2

    local s2='
    /^\s+SHA1\s+hash\s+file\s+name\s*$/!b
    :0
    n
    /^\s*$/q
    s/^\s*([0-9a-f]{40})\s+'"$t"'\/(.*?)\s*$/\1\t\2/p
    b0'

    local h
    local f

    c="\
ssed -nR '$s2'${s:+ \\
$s}|
while read h f; do
    git --git-dir=$g2 cat-file -e \"\$h\" || {
        error \"git object \$h not found\"
        continue
    }
    diff -${b}u$u \\
    -L \"\$f\" <(git --git-dir=$g2 cat-file -p \$h) \\
    -L \"\$f\" \"lib/$t/\$f\"
done"

    $x "$c"
}

