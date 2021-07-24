#!/bin/bash
#
# ex1.bash: Advent2020 game, day 20/game 1.

CMD=${0##*/}
set -o noglob
shopt -s extglob

declare -A strings
declare -a T R B L RT RR RB RL nums

declare -i count=-1
while read -r line; do
    case ${line:0:1} in
        T*)
            ((count++))
            num=${line##* }
            num=${num%%:}
            nums[$count]="$num"
            printf "%s\n" "$num"
            printf "size nums=%s\n" "${#nums[@]}"
            ;;
        \#|.)
            strings[$count]="${strings[$count]} $line"
            ;;
    esac
done
((count--))
#printf "string 2311=[%s]\n" "${strings[2311]}"

for key in "${!nums[@]}"; do
    # shellcheck disable=SC2206
    str=(${strings[$key]})
    printf "str=%d\n" "${#str[@]}"
    top="${str[0]}"
    bottom="${str[9]}"
    T+=("$top")
    B+=("$bottom")
    # find out right and bottom
    unset left right
    for ((i=0; i<10; ++i)); do
        left+=${str[$i]:0:1}
        right+=${str[$i]: -1:1}
    done
    R+=("$right")
    L+=("$left")
    unset fliptop flipbottom flipleft flipright
    for ((i=9; i>=0; --i)); do
        fliptop+=${top:$i:1}
        flipbottom+=${bottom:$i:1}
        flipleft+=${left:$i:1}
        flipright+=${right:$i:1}
    done
    RT+=("$fliptop")
    RR+=("$flipright")
    RB+=("$flipbottom")
    RL+=("$flipleft")

    printf "%d: top=%s right=%s bottom=%s left=%s\n" "$key" "$top" "$right" "$bottom" "$left"
    printf "%d: fliptop=%s flipright=%s flipbottom=%s flipleft=%s\n" "$key" "$fliptop" "$flipright" "$flipbottom" "$flipleft"

done

ALL=("${T[@]}" "${R[@]}" "${B[@]}" "${L[@]}" "${RT[@]}" "${RR[@]}" "${RB[@]}" "${RL[@]}")
#ALL=("${T[@]}" "${R[@]}" "${B[@]}" "${L[@]}" "${RT[@]}" "${RL[@]}")
printf "%d %s\n" "${#ALL[@]}" "${ALL[0]}"
declare -i res=1
for ((i=0; i<${#nums[@]}; ++i)); do
    count=0
    #printf "%d: " "$i"
    #printf "%s " ${T[$i]} ${R[$i]} ${B[$i]} ${L[$i]} ${RT[$i]} ${RR[$i]} ${RB[$i]} ${RL[$i]}
    #echo

    for ((j=0; j<${#nums[@]}; ++j)); do
        if ((j != i)); then
            #printf "i=%d j=%d\n" "$i" "$j"
            ALL=("${T[$j]}" "${R[$j]}" "${B[$j]}" "${L[$j]}" "${RT[$j]}" \
                            "${RR[$j]}" "${RB[$j]}" "${RL[$j]}")
            for s in "${ALL[@]}"; do
                #[[ $a == "$s" ]] && ((count++))
                for t in ${T[$i]} ${R[$i]} ${B[$i]} ${L[$i]} #\
                                  #${RT[$i]} ${RR[$i]} ${RB[$i]} ${RL[$i]}; do
                    do printf "match %d %s with %d %s: " "$i" "$s" "$j" "$t"
                    if [[ $t == "$s" ]]; then
                        printf "yes\n"
                        ((count++))
                    else
                        :
                        printf "no\n"
                    fi
                done
            done

        fi
    done
    printf "COUNT(%d)=%d key=%s\n" "$i" "$count" "${nums[$i]}"
    if ((count == 2)); then
        printf " --> %s\n" "${nums[$i]}"
        ((res*=${nums[$i]}))
    fi
done

printf "%s : res=%d\n" "$CMD" "$res"

exit 0
