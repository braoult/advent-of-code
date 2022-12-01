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

declare -a total
declare max=0
declare -i res=0

parse() {
    local -i elf=0

    while read -r line; do
        if [[ -n $line ]]; then
            (( total[elf] += line))
        else
            (( total[elf] > max )) && (( max = total[elf] ))
            ((elf++))
        fi
    done
}

part1() {
    res=$max
}

part2() {
    local -i i elf newbest

    for ((i=0; i<3; ++i)); do
        newbest=0
        for ((elf=0; elf<${#total[@]}; ++elf)); do
            (( total[elf] > total[newbest] )) && newbest=$elf
        done
        (( res+=total[newbest] ))
        unset "total[$newbest]"                   # remove current max
        total=("${total[@]}")                     # pack array
    done
}

main "$@"
printf "%s: res=%s\n" "$cmdname" "$res"
exit 0
