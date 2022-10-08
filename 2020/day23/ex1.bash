#!/bin/bash
#
# ex1.bash: Advent2020 game, day 23/game 1.

CMD=${0##*/}
shopt -s extglob
set -o noglob

declare -A next                                   # next[i] is cup right to i
declare -i end runs cup _cup

read -r str
tmp="${str//?()/x}"
tmp=${tmp#x}
IFS=x read -ra array <<< "$tmp"

cup=${array[0]}
end=${#array[@]}
runs=100

_cup=$cup
# initialize the next array with input cups
for _next in "${array[@]:1}"; do
    next[$_cup]=$_next
    _cup=$_next
done
next[$_cup]=$cup                                  # close the ring

_cup=$cup

declare -i _1st _2nd _3rd dest
# make the moves: a simple sub-linked list operation.
for ((i = 1; i <= runs; ++i)); do
    _1st="${next[$cup]}"
    _2nd="${next[$_1st]}"
    _3rd="${next[$_2nd]}"
    dest=$cup
    while
        (( --dest > 0 )) || dest=$end
        (( dest == _1st || dest == _2nd || dest == _3rd ))
    do :; done
    (( tmp=next[$dest], next[$dest]=_1st, next[$cup]=next[$_3rd], next[$_3rd]=tmp ))
    (( cup=next[$cup] ))
done

res=""
_cup=1
while
    (( _cup = next[$_cup] ))
    res+="$_cup"
    (( next[$_cup] != 1 ))
do :; done
printf "%s: res=%s\n" "$CMD" "$res"
exit 0
