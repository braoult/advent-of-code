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

declare -a stacks=() rules=()

parse() {
    local -i _queue _state=1
    local _input
    local -a _parse
    local cur=""
    local -i lcur=0 i

    read -r _input
    len=${#_input}

    lcur=0
    for (( i = 0; i < len - 4; ++i )); do
        echo
        nextc=${_input:i:1}                       # get next char
        printf "next=%s cur=%s lcur=%d\n" "$nextc" "$cur" "$lcur"
        for ((j = 0; j < lcur; ++j)); do          # compare with previous ones
            printf "compare %s with %d:%s\n" "$nextc" "$j" "${cur:j:1}"
            if [[ $nextc == "${cur:j:1}" ]]; then
                cur=${cur:j+1}$nextc
                (( lcur = lcur - (j + 1) + 1))
                printf "\t-> equal cur=%d:%s" "$lcur" "$cur"
                continue 2
            fi
        done
        cur+=$nextc
        ((lcur++))
        ((lcur == 4)) && break
    done
    echo "$cur" $((i + 1))
}

solve() {
    local -i _part="$1" _i=0 _j=0 _nb _from _to
    local -a _rule

    for ((; _i < ${#rules[@]}; ++_i )); do
        read -ra _rule <<< "${rules[$_i]}"
        (( _nb = _rule[0], _from = _rule[1] - 1, _to = _rule[2] - 1 ))
        if ((_part == 1)); then
            # move elements one by one
            for ((_j = 0; _j < _nb; ++_j)); do
                stacks[$_to]="${stacks[$_from]:0:1}${stacks[$_to]}"
                stacks[$_from]=${stacks[$_from]:1}
            done
        else
            # move elements by block
            stacks[$_to]="${stacks[$_from]:0:_nb}${stacks[$_to]}"
            stacks[$_from]=${stacks[$_from]:_nb}
        fi
    done
    printf -v res "%c" "${stacks[@]}"
}

main "$@"
exit 0
