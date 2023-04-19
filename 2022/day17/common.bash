#!/usr/bin/env bash
#
# common.bash: Advent of Code 2022, common bash functions
#
# Copyright (C) 2022-2023 Bruno Raoult ("br")
# Licensed under the GNU General Public License v3.0 or later.
# Some rights reserved. See COPYING.
#
# You should have received a copy of the GNU General Public License along with this
# program. If not, see <https://www.gnu.org/licenses/gpl-3.0-standalone.html>.
#
# SPDX-License-Identifier: GPL-3.0-or-later <https://spdx.org/licenses/GPL-3.0-or-later.html>

# shellcheck disable=2034
export cmdname=${0##*/}
export debug=0
export res
export LANG=C

shopt -s extglob
set -o noglob

usage() {
    printf "usage: %s [-d DEBUG] [-p PART]\n" "$cmdname"
    exit 1
}

checkargs() {
    local part=1
    while getopts p:d: todo; do
        case "$todo" in
            d)
                if [[ "$OPTARG" =~ ^[[:digit:]+]$ ]]; then
                    debug="$OPTARG"
                else
                    printf "%s: illegal [%s] debug level.\n" "$CMD" "$OPTARG"
                    exit 1
                fi
                ;;
            p)
                if [[ "$OPTARG" =~ ^[12]$ ]]; then
                    part="$OPTARG"
                else
                    printf "%s: illegal [%s] part.\n" "$CMD" "$OPTARG"
                    exit 1
                fi
                ;;
            *)
                usage
                ;;
        esac
    done
    # Now check remaining argument (backup directory)
    shift $((OPTIND - 1))

    (( $# > 1 )) && usage
    return "$part"
}

main() {
    local -i part

    checkargs "$@"
    part=$?
    parse "$part"
    solve "$part"
    printf "%s: res=%s\n" "$cmdname" "$res"
}
