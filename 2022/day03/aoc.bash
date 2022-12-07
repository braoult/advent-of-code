#!/usr/bin/env bash
#
# aoc.bash: Advent of Code 2022, day 2
#
# Copyright (C) 2022 Bruno Raoult ("br")
# Licensed under the GNU General Public License v3.0 or later.
# Some rights reserved. See COPYING.
#
# You should have received a copy of the GNU General Public License along with this
# program. If not, see <https://www.gnu.org/licenses/gpl-3.0-standalone.html>.
#
# SPDX-License-Identifier: GPL-3.0-or-later <https://spdx.org/licenses/GPL-3.0-or-later.html>

. common.bash

export LANG=C
declare -a tot=(1 2)
declare -a input

# get priority value for an item
# $1: character
prio() {
    local -n _val="$1"
    local _char="$2"

    printf -v _val "%d" "'$_char"
    if [[ $_char > "Z" ]]; then                   # a-z
        ((_val += (1 - 97) ))
    else                                          # A-Z
        ((_val += (27 - 65) ))
    fi
}

parse() {
    readarray -t input
}

part1() {
    local line half1 half2 result=""
    local -i len half i prio

    res=0
    for line in "${input[@]}"; do
        echo "$line"
        (( len = ${#line}, half = len/2 ))
        half1="${line:0:half}"
        half2="${line:half}"
        result=""

        printf "[%d] l=%d h=%d [%s / %s]\n" "$num" "$len" "$half" "$half1" "$half2"

        #c="${half1//[^${half2}]}"
        #prio prio "${c:0:1}"
        #(( part1 += prio ))

        for (( i = 0; i < half; ++i )); do
            c=${half1:$i:1}
            if [[ $result != *$c* && $half2 == *$c* ]]; then
                echo "found $c"
                result=$result$c
                prio prio "${c:0:1}"
                (( res += prio ))
                #printf "%d prio(%c)=%d tot=%d\n" "$line" "$c" "$prio" "$part1"
            fi
        done
    done
}

part2() {
    local common one two three
    local -i len half i prio

    res=0
    for (( i = ${#input[@]} - 1; i > 1; i -= 3)); do
        three=${input[i]}
        two=${input[i - 1]}
        one=${input[i - 2]}
        common=${three//[^${two}]}
        common=${common//[^${one}]}
        #common=${}
        #echo "$line"
        #(( len = ${#line}, half = len/2 ))
        #half1="${line:0:half}"
        #half2="${line:half}"
        #result=""

        printf "[%d] 1=%s 2=%s 3=%s c=%s\n" "$i" "$one" "$two" "$three" "$common"

        #c="${half1//[^${half2}]}"
        prio prio "${common:0:1}"
        (( res += prio ))
    done
}

solve() {
    if (($1 == 1)); then
        part1
    else
        part2
    fi
    #res="${tot[$1]}"
}

main "$@"
exit 0
