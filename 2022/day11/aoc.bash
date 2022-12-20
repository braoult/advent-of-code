#!/usr/bin/env bash
#
# aoc.bash: Advent of Code 2022, day 11
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

declare -ai div ttrue tfalse inspect
declare -a items                                  # (1 2) is "1 2"
declare -a op1 op op2
declare -i reduce=1 nmonkeys=0 divisor=3

calc() {
    local -n _res="$1"
    local -i _m1=0 _m2=0 _i

    for _i in "${inspect[@]}"; do                 # find the 2 biggest numbers
        if ((_i > _m1)); then
            ((_m2 = _m1, _m1 = _i))
        elif ((_i > _m2)); then
            ((_m2 = _i))
        fi
    done
    (( _res = _m1 * _m2 ))
}

inspect() {
    local -i _part="$1" monkey="$2" item _tmp
    local -i _op1=${op1[$monkey]} _op2=${op2[$monkey]}
    local -a _items

    read -ra _items <<< "${items[$monkey]}"       # convert to array
    for item in "${_items[@]}"; do
        (( inspect[monkey]++ ))
        [[ -v op1[$monkey] ]] || _op1=$item
        [[ -v op2[$monkey] ]] || _op2=$item
        if [[ "${op[$monkey]}" == "+" ]]; then
            (( _tmp = _op1 + _op2 ))
        else
            (( _tmp = _op1 * _op2 ))
        fi
        (( _tmp /= divisor, _tmp %= reduce ))
        if ! (( _tmp % div[monkey] )); then
            items[${ttrue[$monkey]}]+=" $_tmp"
        else
            items[${tfalse[$monkey]}]+=" $_tmp"
        fi
    done
    items[$monkey]=""
}

parse() {
    local -i monkey=0
    local -a _items

    while read -r; do                             # ignore Monkey number
        #echo -n "monkey=$monkey "
        IFS=" :," read -ra _items                 # starting items

        items[$monkey]="${_items[*]:2}"
        #echo -n "items[$monkey]=${items[$monkey]} "
        IFS=" :=" read -r _ _ op1[$monkey] op[$monkey] op2[$monkey]          # operator and operand
        [[ ${op1[$monkey]} == old ]] && unset "op1[$monkey]"
        [[ ${op2[$monkey]} == old ]] && unset "op2[$monkey]"
        #echo -n "op=${op[$monkey]} ops=${op1[$monkey]}/${op2[$monkey]} "
        IFS=" :=" read -r _ _ _ div[$monkey]      # divisor
        (( reduce *= div[monkey] ))
        #echo -n "div=${div[$monkey]} "
        read -r _ _ _ _ _ ttrue[$monkey]          # throw if true
        read -r _ _ _ _ _ tfalse[$monkey]
        #echo "T=${ttrue[$monkey]} F=${tfalse[$monkey]}"
        read -r
        (( monkey++ ))
        #break
    done
}

solve() {
    local -i _loops=20
    (( part == 2 )) && (( _loops = 10000, divisor = 1 ))
    for ((round = 0; round < _loops; ++round)); do
        for ((monkey = 0; monkey < ${#div[@]}; ++monkey)); do
            inspect "$part" "$monkey"
        done
    done
    #printf "inspect=%s\n" "${inspect[*]}"
    calc res
}

main "$@"
exit 0
