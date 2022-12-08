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

parse() {
    local -i _part="$1"
    local -a _arr
    declare -ig res=0

    while IFS=-, read -ra _arr; do
        # shellcheck disable=2068
        set -- ${_arr[@]}
        if (( _part == 1 )); then
            (( ( ($1 >= $3 && $2 <= $4) || ($1 <= $3 && $2 >= $4) ) && res++ ))
        else
            (( ( ($1 >= $3 && $1 <= $4) || ($1 <= $3 && $2 >= $3) ) && res++ ))
        fi
    done
}

solve() {
    :
}

main "$@"
exit 0
