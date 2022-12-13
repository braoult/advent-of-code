#!/usr/bin/env bash
#
# aoc.bash: Advent of Code 2022, day 9
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

declare -i xh=0 yh=0 xt=0 yt=0
declare -A visited=()

add_pos() {
    (( visited["$1-$2"]++ ))
}

direction() {
    local -n _dx="$1" _dy="$2"
    local dir="$3"
    (( _dx=0, _dy=0 ))
    case "$dir" in
        L) _dx=-1 ;;
        R) _dx=1 ;;
        U) _dy=1 ;;
        D) _dy=-1 ;;
    esac
}

move_t() {
    local -i dx dy sx sy

    (( dx = xh - xt, dy = yh - yt ))
    (( sx = dx > 0? 1: -1 ))
    (( sy = dy > 0? 1: -1 ))

    #printf " [move_t: dx=%d dy=%d " "$dx" "$dy"
    if (( sx * dx > 1 || sy * dy > 1)); then
        (( dx > 0 )) && (( xt++ ))
        (( dx < 0 )) && (( xt-- ))
        (( dy > 0 )) && (( yt++ ))
        (( dy < 0 )) && (( yt-- ))
    fi
    #printf "xt=%d yt=%d] " "$xt" "$yt"
    add_pos "$xt" "$yt"
}

move_h() {
    local -i dx="$1" dy="$2" n="$3" i
    #printf "\tmove_h dx=%d dy=%d n=%d\n" "$dx" "$dy" "$n"
    for ((i = 0; i < n; ++i)); do
        (( xh += dx, yh += dy ))
        #printf -- "(%d,%d)" "$xh" "$yh"
        move_t
        #printf -- "/(%d,%d)\n" "$xt" "$yt"
    done
}

parse() {
    local dir moves dx dy

    while read -r dir moves; do
        #printf "(%d,%d)/(%d,%d) %s/%d\n" "$xh" "$yh" "$xt" "$yt" "$dir" "$moves"
        direction dx dy "$dir"
        #printf "\tafter direction dx=%d dy=%d\n" "$dx" "$dy"
        move_h "$dx" "$dy" "$moves"
        #printf "\th: (%d,%d)/(%d,%d)\n" "$xh" "$yh" "$xt" "$yt"
        add_pos "$xt" "$yt"
    done
}

part1() {
    declare -ig res
    parse
    #echo FINAL
    #echo "${!visited[@]}"
    res=${#visited[@]}
}

part2() {
    :
}

solve() {
    add_pos 0 0
    if ((part == 1)); then
        part1
    else
        part2
    fi
}

main "$@"
exit 0
