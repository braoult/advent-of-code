#!/bin/bash
#
# ex1.bash: Advent2020 game, day 24/game 2.

CMD=${0##*/}
shopt -s extglob
set -o noglob

declare -A plan=() count=()
declare -i x y loops=100
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

# adjacent cells (x1 y1 x2 y2 etc...)
declare -a directions=(
    2  0 -2  0                                    # east and west
    1 -1  1  1                                    # SE and NE
    -1 -1 -1  1                                   # SW and NW
)

for ((_c = 0; _c < loops; ++_c)) do
    count=()
    for cell in "${!plan[@]}"; do                 # count adjacent tiles
        x=${cell%,*}
        y=${cell#*,}
        for ((i = 0; i < ${#directions[@]}; i += 2)); do
            (( ++count[$((x + directions[$i])),$((y + directions[$((i+1))]))] ))
        done
    done
    for cell in "${!plan[@]}"; do                 # check black tiles
        (( count[$cell] == 0 || count[$cell] > 2)) && unset "plan[$cell]"
        unset "count[$cell]"
    done
    for cell in "${!count[@]}"; do                # remaining ones are white
        ((count[$cell] == 2)) && plan[$cell]=1
    done
done

printf "%s: res=%d\n" "$CMD" "${#plan[@]}"

exit 0
