#!/bin/bash
#
# ex1.bash: Advent2020 game, day 24/game 1.

# See https://www.redblobgames.com/grids/hexagons/ for hexagon possible
# representations.
# I use the "doubled-coordinate" system here :
#   https://www.redblobgames.com/grids/hexagons/#coordinates-doubled
CMD=${0##*/}
shopt -s extglob
set -o noglob

declare -A plan
declare -i x y

while read -r line; do
    x=0
    y=0
    for ((i=0; i<${#line}; ++i)); do
        c=${line:i:1}
        case "$c" in
            e) ((++x))
               ;;
            w) ((--x))
               ;;
            s) ((--y, ++i))
               c=${line:i:1}
               ;;
            n) ((++y, ++i))
               c=${line:i:1}
               ;;
        esac
        if [[ "$c" = e ]]; then
            ((++x))
        elif [[ "$c" = w ]]; then
            ((--x))
        else
            printf "error c=%s\n" "$c"
            exit 1
        fi
    done
    [[ -v plan[$x,$y] ]] && unset "plan[$x,$y]" || plan[$x,$y]=1
done
res=${#plan[@]}
printf "%s: res=%s\n" "$CMD" "$res"
exit 0
