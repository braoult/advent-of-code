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


# params: (%d)depth (int ref)result (array name)player1 (array name)player2

declare -i game=0
run() {
    local -n _res="$1" _p1="$2" _p2="$3"
    local -a p1=(${_p1[@]}) p2=(${_p2[@]}) sp1 sp2
    local -A hash=()
    local winner=0 round=1 g                    # 1: p1, 2: p2
    local str
    ((game++))
    g=$game
    printf "\n=== Game %d ===\n" "$g"
    #printf "   p1=%s \n" "${p1[*]}"
    #printf "   p2=%s \n" "${p2[*]}"
    while ((${#p1[@]} && ${#p2[@]})); do
        echo
        echo -n "-- "
        printf "Round %d (Game %d) --\n" "$round" "$g"
        S="${p1[*]}"
        S="${S// /, }"
        printf "Player 1's deck: %s\n" "$S"
        S="${p2[*]}"
        S="${S// /, }"
        printf "Player 2's deck: %s\n" "$S"
        str="${p1[*]}  ${p2[*]}"

        #[[ -v ${!hash[$_str]} ]] && return 1
        if [[ -v hash["$str"] ]]; then
            _res=1
            return 1
        fi
        printf "Player 1 plays: %s\n" "$p1"
        printf "Player 2 plays: %s\n" "$p2"
        hash["$str"]=""

        if ((p1 < ${#p1[@]} && p2 < ${#p2[@]} )); then
            sp1=(${p1[*]:1:p1})
            sp2=(${p2[*]:1:p2})
            printf "Playing a sub-game to determine the winner...\n"
            run winner sp1 sp2
            winner=$?
            printf "...anyway, back to game %d.\n" "$g"
        else
            ((p1 > p2)) && winner=1 || winner=2
        fi
        if ((winner==1)); then
            p1+=("$p1" "$p2")
            _res=1
            printf "Player 1 wins round %d of game %d!\n" "$round" "$g"
        else
            p2+=("$p2" "$p1")
            _res=2
            printf "Player 2 wins round %d of game %d!\n" "$round" "$g"
        fi
        p1=("${p1[@]:1}")
        p2=("${p2[@]:1}")
        #printf "   p1=%s\n" "${p1[*]}"
        #printf "   p2=%s\n" "${p2[*]}"
        ((round++))
    done
    _p1=(${p1[@]})
    _p2=(${p2[@]})
    _res="$winner"
    printf "The winner of game %d is player %d!\n\n" "$g" "$_res"
    return "$winner"
}

declare -i res
run res CARDS1 CARDS2
declare -i n=$((${#CARDS1[@]}+${#CARDS2[@]})) i
res=0

S="${CARDS1[*]}"
S="${S// /, }"
printf "Player 1's deck: %s\n" "$S"
S="${CARDS2[*]}"
S="${S// /, }"
printf "Player 2's deck: %s\n" "$S"

for i in "${CARDS1[@]}" "${CARDS2[@]}"; do
    ((res+=n*i, n--))
done

printf "%s: res=%d (should be 32760)\n" "$CMD" "$res"
exit 0
