#!/bin/bash
#
# ex2.bash: Advent2020 game, day 21/game 2.

CMD=${0##*/}
shopt -s extglob
set -o noglob

declare -A CANBE=() A_RULES=() FOUND=() ALL_I=() ALL_A=() I_COUNT=()
declare -a R
declare -i count=0

# intersect the words of 2 strings into $1
intersect() {
    local -n _res="$1"
    # shellcheck disable=SC2206
    local -a _str1=($2) _str2=($3)
    local _i _j
    _res=""
    for _i in "${_str1[@]}"; do
        for _j in "${_str2[@]}"; do
            [[ "$_i" == "$_j" ]] && _res+=" $_i"
        done
    done
    _res="${_res##*( )}"
}
# count words in a string and put result in $1
count() {
    local -n _res="$1"
    # shellcheck disable=SC2206
    local -a _str=($2)
    _res=${#_str[@]}
}
# remove a word from a string and put result in $1
delw() {
    local -n _res="$1"
    local _w=$2
    # shellcheck disable=SC2206
    local -a _str=($3)
    local _i
    _res=""
    for _i in "${_str[@]}"; do
        [[ $_i != "$_w" ]] && _res+=" $_i"
    done
    _res="${_res##*( )}"
    _res="${_res%%*( )}"
}

# simple bubble sort for alphanumeric array (ascending)
# args: 1: the array
# todo: add a parameter for asc/desc order.
sort_a() {
    local -a array=( "$@" )
    local -i i max=$(( ${#array[@]} - 1 ))
    local val1 val2 tmp

    for (( max= $(( ${#array[@]} - 1 )); max > 0; max-- )); do
        for (( i=0; i<max; i++ )); do
            val1=${array[$i]}
            val2=${array[$((i + 1))]}

            # switch if necessary
            if [[ $val1 > $val2 ]]; then
                tmp=$val1
                array[$i]=$val2
                array[$((i + 1))]=$tmp
            fi
        done
    done
    echo "${array[@]}"
}

REGEX="(.*) \(contains(.*)\)"
while read -r line; do
    [[ "$line" =~ $REGEX ]]
    ingr="${BASH_REMATCH[1]}"
    allg="${BASH_REMATCH[2]}"
    allg=${allg//,}

    R[$count]="$ingr"
    for ka in $allg; do
        A_RULES[$ka]+=" $count"
        ALL_A[$ka]=""
        for ki in $ingr; do
            ALL_I[$ki]=""
        done
    done
    for ki in $ingr; do
        (( I_COUNT[$ki]++ ))
    done
    ((count++))
done
all_i="${!ALL_I[*]}"
for k in "${!ALL_A[@]}"; do
    CANBE[$k]="$all_i"
done

solved=0
while ((solved==0)); do
    solved=1
    for allerg in "${!CANBE[@]}"; do
        str="${CANBE[$allerg]}"

        if [[ -v FOUND["$allerg"] ]]; then
            continue
        fi
        for rule in ${A_RULES[$allerg]}; do
            intersect str "${R[$rule]}" "$str"
        done
        CANBE[$allerg]="$str"

        count count "$str"
        ((count > 1)) && solved=0
        if ((count==1)); then
            word=$str #{CANBE[$allerg]}
            # shellcheck disable=SC2034
            FOUND[$allerg]="${CANBE[$allerg]}"
            unset ALL_I["${CANBE[$allerg]}"]
            for allerg1 in "${!CANBE[@]}"; do
                if [[ "$allerg" != "$allerg1" ]]; then
                    delw CANBE["$allerg1"]  "$word" "${CANBE[$allerg1]}"
                fi
            done
            for rule in "${!R[@]}"; do
                delw R[$rule] "$word" "${R[$rule]}"
            done
        fi
    done
done

sorted=$(sort_a "${!CANBE[@]}")
res=""
for k in $sorted; do
    res+=",${CANBE[$k]}"
done

printf "%s: res=%s\n" "$CMD" "${res##,}"
exit 0
