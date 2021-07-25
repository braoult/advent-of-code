#!/bin/bash
#
# ex1.bash: Advent2020 game, day 20/game 2.

CMD=${0##*/}
set -o noglob
shopt -s extglob

#declare -A strings
declare -a strings T R B L RT RR RB RL nums
declare -a CORNER EDGE CENTRAL
declare -i count=-1 res=1

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

ALL="${T[@]} ${R[@]} ${B[@]} ${L[@]} ${RT[@]} ${RR[@]} ${RB[@]} ${RL[@]}"
ALLSIZE=${#ALL}

for ((i=0; i<${#nums[@]}; ++i)); do
    count=0

    for t in ${T[$i]} ${R[$i]} ${B[$i]} ${L[$i]}; do
        # https://stackoverflow.com/questions/26212889/bash-counting-substrings-in-a-string/50601141#50601141
        S=${ALL//$t}
        # 10 is line size
        ((count += (ALLSIZE-${#S})/10))
    done
    # 6 is 4 for itself, + 2 for other matching
    #printf "%s: %d\n" "${nums[$i]}" "$count"
    case "$count" in
        6)
            CORNER+=(${nums[$i]})
            ((res*=${nums[$i]}))
            ;;
        7)
            EDGE+=(${nums[$i]})
            ;;
        8)
            CENTRAL+=(${nums[$i]})
            ;;
    esac

done

printf "CENTRAL[%d]=%s\n" "${#CENTRAL[@]}" "${CENTRAL[*]}"
printf "EDGE[%d]=%s\n" "${#EDGE[@]}" "${EDGE[*]}"
printf "CORNER[%d]=%s\n" "${#CORNER[@]}" "${CORNER[*]}"
printf "%s : res=%d\n" "$CMD" "$res"

exit 0
