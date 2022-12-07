#!/usr/bin/env bash
#
# aoc.bash: Advent of Code 2022, day 1
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

declare -A outcome=(                              # shape is known
    [A X]=$((3+1)) [A Y]=$((6+2)) [A Z]=$((0+3))
    [B X]=$((0+1)) [B Y]=$((3+2)) [B Z]=$((6+3))
    [C X]=$((6+1)) [C Y]=$((0+2)) [C Z]=$((3+3))
)
declare -A outcome2=(                              # result is known
    [A X]=$((0+3)) [A Y]=$((3+1)) [A Z]=$((6+2))
    [B X]=$((0+1)) [B Y]=$((3+2)) [B Z]=$((6+3))
    [C X]=$((0+2)) [C Y]=$((3+3)) [C Z]=$((6+1))
)
declare -a tot

parse() {
    local input

    while read -r input; do
        (( tot[1] += outcome[$input] ))
        (( tot[2] += outcome2[$input] ))
    done
}

solve() {
    res="${tot[$1]}"
}

main "$@"
exit 0
