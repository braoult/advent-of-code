#!/usr/bin/env bash
#
# aoc.bash: Advent of Code 2022, day 6
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

declare input                                     # datastream
declare -i len                                    # len of datastream

parse() {
    read -r input
    len=${#input}
}

solve() {
    declare -ig res
    local -i mark=4 lcur=0 part="$1"
    local cur next

    ((part == 2)) && mark=14
    for (( i = 0; i < len - mark; ++i )); do      # loop on all chars
        next=${input:i:1}                         # next char
        for ((j = 0; j < lcur; ++j)); do          # compare with previous ones
            if [[ $next == "${cur:j:1}" ]]; then  # duplicate
                cur="${cur:j+1}$next"             # keep str after dup + new char
                (( lcur = lcur - j ))
                continue 2
            fi
        done
        cur+="$next"                              # add new char
        ((++lcur == mark)) && break               # mark len found
    done
    ((res = i + 1))
}

main "$@"
exit 0
