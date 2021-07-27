#!/bin/bash
#
# ex1.bash: Advent2020 game, day 20/game 2.

CMD=${0##*/}
set -o noglob
shopt -s extglob

source tile.bash

#declare -A strings
declare -a strings T R B L RT RR RB RL nums
declare -a final
declare -A FINAL
declare -a CORNER EDGE CENTRAL
declare -i count=-1 res=1 SQUARE

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
((count++))
case "$count" in
    9)
        SQUARE=3
        ;;
    144)
        SQUARE=12
        ;;
    *)
        printf "Fatal: unknown square size."
        exit 0
        ;;
esac
printf "size=%d\n" "$SQUARE"

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

ALL="${T[*]} ${R[*]} ${B[*]} ${L[*]} ${RT[*]} ${RR[*]} ${RB[*]} ${RL[*]}"
ALLSIZE=${#ALL}

for ((i=0; i<${#nums[@]}; ++i)); do
    c=0

    for t in ${T[$i]} ${R[$i]} ${B[$i]} ${L[$i]}; do
        S=${ALL//$t}
        # 10 is line size
        ((c += (ALLSIZE-${#S})/10))
    done
    # 6 is 4 for itself, + 2 for other matching
    #printf "%s: %d\n" "${nums[$i]}" "$c"
    case "$c" in
        6)
            CORNER+=("$i")
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
printf "%s : count=%d\n" "$CMD" "$count"

for ((row=0; row<SQUARE; ++row)); do
    echo ZOBI
    for ((col=0; col<SQUARE; ++col)); do
        for k in ${!FINAL[@]}; do
            printf "final[%s]=%d\n" "$k" "${FINAL[$k]}"
        done
        printf "square(%d,%d)\n" "$row" "$col"
        ################################## 1st tile (top left)
        if ((row==0 && col==0)); then
            # Choose one corner for top-left, find the correct orientation
            i=${CORNER[0]}
            unset "CORNER[0]"
            CORNER=("${CORNER[@]}")

            printf "corner[0]=%d\n" "${nums[$i]}"
            print_tile "$i"; echo
            t=${B[$i]}
            S=${ALL//$t}
            # flip vertical if bottom is a corner side
            if (( ALLSIZE-${#S} == 10 )); then
                echo FLIP_V
                flip_v "$i"
            fi
            print_tile "$i"; echo
            t=${R[$i]}
            S=${ALL//$t}
            # flip horizontal if bottom is a corner side
            if (( ALLSIZE-${#S} == 10 )); then
                echo FLIP_H
                flip_h "$i"
            fi
            print_tile "$i"
            FINAL[0,0]="$i"
            final+=("${nums[$i]}")
            continue
        fi
        ################################## rest of 1st line
        if ((row==0)); then
            if ((col < SQUARE-1)); then
                list=("${EDGE[@]}")
            else
                list=("${CORNER[@]}")
            fi
            l=${FINAL[0,$((col-1))]}
            printf "FINAL[0,%d]=%d\n" "$((col-1))" "${FINAL[0,$((col-1))]}"
            right right "${strings[$l]}"
            for j in "${list[@]}"; do
                SIDES="${T[$j]} ${R[$j]} ${B[$j]} ${L[$j]} ${RT[$j]} ${RR[$j]} ${RB[$j]} ${RL[$j]}"
                LENGTH=${#SIDES}
                S=${SIDES//$right}
                # 10 is line size
                ((c = (LENGTH-${#S})/10))
                if ((c > 0)); then
                    echo "border tile match: ${nums[$j]}"
                    print_tile "$j"
                    FINAL[$row,$col]="$j"
                    final+=("${nums[$j]}")
                    echo "c=$c"
                    attach_left "$right" "$j"
                    if ((col < SQUARE-1)); then
                        unset "EDGE[$j]"
                        EDGE=("${EDGE[@]}")
                    else
                        unset "CORNER[$j]"
                        CORNER=("${CORNER[@]}")
                    fi
                    break
                fi
            done
            continue
        fi
        ################################## 1st and last col
        if ((col==0 || col==SQUARE-1)); then
            if ((row > 0 && row < SQUARE-1)); then
                list=("${EDGE[@]}")
            else
                list=("${CORNER[@]}")
            fi
            l=${FINAL[$((row-1)),$col]}
            printf "FINAL[%d,%d]=%d\n" "$((row-1))" "$col" "$l"
            printf "UPPER TILE"
            print_tile "$j"
            bottom bottom "${strings[$l]}"
            printf "UPPER bottom line: %s\n" "$bottom"
            for j in "${list[@]}"; do
                printf "looking for tile %d [%d]\n" "$j" "${nums[$j]}"
                SIDES="${T[$j]} ${R[$j]} ${B[$j]} ${L[$j]} ${RT[$j]} ${RR[$j]} ${RB[$j]} ${RL[$j]}"
                LENGTH=${#SIDES}
                S=${SIDES//$bottom}
                # 10 is line size
                ((c = (LENGTH-${#S})/10))
                if ((c > 0)); then
                    echo "border tile match: ${nums[$j]}"
                    print_tile "$j"
                    FINAL[$row,$col]="$j"
                    final+=("${nums[$j]}")
                    echo "c=$c"
                    attach_top "$bottom" "$j"
                    if ((row < SQUARE-1)); then
                        unset "EDGE[$j]"
                        EDGE=("${EDGE[@]}")
                    else
                        unset "CORNER[$j]"
                        CORNER=("${CORNER[@]}")
                    fi
                    break
                fi
            done
            continue
        fi
        ################################## rest of last row
        if ((row == SQUARE-1)); then
            l=${FINAL[$((row-1)),$col]}
            printf "FINAL[%d,%d]=%d\n" "$((row-1))" "$col" "${FINAL[$((row-1)),$col]}"
            bottom bottom "${strings[$l]}"
            for j in "${EDGE[@]}"; do
                SIDES="${T[$j]} ${R[$j]} ${B[$j]} ${L[$j]} ${RT[$j]} ${RR[$j]} ${RB[$j]} ${RL[$j]}"
                LENGTH=${#SIDES}
                S=${SIDES//$bottom}
                # 10 is line size
                ((c = (LENGTH-${#S})/10))
                if ((c > 0)); then
                    echo "border tile match: ${nums[$j]}"
                    print_tile "$j"
                    FINAL[$row,$col]="$j"
                    final+=("${nums[$j]}")
                    echo "c=$c"
                    attach_top "$bottom" "$j"
                    unset "EDGE[$j]"
                    EDGE=("${EDGE[@]}")
                    break
                fi
            done
            continue
        fi
        ################################## central tiles
        l=${FINAL[$((row-1)),$col]}
        printf "FINAL[%d,%d]=%d\n" "$((col-1))" "${FINAL[$,$((col-1))]}"
        bottom bottom "${strings[$l]}"
        for j in "${CENTRAL[@]}"; do
            SIDES="${T[$j]} ${R[$j]} ${B[$j]} ${L[$j]} ${RT[$j]} ${RR[$j]} ${RB[$j]} ${RL[$j]}"
            LENGTH=${#SIDES}
            S=${SIDES//$bottom}
            # 10 is line size
            ((c = (LENGTH-${#S})/10))
            if ((c > 0)); then
                echo "border tile match: ${nums[$j]}"
                print_tile "$j"
                FINAL[$row,$col]="$j"
                final+=("${nums[$j]}")
                echo "c=$c"
                attach_top "$bottom" "$j"
                unset "CENTRAL[$j]"
                CENTRAL=("${CENTRAL[@]}")
                break
            fi
        done
        continue

    done
done

for ((r=0; r<SQUARE; ++r)); do
    for ((c=0; c<SQUARE; ++c)); do
        k=${FINAL[$r,$c]}
        printf "%6d" "${nums[$k]}"
    done
    printf "\n"
done

print_final
trim_borders
print_final
printf "%s \n" "${final[@]}"

exit 0











###################################################### testing
# printf "corner[0]=%d\n" "${nums[$i]}"
# printf "origin:\n"
# print_tile "$i"
# flip_h "$i"
# printf "horizontal flip:\n"
# print_tile "$i"
# flip_h "$i"
# flip_v "$i"
# printf "vertical flip:\n"
# print_tile "$i"
# flip_v "$i"
# printf "origin:\n"
# print_tile "$i"
# r90 "$i"
# printf "r90:\n"
# print_tile "$i"

# #for i in "${CORNER[@]}"; do
# t=${T[$i]}; S=${ALL//$t}; ((count=(ALLSIZE-${#S})/10))
# printf "T  %s: %d\n" "$t" "$count"
# t=${R[$i]}; S=${ALL//$t}; ((count=(ALLSIZE-${#S})/10))
# printf "R  %s: %d\n" "$t" "$count"
# t=${B[$i]}; S=${ALL//$t}; ((count=(ALLSIZE-${#S})/10))
# printf "B  %s: %d\n" "$t" "$count"
# t=${L[$i]}; S=${ALL//$t}; ((count=(ALLSIZE-${#S})/10))
# printf "L  %s: %d\n" "$t" "$count"

# t=${RT[$i]}; S=${ALL//$t}; ((count=(ALLSIZE-${#S})/10))
# printf "RT %s: %d\n" "$t" "$count"
# t=${RR[$i]}; S=${ALL//$t}; ((count=(ALLSIZE-${#S})/10))
# printf "RR %s: %d\n" "$t" "$count"
# t=${RB[$i]}; S=${ALL//$t}; ((count=(ALLSIZE-${#S})/10))
# printf "RB %s: %d\n" "$t" "$count"
# t=${RL[$i]}; S=${ALL//$t}; ((count=(ALLSIZE-${#S})/10))
# printf "RL %s: %d\n" "$t" "$count"

# exit 0
# for t in ${T[$i]} ${R[$i]} ${B[$i]} ${L[$i]} \
    #                   ${RT[$i]} ${RR[$i]} ${RB[$i]} ${RL[$i]}; do
#     S=${ALL//$t}
#     ((count=(ALLSIZE-${#S})/10))
#     printf "%s: %d\n" "$t" "$count"
# done
# echo

# exit 0
# r90 "$i"
# printf "r90:\n"
# print_tile "$i"
# r90 "$i"
# r90 "$i"
# r90 "$i"
# printf "origin:\n"
# print_tile "$i"

# r180 "$i"
# printf "r180:\n"
# print_tile "$i"
# r180 "$i"
# printf "origin:\n"
# print_tile "$i"

# r270 "$i"
# printf "r270:\n"
# print_tile "$i"
# r90 "$i"
# printf "origin:\n"
# print_tile "$i"

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

# printf "RIGHT=%s\n" "$right"
# # find next tile
# for j in "${EDGE[@]}"; do
#     SIDES="${T[$j]} ${R[$j]} ${B[$j]} ${L[$j]} ${RT[$j]} ${RR[$j]} ${RB[$j]} ${RL[$j]}"
#     LENGTH=${#SIDES}
#     S=${SIDES//$right}
#     # 10 is line size
#     ((c = (LENGTH-${#S})/10))
#     if ((c > 0)); then
#         echo "border tile match: ${nums[$j]}"
#         print_tile "$j"
#         final+=("${nums[$j]}")
#         echo "c=$c"
#         attach_left "$right" "$j"
#         unset "EDGE[$j]"
#         EDGE=("${EDGE[@]}")
#         break
#     fi
# done

# echo after done
# print_tile "$j"
# right right "${strings[$j]}"
# printf "RIGHT=%s\n" "$right"
# # find next tile
# for j in "${CORNER[@]}"; do
#     SIDES="${T[$j]} ${R[$j]} ${B[$j]} ${L[$j]} ${RT[$j]} ${RR[$j]} ${RB[$j]} ${RL[$j]}"
#     LENGTH=${#SIDES}
#     S=${SIDES//$right}
#     # 10 is line size
#     ((c = (LENGTH-${#S})/10))
#     if ((c > 0)); then
#         echo "corner tile match: ${nums[$j]}"
#         final+=("${nums[$j]}")
#         print_tile "$j"
#         echo "c=$c"
#         attach_left "$right" "$j"
#         unset "CORNER[$j]"
#         CORNER=("${CORNER[@]}")
#         break
#     fi
# done



# exit 0
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
right r "${strings[$i]}"
printf "right=%s\n" "$r"
printf "origin:\n"
print_tile "$i"
r=""; l=""
left l "${strings[$i]}"
printf "left =%s\n" "$l"
printf "origin:\n"
print_tile "$i"
echo ZOBI
