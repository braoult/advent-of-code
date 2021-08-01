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

run() {
    local -n _p1="$1" _p2="$2"
    # shellcheck disable=SC2206
    local -a p1=(${_p1[@]}) p2=(${_p2[@]}) sp1 sp2
    local -A hash=()
    local -i winner=0                             # 1: p1, 2: p2

    while ((${#p1[@]} && ${#p2[@]})); do
        str="${p1[*]}  ${p2[*]}"

        [[ -v hash["$str"] ]] && return 1
        # shellcheck disable=SC2034
        hash["$str"]=""

        if ((p1 < ${#p1[@]} && p2 < ${#p2[@]} )); then
            # shellcheck disable=SC2034,SC2206
            sp1=(${p1[*]:1:p1}) && sp2=(${p2[*]:1:p2})
            run sp1 sp2
            winner=$?
        else
            ((p1 > p2)) && winner=1 || winner=2
        fi
        if ((winner==1)); then
            # shellcheck disable=SC2128
            p1+=("$p1" "$p2")
        else
            # shellcheck disable=SC2128
            p2+=("$p2" "$p1")
        fi
        p1=("${p1[@]:1}")
        p2=("${p2[@]:1}")
    done
    # shellcheck disable=SC2206
    _p1=(${p1[@]}) && _p2=(${p2[@]})
    return "$winner"
}

run CARDS1 CARDS2
declare -i n=$((${#CARDS1[@]}+${#CARDS2[@]})) i
res=0

for i in "${CARDS1[@]}" "${CARDS2[@]}"; do
    ((res+=n*i, n--))
done

printf "%s: res=%d\n" "$CMD" "$res"
exit 0
