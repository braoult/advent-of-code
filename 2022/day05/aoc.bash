#!/usr/bin/env bash
#
# aoc.bash: Advent of Code 2022, day 5
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

    while IFS= read -r _input; do
        if [[ -z ${_input} || ${_input:1:1} == "1" ]]; then
            _state=2
            continue
        fi
        if (( _state == 1 )); then                # stacks description
            for (( i = 0, _queue = 0; i < ${#_input}; i += 4, _queue++ )); do
                sub=${_input:i:4}
                [[ $sub == " "* ]] || stacks[$_queue]+="${sub:1:1}"
            done
        else                                      # moves description
            rules+=( "${_input//[a-z]/}" )
        fi
    done
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
