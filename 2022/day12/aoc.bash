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
    printf "push d=%d x=%d y=%d h=%d nh=%d... " "$_d" "$_x" "$_y" "$_h" \
           "${map[$_c]}"
    if (( !visited[$_c] && map[$_c] <= (_h + 1) )); then
        (( _d++ ))
        visited["$_c"]=$_d
        queue+=("$_d:$_c")
        echo "$_d:$_c" pushed.
    else
        echo not pushed.
    fi
    #local str="$_d:$_x-$_y"
    #queue+=("$1")
}

push2() {                                         # push part 2
    local -i _d="$1" _x="$2" _y="$3" _h="$4"
    local _c="$_x,$_y"
    printf "push d=%d x=%d y=%d h=%d nh=%d... " "$_d" "$_x" "$_y" "$_h" \
           "${map[$_c]}"
    if (( !visited[$_c] && map[$_c] >= (_h - 1) )); then
        (( _d++ ))
        visited["$_c"]=$_d
        queue+=("$_d:$_c")
        echo "$_d:$_c" pushed.
    else
        echo not pushed.
    fi
    #local str="$_d:$_x-$_y"
    #queue+=("$1")
}

pop() {
    local -n _d="$1" _x="$2" _y="$3"
    local head
    ((!${#queue[@]})) && echo pop: queue empty. && return 1
    head="${queue[0]}"
    #echo "Q=${#queue[@]}=${queue[*]} head=$head"
    # shellcheck disable=2034
    printf "pop: %s\t" "$head"
    _d=${head%:*}
    # shellcheck disable=2034
    head=${head#*:}
    _x=${head%,*}
    _y=${head#*,}
    unset 'queue[0]'
    queue=("${queue[@]}")
    printf "pop: d=%d x=%d y=%d remain=%d\n"  "$_d" "$_x" "$_y" "${#queue[@]}"
    #echo "Q=${#queue[@]}=${queue[*]} head=$head"
    return 0
}

# initialize height values
init() {
    declare -i i=1
    for c in {a..z}; do
        (( height[$c] = i++ ))
        #echo
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
    echo "X=$X Y=$Y"
    # create array map[x,y]
    for (( y = 0; y < Y; ++y )); do
        for (( x = 0; x < X; ++x )); do
            c=${_map[$y]:x:1}
            # printf "y=%d x=%d c=%s h=%d " "$y" "$x" "$c" "${height[$c]}"
            map["$x,$y"]=${height[$c]}
            # printf "M=%s\n" "${map["$x,$y"]}"
            case "$c" in
                S) start="$x,$y"
                   ((part == 1)) && $push 0 "$x" "$y" 1
                   #printf "\tstart\n"
                   ;;
                E) end="$x,$y"
                   ((part == 2)) && $push 0 "$x" "$y" 1
                   #printf "\tend\n"
                   ;;
            esac
        done
    done
    printf "start=%s end=%s\n" "$start" "$end"
    printmap
}

solve() {
    #local depth coord
    local -i h d x y


    #push 0 ${start%-*} ${start#*-} 1
    #push "11:46-55"
    while pop d x y; do
        (( h=${map["$x,$y"]} ))
        if [[ $part == 1 ]]; then
            if [[ "$x,$y" == "$end" ]]; then
                res=$((d-1))
                printmap
                echo
                printvis
                return
            fi
        else
            if [[ "${map["$x,$y"]}" == 1 ]]; then
                res=$((d-1))
                printmap
                echo
                printvis
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
        #echo "d=$d x=$x y=$y"
        echo
    done
    #push "10:45-54"
    # push "11:46-55"
    # pop dep x y
    # echo "d=$d x=$x y=$y"
    # pop dep x y
    # echo "d=$d x=$x y=$y"
    # push "78:1-2"
    # pop dep x y
    # echo "d=$d x=$x y=$y"
    # #pop d c
    # #echo "d=$d c=$c"

    # :
    # #(( part == 2 )) && (( _loops = 10000, divisor = 1 ))
}

main "$@"
exit 0
