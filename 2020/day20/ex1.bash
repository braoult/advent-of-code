#!/bin/bash
#
# ex1.bash: Advent2020 game, day 20/game 1.

CMD=${0##*/}
set -o noglob
shopt -s extglob

#declare -A strings
declare -a strings T R B L RT RR RB RL nums

declare -i count=-1
while read -r line; do
    case ${line:0:1} in
        T*)
            ((count++))
            num=${line##* }
            num=${num%%:}
            nums+=("$num")
            ;;
        \#|.)
            strings[$count]="${strings[$count]} $line"
            ;;
    esac
done
((count--))

for key in "${!nums[@]}"; do
    # shellcheck disable=SC2206
    str=(${strings[$key]})
    top="${str[0]}"
    bottom="${str[9]}"
    T+=("$top")
    B+=("$bottom")
    # find out right and bottom
    unset left right
    for ((i=0; i<10; ++i)); do
        left+=${str[$i]:0:1}
        right+=${str[$i]: -1:1}
    done
    R+=("$right")
    L+=("$left")
    unset fliptop flipbottom flipleft flipright
    for ((i=9; i>=0; --i)); do
        fliptop+=${top:$i:1}
        flipbottom+=${bottom:$i:1}
        flipleft+=${left:$i:1}
        flipright+=${right:$i:1}
    done
    RT+=("$fliptop")
    RR+=("$flipright")
    RB+=("$flipbottom")
    RL+=("$flipleft")

done

ALL=("${T[@]}" "${R[@]}" "${B[@]}" "${L[@]}" "${RT[@]}" "${RR[@]}" "${RB[@]}" "${RL[@]}")
declare -i res=1 count
for ((i=0; i<${#nums[@]}; ++i)); do
    count=0

    for t in ${T[$i]} ${R[$i]} ${B[$i]} ${L[$i]}; do
        for s in "${ALL[@]}"; do
            [[ $t == "$s" ]] && ((count++))
        done
    done

    ((count ==6)) && ((res*=${nums[$i]}))
done

printf "%s : res=%d\n" "$CMD" "$res"

exit 0
