#!/bin/bash
#
# ex1.bash: Advent2020 game, day 20/game 2.

CMD=${0##*/}
set -o noglob
shopt -s extglob

source tile.bash

#declare -A strings
declare -a strings T R B L RT RR RB RL nums
declare -A final
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

#test start
# key=1
# str=(${strings[$key]})
# top1="${str[0]}"
# bottom1="${str[9]}"
# declare top bottom left right
# unset left1 right1
# for ((i=0; i<10; ++i)); do
#     left1+=${str[$i]:0:1}
#     right1+=${str[$i]: -1:1}
# done
# top top ${str[@]}
# bottom bottom ${str[@]}
# right right ${str[@]}
# left left ${str[@]}
# print_tile 1
# printf "T=%s\n" "$top1"
# printf "T=%s\n" "$top"
# echo
# printf "R=%s\n" "$right1"
# printf "R=%s\n" "$right"
# echo
# printf "B=%s\n" "$bottom1"
# printf "B=%s\n" "$bottom"
# echo
# printf "L=%s\n" "$left1"
# printf "L=%s\n" "$left"
# echo

# for ((i=9; i>=0; --i)); do
#     fliptop1+=${top:$i:1}
#     flipbottom1+=${bottom:$i:1}
# done
# flip fliptop $top
# flip flibottom $bottom
# printf "T=%s\n" "$fliptop1"
# printf "T=%s\n" "$fliptop"
# echo

# exit 0
#test end

declare top bottom left right
declare fliptop flipbottom flipleft flipright

for key in "${!nums[@]}"; do
    # shellcheck disable=SC2206
    str=(${strings[$key]})
    top top "${str[@]}"
    right right "${str[@]}"
    bottom bottom "${str[@]}"
    left left "${str[@]}"
    flip fliptop "$top"
    flip flipright "$right"
    flip flipbottom "$bottom"
    flip flipleft "$left"

    T+=("$top")
    B+=("$bottom")
    R+=("$right")
    L+=("$left")
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
            CORNER+=("$i")
            ((res*=${nums[$i]}))
            ;;
        7)
            EDGE+=("$i")
            ;;
        8)
            CENTRAL+=("$i")
            ;;
    esac

done


printf "CENTRAL[%d]=%s\n" "${#CENTRAL[@]}" "${CENTRAL[*]}"
printf "EDGE[%d]=%s\n" "${#EDGE[@]}" "${EDGE[*]}"
printf "CORNER[%d]=%s\n" "${#CORNER[@]}" "${CORNER[*]}"
printf "%s : res=%d\n" "$CMD" "$res"

i=${CORNER[0]}
printf "corner=%d=%d\n" "$i" "${nums[$i]}"
printf "origin:\n"
print_tile "$i"
flip_h "$i"
printf "horizontal flip:\n"
print_tile "$i"
flip_h "$i"
flip_v "$i"
printf "vertical flip:\n"
print_tile "$i"
flip_v "$i"
printf "origin:\n"
print_tile "$i"

declare l="" r=""
rightcol r "${strings[$i]}"
printf "right=%s\n" "$r"
printf "origin:\n"
print_tile "$i"
r=""; l=""
leftcol l "${strings[$i]}"
printf "left =%s\n" "$l"
printf "origin:\n"
print_tile "$i"


exit 0


# Choose one corner for top-left, find the correct flip
i=${CORNER[0]}
printf "corner[0]=%d\n" "${nums[$i]}"
print_tile "$i"; echo
t=${B[$i]}
S=${ALL//$t}
# flip vertical if bottom is a corner side
if (( ALLSIZE-${#S} == 10)); then
    echo FLIP_V
    flip_v "$i"
fi
print_tile "$i"; echo

i=${CORNER[0]}
t=${R[$i]}
S=${ALL//$t}
if (( ALLSIZE-${#S} == 10)); then
    echo FLIP_H
    flip_h "$i"
fi
print_tile "$i"
final[0,0]="${strings[$i]}"
unset strings[$i]

# find next tile
#for i in ${EDGE[@]}; do

#done


    exit 0
printf "corner[0]=%d\n" "${nums[$i]}"
printf "origin:\n"
print_tile "$i"
flip_h "$i"
printf "horizontal flip:\n"
print_tile "$i"
flip_h "$i"
flip_v "$i"
printf "vertical flip:\n"
print_tile "$i"
flip_v "$i"
printf "origin:\n"
print_tile "$i"
r90 "$i"
printf "r90:\n"
print_tile "$i"

#for i in "${CORNER[@]}"; do
t=${T[$i]}; S=${ALL//$t}; ((count=(ALLSIZE-${#S})/10))
printf "T  %s: %d\n" "$t" "$count"
t=${R[$i]}; S=${ALL//$t}; ((count=(ALLSIZE-${#S})/10))
printf "R  %s: %d\n" "$t" "$count"
t=${B[$i]}; S=${ALL//$t}; ((count=(ALLSIZE-${#S})/10))
printf "B  %s: %d\n" "$t" "$count"
t=${L[$i]}; S=${ALL//$t}; ((count=(ALLSIZE-${#S})/10))
printf "L  %s: %d\n" "$t" "$count"

t=${RT[$i]}; S=${ALL//$t}; ((count=(ALLSIZE-${#S})/10))
printf "RT %s: %d\n" "$t" "$count"
t=${RR[$i]}; S=${ALL//$t}; ((count=(ALLSIZE-${#S})/10))
printf "RR %s: %d\n" "$t" "$count"
t=${RB[$i]}; S=${ALL//$t}; ((count=(ALLSIZE-${#S})/10))
printf "RB %s: %d\n" "$t" "$count"
t=${RL[$i]}; S=${ALL//$t}; ((count=(ALLSIZE-${#S})/10))
printf "RL %s: %d\n" "$t" "$count"

exit 0
for t in ${T[$i]} ${R[$i]} ${B[$i]} ${L[$i]} \
                      ${RT[$i]} ${RR[$i]} ${RB[$i]} ${RL[$i]}; do
        S=${ALL//$t}
        ((count=(ALLSIZE-${#S})/10))
        printf "%s: %d\n" "$t" "$count"
    done
    echo
#done

exit 0
r90 "$i"
printf "r90:\n"
print_tile "$i"
r90 "$i"
r90 "$i"
r90 "$i"
printf "origin:\n"
print_tile "$i"

r180 "$i"
printf "r180:\n"
print_tile "$i"
r180 "$i"
printf "origin:\n"
print_tile "$i"

r270 "$i"
printf "r270:\n"
print_tile "$i"
r90 "$i"
printf "origin:\n"
print_tile "$i"
