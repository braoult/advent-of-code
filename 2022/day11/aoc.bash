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
declare -i reduce=1

print() {
    for i in "${!items[@]}"; do
        printf "Monkey %d: %s\n" "$i" "${items[$i]}"
    done
}

calc1() {
    local -i m1=0 m2=0 i
    local -n _res="$1"
    for i in "${inspect[@]}"; do
        if ((i > m1)); then
            ((m2 = m1, m1 = i))
        elif ((i > m2)); then
            ((m2 = i))
        fi
    done
    echo "m1=$m1 m2=$m2"
    (( _res = m1 * m2 ))
}

inspect() {
    local -i _part="$1" monkey="$2" item _tmp
    local -i _op1=${op1[$monkey]} _op2=${op2[$monkey]}
    local -a _items
    read -ra _items <<< "${items[$monkey]}"
    printf "ITEMS [%s] %d:(%s)\n" "${items[$monkey]}" "${#_items[@]}" "${_items[*]}"
    for item in "${_items[@]}"; do
        (( inspect[monkey]++ ))
        echo "M=$monkey items=(${items[$monkey]})/$item"
        [[ -v op1[$monkey] ]] || _op1=$item
        [[ -v op2[$monkey] ]] || _op2=$item
        echo "old=$item/op1=$_op1 op2=$_op2"
        case "${op[$monkey]}" in
            \+)
                (( _tmp = _op1 + _op2 ))
                printf "%d + %d = %d " "$_op1" "$_op2" "$_tmp"
                ;;
            \*)
                (( _tmp = _op1 * _op2 ))
                printf "%d * %d = %d " "$_op1" "$_op2" "$_tmp"
                ;;
        esac
        ((_part == 1)) && (( _tmp /= 3 ))
        (( _tmp %= reduce ))
        if ! (( _tmp % div[monkey] )); then
            items[${ttrue[$monkey]}]+=" $_tmp"
        else
            items[${tfalse[$monkey]}]+=" $_tmp"
        fi
        printf "/3 = %d\n" "$_tmp"
        printf "M=%d new=%d\n" "$monkey" "$_tmp"
    done
    items[$monkey]=""
    print
    echo
}

parse() {
    local -i monkey=0
    local -a _items

    while read -r; do                             # ignore Monkey number
        #echo -n "monkey=$monkey "
        IFS=" :," read -ra _items                 # starting items

        items[$monkey]="${_items[*]:2}"
        echo -n "items[$monkey]=${items[$monkey]} "
        IFS=" :=" read -r _ _ op1[$monkey] op[$monkey] op2[$monkey]          # operator and operand
        [[ ${op1[$monkey]} == old ]] && unset "op1[$monkey]"
        [[ ${op2[$monkey]} == old ]] && unset "op2[$monkey]"
        echo -n "op=${op[$monkey]} ops=${op1[$monkey]}/${op2[$monkey]} "
        IFS=" :=" read -r _ _ _ div[$monkey]      # divisor
        (( reduce *= div[monkey] ))
        echo -n "div=${div[$monkey]} "
        read -r _ _ _ _ _ ttrue[$monkey]          # throw if true
        read -r _ _ _ _ _ tfalse[$monkey]
        echo "T=${ttrue[$monkey]} F=${tfalse[$monkey]}"
        read -r
        (( monkey++ ))
        #break
    done
}

solve() {
    local -i _loops=20
    (( part == 2 )) && (( _loops = 10000 ))
    for ((round = 0; round < _loops; ++round)); do
        for ((monkey = 0; monkey < ${#div[@]}; ++monkey)); do
            inspect "$part" "$monkey"
        done
    done
    printf "inspect=%s\n" "${inspect[*]}"
    # remove last '\n', add starting '\n'
    calc1 res
    #(( part == 2 )) && res=$'\n'${res::-1}
}

main "$@"
exit 0
