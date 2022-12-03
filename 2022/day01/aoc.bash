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

declare -a elf
declare -i maxwanted=3

parse() {
    local -i tot=0 i res

    while true; do
        read -r line
        res=$?
        # number found: sum & continue
        [[ -n $line ]] && (( tot += line)) && continue
        # first elf: we set it and continue
        (( !${#elf[@]} )) && (( elf[0] = tot )) && continue
        # find a place to (maybe) insert new high (keep array sorted)
        for (( i = 0; i < ${#elf[@]}; ++i )); do
            if (( tot > elf[i] )); then         # normal insert (tot > old value)
                elf=( "${elf[@]:0:i}" "$tot" "${elf[@]:i}" )
                break
            elif (( i == ${#elf[@]}-1 )); then  # insert at end
                elf+=( "$tot" )
                break
            fi
        done
        # keep array size <= maxwanted
        elf=( "${elf[@]:0:maxwanted}" )
        (( tot = 0 ))
        ((res > 0)) && break                      # EOF
    done
}

solve() {
    local -i part="$1"
    if ((part == 1)); then
        (( res = elf[0] ))
    else
        (( res = elf[0] + elf[1] + elf[2] ))
    fi
}

main "$@"
exit 0
