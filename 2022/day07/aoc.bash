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

declare -A files=()                               # list of files in dir
declare -A sizes=()                               # file size
declare -A dirs=()                                # directories
declare -a lines                                  # shell lines
declare -i curline=0                              # current line in shell
declare curdir

print_sizes() {
    local idx
    echo +++++++++++++++++++
    for idx in "${!sizes[@]}"; do
        printf "size(%s)=%d\n" "$idx" "${sizes[$idx]}"
    done
    echo +++++++++++++++++++
}

do_cd() {
    local dir="$1"
    echo ">>> $curline ${lines[$curline]}"
    case "$dir" in
        "..")
            echo ".. before: $curdir"
            curdir=${curdir%/*}
            [[ -z $curdir ]] && curdir="/"
            echo ".. after: $curdir"
            ;;
        "/")
            curdir="/"
            ;;
        *)
            [[ $curdir != "/" ]] && curdir+="/"
            curdir+="$dir"
    esac
    dirs[$curdir]="$curdir"
    echo "> CD $dir newdir=$curdir"
}

do_ls() {
    local info file subdir remain
    echo ">>> $curline ${lines[$curline]}"
    ((curline++))
    while [[ $curline -lt ${#lines[@]} && ${lines[$curline]:0:1} != "\$" ]]; do
        read -r info file <<< "${lines[$curline]}"
        files[$curdir]+="$file"
        if [[ $info != dir ]]; then          # file
            remain="$curdir/$file"
            remain=${remain//+(\/)/\/}
            #sizes[$fullname]=$info
            # recurse up curdir and adjust sizes
            #subdir=${curdir}
            echo "info=$info [$remain]"
            while [[ -n $remain ]]; do
                echo "remain=$remain subdir=$subdir"
                ((sizes[$remain] += info))
                remain=${remain%/*}
                subdir=${remain##*/}
                echo "  -> remain=$remain subdir=$subdir"
            done
            (( sizes["/"] += info ))
        fi
        echo "ls: ${lines[$curline]}"
        ((curline++))
    done
    ((curline--))
    print_sizes
}

parse() {
    readarray -t lines
    curline=0
    declare -a line

    while ((curline < ${#lines[@]})); do
        read -ra line <<< "${lines[$curline]}"
        if [[ "${line[0]}" != "\$" ]]; then
            printf "ERROR line %d = %s\n" "$curline" "${lines[$curline]}"
            exit 1
        fi
        case "${line[1]}" in
            "cd")
                do_cd "${line[2]}"
                ;;
            "ls")
                do_ls
                ;;
        esac
        ((curline++))
        printf "WARNING curline=%d\n lines=%d\n" "$curline" "${#lines[@]}"
    done
}

solve() {
    declare -ig res
    local part="$1" dir mindir="/"
    local -i needed

    if ((part == 1)); then
        for dir in "${dirs[@]}"; do
            printf "size(%s)=%d\n" "$dir" "${sizes[$dir]}"
            if ((sizes[$dir] <= 100000 )); then
                ((res+=sizes[$dir]))
            fi
        done
    else
        (( needed = sizes["/"] - (70000000-30000000) ))
        printf "remain=%d\n" "$needed"
        ((res = sizes["/"]))
        for dir in "${!dirs[@]}"; do
            #printf "size(%s)=%d\n" "$dir" "${sizes[$dir]}"
            if (( sizes[$dir] >= needed )); then
                printf "dir %s (%d) will free enough res=%d\n" "$dir" "${sizes[$dir]}" "$res"
                if (( sizes[$dir] <= res )); then
                    mindir=$dir
                    ((res = sizes[$mindir]))
                    printf "new mindir=%s (%d)\n" "$mindir" "$res"
                fi
            fi
        done

    fi
}

main "$@"
exit 0
