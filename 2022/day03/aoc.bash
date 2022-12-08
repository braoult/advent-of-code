#!/usr/bin/env bash
#
# aoc.bash: Advent of Code 2022, day 3
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

declare -a input

# prio - get priority value for an item
# $1: reference of variable for result
# $2: string
prio() {
    local -n _val="$1"
    local _char="${2:0:1}"

    printf -v _val "%d" "'$_char"
    if [[ $_char > "Z" ]]; then                   # a-z
        ((_val += (1 - 97) ))
    else                                          # A-Z
        ((_val += (27 - 65) ))
    fi
}

# common=chars - get common characters in two strings
# $1: reference of variable for result
# $2, $3: The two strings
common_chars() {
    local -n _res="$1"

    _res="${2//[^$3]}"
}

parse() {
    readarray -t input
}

part1() {
    local line half1 half2 common
    local -i half i prio
    declare -ig res=0

    for line in "${input[@]}"; do
        (( half = ${#line} / 2 ))
        half1="${line:0:half}"
        half2="${line:half}"
        common="${half1//[^${half2}]}"
        prio prio "${common}"
        (( res += prio ))
    done
}

part2() {
    local one two three common
    local -i i prio
    declare -ig res=0

    for (( i = ${#input[@]} - 1; i > 1; i -= 3)); do
        three=${input[i]}
        two=${input[i - 1]}
        one=${input[i - 2]}
        common_chars common "$three" "$two"
        common_chars common "$common" "$one"
        prio prio "${common}"
        (( res += prio ))
    done
}

solve() {
    if (($1 == 1)); then
        part1
    else
        part2
    fi
}

main "$@"
exit 0
