#!/usr/bin/env bash
#
# aoc.bash: Advent of Code 2022, day 7
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

declare -A sizes=()                               # file size
declare -A dirs=()                                # directories
declare -a lines                                  # shell lines
declare -i curline=0                              # current line in shell
declare curdir

do_cd() {
    local dir="$1"
    case "$dir" in
        "..")
            curdir=${curdir%/*}
            [[ -z $curdir ]] && curdir="/"
            ;;
        "/")
            curdir="/"
            ;;
        *)
            [[ $curdir != "/" ]] && curdir+="/"
            curdir+="$dir"
    esac
    dirs[$curdir]="$curdir"
}

do_ls() {
    local remain
    local -i size
    ((curline++))
    while [[ $curline -lt ${#lines[@]} && ${lines[$curline]:0:1} != "\$" ]]; do
        if [[ "${lines[$curline]}" != dir* ]]; then
            size=${lines[$curline]% *}
            remain="$curdir"
            while [[ -n $remain ]]; do        # recurse up curdir and adjust sizes
                ((sizes[$remain] += size))
                remain=${remain%/*}
            done
            (( sizes["/"] += size ))
        fi
        ((curline++))
    done
    ((curline--))
}

parse() {
    readarray -t lines
    curline=0
    declare -a line

    while ((curline < ${#lines[@]})); do
        read -ra line <<< "${lines[$curline]}"
        case "${line[1]}" in
            "cd")
                do_cd "${line[2]}"
                ;;
            "ls")
                do_ls
                ;;
        esac
        ((curline++))
    done
}

solve() {
    declare -ig res
    local part="$1" dir mindir="/"
    local -i needed

    if ((part == 1)); then
        for dir in "${dirs[@]}"; do
            ((sizes[$dir] <= 100000 )) && ((res+=sizes[$dir]))
        done
    else
        (( res = sizes["/"] ))
        (( needed = sizes["/"] - (70000000-30000000) ))
        for dir in "${!dirs[@]}"; do
            if (( sizes[$dir] >= needed )); then
                if (( sizes[$dir] <= res )); then
                    mindir=$dir
                    ((res = sizes[$mindir]))
                fi
            fi
        done

    fi
}

main "$@"
exit 0
