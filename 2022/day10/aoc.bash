#!/usr/bin/env bash
#
# aoc.bash: Advent of Code 2022, day 10
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

declare -i cycle=0 regx=1

draw() {
    local -i pos

    (( pos = cycle % 40 - regx - 1, pos = pos < 0? -pos: pos ))
    (( pos < 2 )) && res+="#" || res+="."
    if ! (( cycle % 40 )); then
        res+=$'\n'
    fi
}

tick() {
    (( cycle ++ ))
    if (( part == 1 )); then
        (( (cycle + 20) % 40 )) || (( res += cycle * regx ))
    else
        draw
    fi
}

do_noop() {
    tick
}

do_add() {
    tick
    tick
    (( regx += $1 ))
}

parse() {
    while read -r instr value; do
        # not too hacky, to prepare for next puzzles ;-)
        case "$instr" in
            "addx")
                do_add "$value"
                ;;
            "noop")
                do_noop
                ;;
        esac
    done
}

solve() {
    # remove last '\n', add starting '\n'
    (( part == 2 )) && res=$'\n'${res::-1}
}

main "$@"
exit 0
