#!/bin/bash
#
# ex1.bash: Advent2020 game, day 23/game 1.

CMD=${0##*/}
shopt -s extglob
set -o noglob

#declare -a INPUT
declare rev="98765432198765"
runs=${1:-10}
read -r str
#[[ "$str" =~ ${str//?/(.)} ]]
#INPUT=("${BASH_REMATCH[@]:1}")
#printf "INPUT=%s \n" "${INPUT[*]}"

pos() {
    local -n _pos=$1
    local c=$2 s=$3
    local -i i
    for ((i=0; i<9; ++i)); do
        [[ ${s:i:1} = "$c" ]] && break
    done
    printf "pos(%s, %s)=%d\n" "$s" "$c" "$i"
    _pos=$((i+1))
}

# charpos - get cup position in a string
# $1 reference of variable to store the position
# $2: The string
# $3: The cup to find
#
# @Return: cup position
# example: charpos "987654321" "9" will return 0
charpos() {
    local -n _res="$1"
    local left="${2%%"$3"*}"
    # printf " charpos(%s,%s)=%d " "$2" "$3" "${#left}"
    [[ "$left" = "$2" ]] && _res=-1 || _res=${#left}
}

# destcup - get insertion position
# $1: reference of variable to store the cup position in string
# $2: current cup
# $3: The string
#
# @Return: the next cup
# example: nextchar "987654321" "9" will return 8
destcup() {
    local -n _pos="$1"
    local _cup="$2" _str="$3"
    #(( _cup-- ))
    printf "destcup str=%s _pos=%d cup=%d\n" "$_str" "$_pos" "$_cup"
    while (( _cup = (_cup + 8) % 9))
          (( _cup == 0 )) && (( _cup = 9 ))
          printf "trying %d in %s ..." "$_cup" "$_str"
          charpos _pos "$_str" "$_cup"
          printf " pos=%d " "$_pos"
          (( _pos == -1 )); do
        echo fail
    done
    echo ok
}

# nextcup - get next cup in string
# $1: reference of current cup
# $2: The string
#
# @Return: the next cup
nextcup() {
    local -n _cup="$1"
    local _str="$2$2"
    _str=${_str#*"$_cup"}
    # printf "str=%s\n" "$_str"
    _cup=${_str:0:1}
}

# pickup - get the pickup string after current one
# $1 reference of variable to store the pickup string
# $2: reference of string string
# $3: The current cup
#
# @Return: the pickup string
# example: charpos "987654321" "2" will return "198"
pickup() {
    local -n _pick="$1" _str="$2"
    local -i _pos
    local tmp="$_str$_str"
    #printf "STR2= %s %s\n" "$2" "$_str"
    charpos _pos "$_str" "$3"
    printf " str=%s _pos(%s)=%d->" "$_str" "$3" "$_pos"
    (( _pos = (_pos + 1) % 9 ))
    _pick="${tmp:_pos:3}"
    tmp=${tmp//[$_pick]}
    _str=${tmp:0:6}
    printf "%d = pick=%s str=%s\n" "$_pos" "$_pick" "$_str"
}

declare -i res pos=0 cup=${str:0:1}
declare pick
#printf "BEFORE str=%s cup=%s pos=%d\n" "$str" "$cup" "$pos"
#nextcup cup pos "$str"
#printf "AFTER str=%s cup=%s pos=%d\n" "$str" "$cup" "$pos"

for ((i = 1; i <= 100; ++i)); do
    printf "\nloop=%d str=%s cup=%s pos=%d\n" "$i" "$str" "$cup" "$pos"
    pickup pick str "$cup"
    printf "AFTER1 str=%s pick=%s cup=%s pos=%d\n" "$str" "$pick" "$cup" "$pos"
    destcup pos "$cup" "$str"
    printf "AFTER2 str=%s pick=%s cup=%s pos=%d\n" "$str" "$pick" "$cup" "$pos"
    #insert str "$pick" "$pos"
    str="${str:0:pos+1}$pick${str:pos+1}"
    printf "newstr=%s pick=%s\n" "$str" "$pick"
    nextcup cup "$str"
    printf "next cup=%s\n" "$cup"
    #charpos res "$str" "$i"
    #nextchar next "$str" "$i"
    #printf "a(%s, %d)=%d next=%s\n" "$str" "$i" "$res" "$next"

done
res="${str#*1}${str%1*}"
printf "%s: res=%d\n" "$CMD" "$res"
exit 0

# move - rune a move
# $1: The input string (by address)
# $2: The current cup
#
# @Return: The next cup
move() {

    :
}

run() {
    local -i l="$1" p="$2" pos
    local str="$3"
    local str2 o c

    while ((--l)); do
        printf "%s move %d --\n" "--" $(($1 - l))
        printf "cups=%s\n" "$str"
        printf "pos=%s\n" "$p"
        #
        #printf "cups:"
        #for ((j=0; j<9; ++j)); do

        #printf "str2=%s\n" "$str2"

        o=${str2:p+1:3}
        c=${str2:p:1}
        # remove the 3 cups
        str2=${str2//$o/}
        rev1=${rev#*$c}
        printf "c=%s\npick up=%s\nstr2=%s\n" "$c" "$o" "$str2"
        printf "rev1=%s\n" "$rev1"

        # find destination cup
        for ((i=0; i<4; ++i)); do
            c2=${rev1:i:1}
            printf "testing %c in %s\n" "$c2" "$o"
            [[ $o =~ .*$c2.* ]] || break
        done
        printf "destination=%s\n" "$c2"
        pos pos "$c2" "$str2"
        str2=${str2:0:$pos}$o${str2:$pos}
        #str2=${str2/$c2/$c2$o}
        #str2=${str2/$c2/$c2$o}

        printf "str2=%s\n" "$str2"
        str=${str2:0:9}

        ((++p==9)) && p=0
        printf "new str=%s\n\n" "$str"
    done
}
