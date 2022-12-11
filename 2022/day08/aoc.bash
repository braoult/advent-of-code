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

        # y=${trees[0]:i:1}
        # xrev=${trees[i]:size-1:1}
        # yrev=${trees[size-1]:i:1}
        # printf "x=%d\n" "$x"
        # printf "y=%d\n" "$y"
        # printf "xrev=%d\n" "$xrev"
# printf "yrev=%d\n" "$yrev"

# height - return height of a tree
# $1: reference of return value
# $2, $3: x, y
# x is column, y is row
height() {
    local -n _ret="$1"
    local -i _x="$2" _y="$3"
    _ret=${trees[_y]:_x:1}
}

check_visible() {
    local -n _max="$1"
    local -i _x="$2" _y="$3" _c
    echo "m=$_max x=$x y=$y"
    height _c "$x" "y"
    printf "(%d,%d)=%d\n" "$x" "$y" "$_c"
    if (( _c > _max )); then
        printf "right visible (%d,%d)=%d\n" "$x" "$y" "$_c"
        (( visible[$x-$y]++, _max=_c ))
        (( _max == 9 )) && return 1
    fi
    return 0
}

part1() {
    declare -ig res
    local -i x y max c

    # left to right
    for ((y = 1; y < size -1; ++y)); do           # row
        height max 0 "$y"
        printf "**** left=%d max=%d\n" "$i" "$max"
        (( max == 9 )) && continue
        for ((x = 1; x < size -1; ++x)); do       # column
            check_visible max "$x" "$y" || break
        done
    done
    echo
    # right to left
    for ((y = 1; y < size -1; ++y)); do           # row
        height max $((size - 1)) "$y"
        printf "**** right=%d max=%d\n" "$i" "$max"
        (( max == 9 )) && continue
        for ((x = size - 2; x > 0; --x)); do       # column
            check_visible max "$x" "$y" || break
        done
    done
    echo
    # top to bottom
    for ((x = 1; x < size -1; ++x)); do           # column
        height max "$x" 0
        printf "**** top=%d max=%d\n" "$i" "$max"
        (( max == 9 )) && continue
        for ((y = 1; y < size -1; ++y)); do       # column
            check_visible max "$x" "$y" || break
        done
    done
    echo
    # bottom to top
    for ((x = 1; x < size -1; ++x)); do           # row
        height max "$x" $((size - 1))
        printf "**** bottom=%d max=%d\n" "$i" "$max"
        (( max == 9 )) && continue
        for ((y = size - 2; y > 0; --y)); do       # column
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
    printf "********** part2(%d,%d) h=%d\n" "$X" "$Y" "$h"

    # east
    for ((x = X + 1; x < size ; ++x)); do
        height c "$x" "Y"
        printf "(%d,%d)=%d " "$x" "$Y" "$c"
        (( vis[0]++ ))
        ((c >= h)) && break
    done
    (( res *= vis[0] ))
    echo
    printf "east=%d\n" "${vis[0]}"

    # west
    for ((x = X - 1; x >= 0; --x)); do
        height c "$x" "Y"
        printf "(%d,%d)=%d " "$x" "$Y" "$c"
        (( vis[1]++ ))
        ((c >= h)) && break
    done
    (( res *= vis[1] ))
    echo
    printf "west=%d\n" "${vis[1]}"

    # south
    for ((y = Y + 1; y < size; ++y)); do
        height c "$X" "$y"
        printf "(%d,%d)=%d " "$X" "$y" "$c"
        (( vis[2]++ ))
        ((c >= h)) && break
    done
    (( res *= vis[2] ))
    echo
    printf "south=%d\n" "${vis[2]}"

    # north
    for ((y = Y - 1; y >= 0; --y)); do
        height c "$X" "$y"
        printf "(%d,%d)=%d " "$X" "$y" "$c"
        (( vis[3]++ ))
        ((c >= h)) && break
    done
    (( res *= vis[3] ))
    echo
    printf "north=%d\n" "${vis[3]}"
    # shellcheck disable=1102
    res=$(( "${vis[@]/%/ *}" 1))
    printf "res(%d,%d)=%d * %d * %d * %d = %d\n" "$X" "$Y" "${vis[0]}" "${vis[1]}" "${vis[2]}" "${vis[3]}" "$res"
    echo
}


part2() {
    local -ig res=0
    local -i tmp=1 x y

    for ((x = 1; x < size - 1; ++x)); do
        for ((y = 1; y < size - 1; ++y)); do
            #echo "ZOB $x $y"
            check_tree tmp "$x" "$y"
            if ((tmp > res)); then
                ((res = tmp))
                printf "NEW MAX at (%d,%d)=%d\n" "$x" "$y" "$tmp"
            fi
        done
    done

    #check_tree tmp 2 3
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

    for k in "${!visible[@]}"; do
        printf "k=%s %d\n" "$k" "${visible[$k]}"
    done
    echo
    for (( i = 1; i < size - 1; ++i )); do
        max=${trees[i]:size-1:1}
        printf "i=%d max=%d\n" "$i" "$max"
        for (( j = size - 2; j > 0; --j )); do
            if (( ${trees[i]:j:1} > max)); then
                printf "(%d,%d)=%d max=%d\n" "$i" "$j" "${trees[i]:j:1}" "$max"
                (( visible[$i-$j]++ ))
                max=${trees[i]:j:1}
                printf "new max=%d\n" "$max"
                (( max == 9 )) && break
            fi
        done
    done
    echo
    for k in "${!visible[@]}"; do
        printf "k=%s %d\n" "$k" "${visible[$k]}"
    done
    res=${#visible[@]}
