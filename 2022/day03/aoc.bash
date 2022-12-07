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
    local input half1 half2 result=""
    local -i len half i j=1 prio part1=0 line=1

    while read -r input; do
        (( len = ${#input}, half = len/2 ))
        half1="${input:0:half}"
        half2="${input:half}"
        result=""

        #printf "[%d] l=%d h=%d [%s / %s]\n" "$line" "$len" "$half" "$half1" "$half2"

        #c="${half1//[^${half2}]}"
        #prio prio "${c:0:1}"
        #(( part1 += prio ))

        for ((i = 0; i < half; ++i)); do
            c=${half1:$i:1}
            if [[ $result != *$c* && $half2 == *$c* ]]; then
                echo "found $c"
                result=$result$c
                prio prio "${c:0:1}"
                (( part1 += prio ))
                #printf "%d prio(%c)=%d tot=%d\n" "$line" "$c" "$prio" "$part1"
            fi
        done
        ((j++, line++))
        echo
    done
    echo "result=$part1"
}

solve() {
    res="${tot[$1]}"
}

main "$@"
exit 0
