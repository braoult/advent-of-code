#!/usr/bin/env bash
#
# aoc.bash: Advent of Code 2022, day 8
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

declare -a trees=()                              # trees height
declare -Ai visible=()                            # visible if set
declare -i size

parse() {
    readarray -t trees
    size=${#trees[@]}
}

# height - return height of a tree
# $1: reference of return value
# $2, $3: x, y
# x is column, y is row
height() {
    local -n _ret="$1"
    _ret=${trees[$3]:$2:1}
}

check_visible() {
    local -n _max="$1"
    local -i _x="$2" _y="$3" _c

    height _c "$x" "y"
    if (( _c > _max )); then
        (( visible[$x-$y]++, _max=_c ))
        (( _max == 9 )) && return 1
    fi
    return 0
}

part1() {
    declare -ig res
    local -i x y max

    for ((y = 1; y < size -1; ++y)); do           # to east
        height max 0 "$y"
        (( max == 9 )) && continue
        for ((x = 1; x < size -1; ++x)); do
            check_visible max "$x" "$y" || break
        done
    done
    for ((y = 1; y < size -1; ++y)); do           # to west
        height max $((size - 1)) "$y"
        (( max == 9 )) && continue
        for ((x = size - 2; x > 0; --x)); do
            check_visible max "$x" "$y" || break
        done
    done
    for ((x = 1; x < size -1; ++x)); do           # to south
        height max "$x" 0
        (( max == 9 )) && continue
        for ((y = 1; y < size -1; ++y)); do
            check_visible max "$x" "$y" || break
        done
    done
    for ((x = 1; x < size -1; ++x)); do           # to north
        height max "$x" $((size - 1))
        (( max == 9 )) && continue
        for ((y = size - 2; y > 0; --y)); do
            check_visible max "$x" "$y" || break
        done
    done
    (( res = ${#visible[@]} + size * 4 - 4 ))
}

check_tree() {
    local -n res=$1
    local -i X="$2" Y="$3" c x y h
    local -ai vis=()

    height h "$X" "$Y"
    for ((x = X + 1; x < size ; ++x)); do         # east
        height c "$x" "Y"
        (( vis[0]++ ))
        ((c >= h)) && break
    done
    for ((x = X - 1; x >= 0; --x)); do            # west
        height c "$x" "Y"
        (( vis[1]++ ))
        ((c >= h)) && break
    done
    for ((y = Y + 1; y < size; ++y)); do          # south
        height c "$X" "$y"
        (( vis[2]++ ))
        ((c >= h)) && break
    done
    for ((y = Y - 1; y >= 0; --y)); do            # north
        height c "$X" "$y"
        (( vis[3]++ ))
        ((c >= h)) && break
    done
    # shellcheck disable=1102
    res=$(( "${vis[@]/%/ *}" 1))
}


part2() {
    local -ig res=0
    local -i tmp=1 x y

    for ((x = 1; x < size - 1; ++x)); do
        for ((y = 1; y < size - 1; ++y)); do
            check_tree tmp "$x" "$y"
            if ((tmp > res)); then
                ((res = tmp))
            fi
        done
    done
}

solve() {
    if ((part == 1)); then
        part1
    else
        part2
    fi
}

main "$@"
exit 0
