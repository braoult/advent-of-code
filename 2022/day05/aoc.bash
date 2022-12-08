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

declare -a stacks=() rules=()

printall() {
    local i
    for (( i = 0; i < ${#stacks[@]}; ++i )); do
        printf "stack %d: %s\n" "$i" "${stacks[i]}"
    done
    #for (( i = 0; i < ${#rules[@]}; ++i )); do
    #    printf "rule %d: %s\n" "$i" "${rules[i]}"
    #done
}

parse() {
    local -i _part="$1" _queue li _state=1
    local _input
    local -a _parse
    declare -ig _res=0

    li=1
    while IFS= read -r _input; do
        if [[ -z ${_input} || ${_input:1:1} == "1" ]]; then
            printf "\tzobi\n"
            _state=2
            li=0
            continue
        fi
        case $_state in
            1)                                    # stacks description
                # get blocks of 4 characters
                #echo GGG
                for ((i=0, _queue=0; i<${#_input}; i+=4, _queue++)); do
                    #printf "line=%d queue=%d\n" "$li" "$_queue"
                    sub=${_input:i:4}
                    if [[ $sub == "    " ]]; then
                        :
                        #printf "\t SKIP line %d queue\n" "$li" "$_queue"
                    else
                        # printf "\tline %d queue %d CHAR %s\n" "$li" "$_queue" "${sub:1:1}"
                        stacks[$_queue]+="${sub:1:1}"
                    fi
                done
                ;;
            2)                                    # moves description
                #echo HHH
                read -ra _parse <<<"$_input"
                #printf "RULE: %d\n" ${#_parse[@]}
                #printf "rule %d: %d %d %d\n" "$li" "${_parse[1]}" "${_parse[3]}" "${_parse[5]}"
                rules[$li]="${_parse[1]} ${_parse[3]} ${_parse[5]}"
        esac

        ((li++))
    done
    #printall
}

part1() {
    local -i _i _j _nb _from
    local -a _rule
    local _tmp
    printall
    for (( _i=0; _i<${#rules[@]}; ++_i )); do
        # shellcheck disable=2086
        read -ra _rule <<< ${rules[$_i]}
        (( _nb = _rule[0] ))
        ((_from = _rule[1] - 1 ))
        ((_to = _rule[2] - 1 ))
        #printf "rule=%d nb=%d from=%d=%s to=%d=%s\n" "$_i" "$_nb" "$_from" "${stacks[$_from]}" "$_to" "${stacks[$_to]}"
        #printf "_rule=%d nb=%s from=%s/%s to=%s/%s\n" ${#_rule[@]} "${_rule[0]}" "${_rule[1]}" "${stacks[${_rule[1]}]}" "${_rule[2]}" "${stacks[${_rule[2]}]}"
        for ((_j = 0; _j < _nb; ++_j)); do
            #printf "moving char %d f=%s t=%s\n" "$_i" "${stacks[$_from]}" "${stacks[$_to]}"
            stacks[$_to]="${stacks[$_from]:0:1}${stacks[$_to]}"
            stacks[$_from]=${stacks[$_from]:1}
            #printf "  --> f=%s t=%s\n"  "${stacks[$_from]}" "${stacks[$_to]}"
        done

        printall
    done
}

part2() {
    local -i _i _nb _from
    local -a _rule
    local _tmp
    #printall
    for (( _i=0; _i<${#rules[@]}; ++_i )); do
        # shellcheck disable=2086
        read -ra _rule <<< ${rules[$_i]}
        (( _nb = _rule[0] ))
        (( _from = _rule[1] - 1 ))
        (( _to = _rule[2] - 1 ))
        stacks[$_to]="${stacks[$_from]:0:_nb}${stacks[$_to]}"
        stacks[$_from]=${stacks[$_from]:_nb}

        #printf "rule=%d nb=%d from=%d=%s to=%d=%s\n" "$_i" "$_nb" "$_from" "${stacks[$_from]}" "$_to" "${stacks[$_to]}"
        #printf "_rule=%d nb=%s from=%s/%s to=%s/%s\n" ${#_rule[@]} "${_rule[0]}" "${_rule[1]}" "${stacks[${_rule[1]}]}" "${_rule[2]}" "${stacks[${_rule[2]}]}"

        #printall
    done
}

solve() {
    local -i _i
    if (($1 == 1)); then
        part1
    else
        part2
    fi
    res=""
    for ((_i = 0; _i < ${#stacks[@]}; ++_i)); do
        res+="${stacks[_i]:0:1}"
    done
    #echo "res=$res"
}

main "$@"
exit 0
