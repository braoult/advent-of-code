#!/bin/bash
#
# ex1.bash: Advent2020 game, day 22/game 1.

CMD=${0##*/}
shopt -s extglob
set -o noglob

declare -a cards=() CARDS1=() CARDS2=()
declare -i count=-1

while read -r line; do
    case ${line:0:1} in
        P*)
            ((count++))
            ;;
        [1-9]*)
            cards[$count]+=" $line"
            ;;
    esac
done

# shellcheck disable=SC2206
CARDS1=(${cards[0]}) && CARDS2=(${cards[1]})

while ((${#CARDS1[@]} && ${#CARDS2[@]})); do
    # shellcheck disable=SC2128
    ((CARDS1 > CARDS2)) && CARDS1+=("$CARDS1" "$CARDS2") || CARDS2+=("$CARDS2" "$CARDS1")
    CARDS1=("${CARDS1[@]:1}")
    CARDS2=("${CARDS2[@]:1}")
done

declare -i n=$((${#CARDS1[@]}+${#CARDS2[@]})) res=0 i

for i in "${CARDS1[@]}" "${CARDS2[@]}"; do
    ((res+=n*i, n--))
done
printf "%s: res=%d\n" "$CMD" "$res"
exit 0
