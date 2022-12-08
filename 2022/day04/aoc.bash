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

export LANG=C
declare -a sections

parse() {
    local -a _arr
    local -i _tmp

    while IFS=-, read -ra _arr; do
        # arrange the two sections so that the lowest is the first
        printf "_arr=%s\n" "${_arr[*]}"
        if ((_arr[0] > _arr[2])); then
            ((_tmp=_arr[0], _arr[0]=_arr[2], _arr[2]=_tmp))
            ((_tmp=_arr[1], _arr[1]=_arr[3], _arr[3]=_tmp))
            printf "\t->%s\n" "${_arr[*]}"
        fi
        sections+=("${_arr[*]}")
    done
    #readarray -t input
}

part1() {
    declare -ig res=0
    local -a _sect

    for line in "${sections[@]}"; do
        # shellcheck disable=SC2206
        _sect=($line)
        (( _sect[1] >= _sect[3] ||
               (_sect[0] == _sect[2] && _sect[3] >= _sect[1]) )) && (( res++ ))
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
