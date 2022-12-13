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

declare -a h{0..10}                               # h0 is head
declare -A visited=()                             # keep count of visited
declare -i last=1                                 # tail (last knot)

add_pos() {
    local -n queue="$1"
    visited["${queue[0]}"/"${queue[1]}"]=1
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

do_tail() {
    local -n _h="$1" _t="$2"
    local -i dx dy sx sy

    (( dx = _h[0] - _t[0], dy = _h[1] - _t[1] ))
    (( sx = dx > 0? 1: -1 ))
    (( sy = dy > 0? 1: -1 ))
    if (( sx * dx > 1 || sy * dy > 1)); then
        (( dx > 0 )) && (( _t[0]++ ))
        (( dx < 0 )) && (( _t[0]-- ))
        (( dy > 0 )) && (( _t[1]++ ))
        (( dy < 0 )) && (( _t[1]-- ))
    fi
}

move_t() {
    local -i _n

    for (( _n = 0; _n < last; ++_n )); do
        do_tail "h$_n" "h$((_n + 1))"
    done
    add_pos "h$last"
    return
}

move_h() {
    local -i _dx="$1" _dy="$2" _m="$3" _i _n
    for ((_i = 0; _i < _m; ++_i)); do
        (( h0[0] += _dx, h0[1] += _dy ))
        move_t
    done
}

parse() {
    local dir moves dx dy
    (( last = $1 == 1? 1: 9 ))
    while read -r dir moves; do
        direction dx dy "$dir"
        move_h "$dx" "$dy" "$moves"
    done
}

part1() {
    local -n _t="h$last"
    res=${#visited[@]}
}

part2() {
    local -n _t="h$last"
    res=${#visited[@]}
}


solve() {
    local part="$1"
    add_pos "h$last"
    if ((part == 1)); then
        part1
    else
        part2
    fi
}

main "$@"
exit 0
