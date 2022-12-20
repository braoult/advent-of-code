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

declare -ai div ttrue tfalse vis
declare -a it                                     # (1 2) is "1 2"
declare -a op1 op op2
declare -i lcm=1 monks=0 divisor=3

calc() {
    local -n _res="$1"
    local -i _m1=0 _m2=0 _i

    for _i in "${vis[@]}"; do                     # find the 2 biggest numbers
        if ((_i > _m1)); then
            ((_m2 = _m1, _m1 = _i))
        elif ((_i > _m2)); then
            ((_m2 = _i))
        fi
    done
    (( _res = _m1 * _m2 ))
}

parse() {
    local -a _it

    while read -r; do                             # ignore Monkey number
        IFS=" :," read -ra _it                    # starting items
        it[$monks]="${_it[*]:2}"
        IFS=" :=" read -r _ _ op1[$monks] op[$monks] op2[$monks]
        [[ ${op[$monks]} == "+" ]] && unset "op[$monks]"
        [[ ${op1[$monks]} == old ]] && unset "op1[$monks]"
        [[ ${op2[$monks]} == old ]] && unset "op2[$monks]"
        IFS=" :=" read -r _ _ _ div[$monks]       # divisor
        (( lcm *= div[monks] ))
        read -r _ _ _ _ _ ttrue[$monks]           # throw if true
        read -r _ _ _ _ _ tfalse[$monks]
        read -r
        (( monks++ ))
    done
}

solve() {
    local -i _loops=20 round m _op1 _op2 i

    (( part == 2 )) && (( _loops = 10000, divisor = 1 ))
    for ((round = 0; round < _loops; ++round)); do
        for ((m = 0; m < monks; ++m)); do
            _op1=${op1[$m]}
            _op2=${op2[$m]}

            # shellcheck disable=SC2068
            for i in ${it[$m]}; do
                (( vis[m]++ ))
                [[ -v op1[$m] ]] || _op1=$i
                [[ -v op2[$m] ]] || _op2=$i
                if [[ -v op[$m] ]]; then
                    (( _tmp = (_op1 * _op2) / divisor % lcm ))
                else
                    (( _tmp = (_op1 + _op2) / divisor % lcm ))
                fi
                if (( _tmp % div[m] )); then
                    it[${tfalse[$m]}]+=" $_tmp"
                else
                    it[${ttrue[$m]}]+=" $_tmp"
                fi
            done
            it[$m]=""
        done
    done
    calc res
}

main "$@"
exit 0
