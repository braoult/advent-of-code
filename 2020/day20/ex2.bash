#!/bin/bash
#
# ex1.bash: Advent2020 game, day 20/game 2.

CMD=${0##*/}
set -o noglob
shopt -s extglob

source tile.bash

declare -a strings T R B L RT RR RB RL nums
declare -a final
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

for ((row=0; row<SQUARE; ++row)); do
    for ((col=0; col<SQUARE; ++col)); do
        found=0
        ################################## 1st tile (top left)
        if ((row==0 && col==0)); then
            # Choose one corner for top-left, find the correct orientation
            i=${CORNER[0]}
            unset "CORNER[0]"
            CORNER=("${CORNER[@]}")

            t=${B[$i]}
            S=${ALL//$t}
            # flip vertical if bottom is a corner side
            if (( ALLSIZE-${#S} == 10 )); then
                flip_v "$i"
            fi
            t=${R[$i]}
            S=${ALL//$t}
            # flip horizontal if right is a corner side
            if (( ALLSIZE-${#S} == 10 )); then
                flip_h "$i"
            fi
            FINAL[0,0]="$i"
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
            right right "${strings[$l]}"
            index=0
            for j in "${list[@]}"; do
                SIDES="${T[$j]} ${R[$j]} ${B[$j]} ${L[$j]} ${RT[$j]} ${RR[$j]} ${RB[$j]} ${RL[$j]}"
                LENGTH=${#SIDES}
                S=${SIDES//$right}
                # 10 is line size
                ((c = (LENGTH-${#S})/10))
                if ((c > 0)); then
                    FINAL[$row,$col]="$j"
                    attach_left "$right" "$j"
                    final_add "$row" "$j"
                    if ((col < SQUARE-1)); then
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
            bottom bottom "${strings[$l]}"
            index=0
            for j in "${list[@]}"; do
                SIDES="${T[$j]} ${R[$j]} ${B[$j]} ${L[$j]} ${RT[$j]} ${RR[$j]} ${RB[$j]} ${RL[$j]}"
                LENGTH=${#SIDES}
                S=${SIDES//$bottom}
                # 10 is line size
                ((c = (LENGTH-${#S})/10))
                if ((c > 0)); then
                    FINAL[$row,$col]="$j"
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
                exit 0
            fi
            continue
        fi
        ################################## rest of last row
        if ((row == SQUARE-1)); then
            l=${FINAL[$((row-1)),$col]}
            bottom bottom "${strings[$l]}"
            index=0
            for j in "${EDGE[@]}"; do
                SIDES="${T[$j]} ${R[$j]} ${B[$j]} ${L[$j]} ${RT[$j]} ${RR[$j]} ${RB[$j]} ${RL[$j]}"
                LENGTH=${#SIDES}
                S=${SIDES//$bottom}
                # 10 is line size
                ((c = (LENGTH-${#S})/10))
                if ((c > 0)); then
                    FINAL[$row,$col]="$j"
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
                exit 0
            fi
            continue
        fi
        ################################## central tiles
        l=${FINAL[$((row-1)),$col]}
        bottom bottom "${strings[$l]}"
        index=0
        for j in "${CENTRAL[@]}"; do
            SIDES="${T[$j]} ${R[$j]} ${B[$j]} ${L[$j]} ${RT[$j]} ${RR[$j]} ${RB[$j]} ${RL[$j]}"
            LENGTH=${#SIDES}
            S=${SIDES//$bottom}
            # 10 is line size
            ((c = (LENGTH-${#S})/10))
            if ((c > 0)); then
                FINAL[$row,$col]="$j"
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
            exit 0
        fi
        continue
    done
done

# dragon:
#    01234567890123456789
# 0                    #
# 1  #    ##    ##    ###
# 2   #  #  #  #  #  #
find_dragons() {
    local drag l1 l2 l3
    local -i found=0 r c len
    len=${#final[@]}
    for ((r=0; r<len-2; ++r)); do
        l1=${final[$r]}
        l2=${final[$((r+1))]}
        l3=${final[$((r+2))]}
        for ((c=0; c<len-20; ++c)); do
            drag=${l1:$c+18:1}${l2:$c:1}${l2:$c+5:2}${l2:$c+11:2}${l2:$c+17:3}
            drag+=${l3:$c+1:1}${l3:$c+4:1}${l3:$c+7:1}${l3:$c+10:1}${l3:$c+13:1}${l3:$c+16:1}
            if [[ "$drag" == '###############' ]]; then
                ((found++))
            fi
        done
    done
    return $found
}

for ((i=0; i<4; ++i)); do
    find_dragons
    found=$?
    ((found>0)) && break

    fliph_final
    find_dragons
    found=$?
    ((found>0)) && break
    fliph_final

    flipv_final
    find_dragons
    found=$?
    ((found>0)) && break
    flipv_final

    rotate_final
done

fullstr="${final[*]}"
fullstr="${fullstr//[. ]}"
sharp=${#fullstr}
printf "%s res=%d \n" "$CMD" $((sharp - found*15))

exit 0
