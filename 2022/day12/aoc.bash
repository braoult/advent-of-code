#!/usr/bin/env bash
#
# aoc.bash: Advent of Code 2022, day 12
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

#declare -a map map2
declare -A height map visited
declare -i X Y
declare start end push=push1
declare -a queue

printmap() {
    local -i x y
    for (( y = 0; y < Y; ++y )); do
        for (( x = 0; x < X; ++x )); do
            printf "%2d " "${map["$x,$y"]}"
        done
        echo
    done
}

printvis() {
    local -i x y
    for (( y = 0; y < Y; ++y )); do
        for (( x = 0; x < X; ++x )); do
            printf "%2d " "${visited["$x,$y"]}"
        done
        echo
    done
}

push1() {                                         # push part 1
    local -i _d="$1" _x="$2" _y="$3" _h="$4"
    local _c="$_x,$_y"
    if (( !visited[$_c] && map[$_c] <= (_h + 1) )); then
        (( _d++ ))
        visited["$_c"]=$_d
        queue+=("$_d:$_c")
    fi
}

push2() {                                         # push part 2
    local -i _d="$1" _x="$2" _y="$3" _h="$4"
    local _c="$_x,$_y"
    if (( !visited[$_c] && map[$_c] >= (_h - 1) )); then
        (( _d++ ))
        visited["$_c"]=$_d
        queue+=("$_d:$_c")
    fi
}

pop() {
    local -n _d="$1" _x="$2" _y="$3"
    local head
    ((!${#queue[@]})) && echo pop: queue empty. && exit 1
    head="${queue[0]}"
    _d=${head%:*}
    head=${head#*:}
    _x=${head%,*}
    _y=${head#*,}
    unset 'queue[0]'
    queue=("${queue[@]}")
    return 0
}

# initialize height values
init() {
    declare -i i=1
    for c in {a..z}; do
        (( height[$c] = i++ ))
    done
    height[S]=1
    height[E]=26
}

parse() {
    local -i part="$1"
    local -a _map
    local -i x y
    local c
    ((part == 2)) && push=push2

    init
    readarray -t _map
    X=${#_map[0]}
    Y=${#_map[@]}
    # create array map[x,y]
    for (( y = 0; y < Y; ++y )); do
        for (( x = 0; x < X; ++x )); do
            c=${_map[$y]:x:1}
            map["$x,$y"]=${height[$c]}
            case "$c" in
                S) start="$x,$y"
                   ((part == 1)) && $push 0 "$x" "$y" 1
                   ;;
                E) end="$x,$y"
                   ((part == 2)) && $push 0 "$x" "$y" 1
                   ;;
            esac
        done
    done
}

solve() {
    local -i h d x y

    while pop d x y; do
        (( h=${map["$x,$y"]} ))
        if [[ $part == 1 ]]; then
            if [[ "$x,$y" == "$end" ]]; then
                res=$((d-1))
                return
            fi
        else
            if [[ "${map["$x,$y"]}" == 1 ]]; then
                res=$((d-1))
                return
            fi
        fi
        if ((y > 0)); then                        # north
            $push "$d" "$x" "$((y-1))" "$h"
        fi
        if ((x+1 < X)); then                      # east
            $push "$d" "$((x+1))" "$y" "$h"
        fi
        if ((y+1 < Y)); then                      # south
            $push "$d" "$x" "$((y+1))" "$h"
        fi
        if ((x > 0)); then                        # west
            $push "$d" "$((x-1))" "$y" "$h"
        fi
    done
}

main "$@"
exit 0
