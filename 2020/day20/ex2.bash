#!/bin/bash
#
# ex1.bash: Advent2020 game, day 20/game 2.

CMD=${0##*/}
set -o noglob
shopt -s extglob

source tile.bash

#declare -A strings
declare -a strings T R B L RT RR RB RL nums
declare -a final finalfoo
declare -A FINAL
declare -a CORNER EDGE CENTRAL
declare -i count=-1 SQUARE

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
#echo BORDERS:
#print_all_borders
#exit 0
ALL="${T[*]} ${R[*]} ${B[*]} ${L[*]} ${RT[*]} ${RR[*]} ${RB[*]} ${RL[*]} "
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
for ((i=0; i<${#CORNER[@]}; ++i)); do
    printf "corner[%d]=%d\n" "$i" "${nums[$i]}"
done

#printf "ALL=%s\n" "$ALL"
printf "ALLSIZE=%d\n" "$ALLSIZE"
printf "%s : count=%d\n" "$CMD" "$count"

for ((row=0; row<SQUARE; ++row)); do
    for ((col=0; col<SQUARE; ++col)); do
        for k in "${!FINAL[@]}"; do
            printf "final[%s]=%d\n" "$k" "${FINAL[$k]}"
        done
        found=0
        printf "square(%d,%d)\n" "$row" "$col"
        ################################## 1st tile (top left)
        if ((row==0 && col==0)); then
            # Choose one corner for top-left, find the correct orientation
            i=${CORNER[0]}
            printf "corner[0]=%d\n" "${nums[$i]}"
            unset "CORNER[0]"
            CORNER=("${CORNER[@]}")

            print_tile "$i"; echo
            t=${L[$i]}
            S=${ALL//$t}
            printf "left matches=%d\n" $((ALLSIZE-${#S}))
            t=${T[$i]}
            S=${ALL//$t}
            printf "top matches=%d\n" $((ALLSIZE-${#S}))

            t=${B[$i]}
            S=${ALL//$t}
            # flip vertical if bottom is a corner side
            printf "bottom matches=%d\n" $((ALLSIZE-${#S}))
            if (( ALLSIZE-${#S} == 10 )); then
                echo FLIP_V
                flip_v "$i"
            fi
            print_tile "$i"; echo
            t=${R[$i]}
            S=${ALL//$t}
            # flip horizontal if right is a corner side
            printf "right matches=%d\n" $((ALLSIZE-${#S}))
            if (( ALLSIZE-${#S} == 10 )); then
                echo FLIP_H
                flip_h "$i"
            fi
            print_tile "$i"
            FINAL[0,0]="$i"
            final+=("${nums[$i]}")
            final_add "$row" "$i"
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
            #printf "FINAL[0,%d]=%d\n" "$((col-1))" "${FINAL[0,$((col-1))]}"
            right right "${strings[$l]}"
            index=0
            for j in "${list[@]}"; do
                SIDES="${T[$j]} ${R[$j]} ${B[$j]} ${L[$j]} ${RT[$j]} ${RR[$j]} ${RB[$j]} ${RL[$j]}"
                LENGTH=${#SIDES}
                S=${SIDES//$right}
                # 10 is line size
                ((c = (LENGTH-${#S})/10))
                if ((c > 0)); then
                    echo "border tile match: idx=$index $j [${nums[$j]}]"
                    print_tile "$j"
                    FINAL[$row,$col]="$j"
                    final+=("${nums[$j]}")
                    echo "c=$c"
                    attach_left "$right" "$j"
                    final_add "$row" "$j"
                    if ((col < SQUARE-1)); then
                        unset "EDGE[$index]"
                        EDGE=("${EDGE[@]}")
                        printf "after removing EDGE $j\n"
                        printf "EDGE[%d]=%s\n" "${#EDGE[@]}" "${EDGE[*]}"
                    else
                        unset "CORNER[$index]"
                        CORNER=("${CORNER[@]}")
                        printf "after removing CORNER $j\n"
                        printf "CORNER[%d]=%s\n" "${#CORNER[@]}" "${CORNER[*]}"
                    fi
                    found=1
                    break
                fi
                ((index++))
            done
            if ((found==0)); then
                printf "NOT FOUND\n"
                exit 0
            fi
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
            index=0
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
                    final_add "$row" "$j"
                    if ((row < SQUARE-1)); then
                        unset "EDGE[$index]"
                        EDGE=("${EDGE[@]}")
                    else
                        unset "CORNER[$index]"
                        CORNER=("${CORNER[@]}")
                    fi
                    found=1
                    break
                fi
                ((index++))
            done
            if ((found==0)); then
                printf "NOT FOUND\n"
                exit 0
            fi
            continue
        fi
        ################################## rest of last row
        if ((row == SQUARE-1)); then
            l=${FINAL[$((row-1)),$col]}
            printf "FINAL[%d,%d]=%d\n" "$((row-1))" "$col" "${FINAL[$((row-1)),$col]}"
            bottom bottom "${strings[$l]}"
            index=0
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
                    final_add "$row" "$j"
                    unset "EDGE[$index]"
                    EDGE=("${EDGE[@]}")
                    found=1
                    break
                fi
                ((index++))
            done
            if ((found==0)); then
                printf "NOT FOUND\n"
                exit 0
            fi
            continue
        fi
        ################################## central tiles
        l=${FINAL[$((row-1)),$col]}
        printf "FINAL[%d,%d]=%d\n" "$((col-1))" "${FINAL[$,$((col-1))]}"
        bottom bottom "${strings[$l]}"
        index=0
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
                final_add "$row" "$j"
                unset "CENTRAL[$index]"
                CENTRAL=("${CENTRAL[@]}")
                found=1
                break
            fi
            ((index++))
        done
        if ((found==0)); then
            printf "NOT FOUND\n"
            exit 0
        fi
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

# dragon:
#    01234567890123456789
# 0                    #
# 1  #    ##    ##    ###
# 2   #  #  #  #  #  #
find_dragons() {
    local drag l1 l2 l3
    local -i found=0 r c len
    printf "square=%d\n" "$SQUARE"
    len=${#finalfoo[@]}
    for ((r=0; r<len-2; ++r)); do
        l1=${finalfoo[$r]}
        l2=${finalfoo[$((r+1))]}
        l3=${finalfoo[$((r+2))]}
        for ((c=0; c<len-20; ++c)); do
            drag=${l1:$c+18:1}${l2:$c:1}${l2:$c+5:2}${l2:$c+11:2}${l2:$c+17:3}
            drag+=${l3:$c+1:1}${l3:$c+4:1}${l3:$c+7:1}${l3:$c+10:1}${l3:$c+13:1}${l3:$c+16:1}
            printf "dragon=%s len=%d\n" "$drag" "${#drag}"
            if [[ "$drag" == '###############' ]]; then
                printf "found dragon at (%d,%d)\n" "$r" "$c"
                ((found++))
            fi
        done
    done
    return $found
}

printf "\n"
for k in "${!finalfoo[@]}"; do
    printf "%s\n" "${finalfoo[$k]}"
done
for ((i=0; i<4; ++i)); do
    find_dragons
    found=$?
    ((found>0)) && break

    fliph_final
    printf "FLIPH_DRAG\n"
    for k in "${!finalfoo[@]}"; do
        printf "%s\n" "${finalfoo[$k]}"
    done
    find_dragons
    found=$?
    ((found>0)) && break
    fliph_final

    flipv_final
    printf "FLIPV_DRAG\n"
    for k in "${!finalfoo[@]}"; do
        printf "%s\n" "${finalfoo[$k]}"
    done
    find_dragons
    found=$?
    ((found>0)) && break
    flipv_final

    rotate_final
    printf "ROTATE_DRAG\n"
    for k in "${!finalfoo[@]}"; do
        printf "%s\n" "${finalfoo[$k]}"
    done
done

fullstr="${finalfoo[*]}"
fullstr="${fullstr//[. ]}"
sharp=${#fullstr}
printf "found=%d \n" $((sharp - found*15))

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
