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

move() {
    local -i _dx="$1" _dy="$2" _m="$3" _i _n
    local -i __dx __dy __sx __sy
    local -n _l="h$last"

    for ((_i = 0; _i < _m; ++_i)); do             # for each move
        (( h0[0] += _dx, h0[1] += _dy ))          # head move
        for (( _n = 0; _n < last; ++_n )); do     # for each other node
            local -n _h="h$_n" _t="h$((_n+1))"
            (( __dx = _h[0] - _t[0], __dy = _h[1] - _t[1] ))
            (( __sx = __dx? __dx > 0? 1: -1 : 0 ))
            (( __sy = __dy? __dy > 0? 1: -1 : 0 ))
            if (( __sx * __dx > 1 || __sy * __dy > 1)); then
                (( _t[0] += __sx ))
                (( _t[1] += __sy ))
            fi
        done
        visited["${_l[0]}"/"${_l[1]}"]=1
    done
}

parse() {
    local dir moves dx dy

    (( $1 == 2)) && last=9
    while read -r dir moves; do
        dx=0
        dy=0
        case "$dir" in
            L) dx=-1 ;;
            R) dx=1 ;;
            U) dy=1 ;;
            D) dy=-1 ;;
        esac
        move "$dx" "$dy" "$moves"
    done
}

solve() {
    res=${#visited[@]}
}

main "$@"
exit 0
