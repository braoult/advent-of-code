#!/usr/bin/bash

print_tile() {
    local -i t=$1
    # shellcheck disable=SC2206
    local -a src=(${strings[$t]})
    printf "%s\n" "${src[@]}"
}

print_final() {
    local k str
    local -i l len r c
    local -a tile
    str=${strings[0]%% *}
    len=${#str}
    printf "print_final: len=%d\n" "$len"
    for ((r=0; r<SQUARE; ++r)); do
        for ((l=0; l<len; ++l)); do # each line
            for ((c=0; c<SQUARE; ++c)); do
                k=${FINAL[$r,$c]}
                # shellcheck disable=SC2206
                tile=(${strings[$k]})
                str=${tile[$l]}
                printf "%s " "$str"
            done
            printf "\n"
        done
        printf "\n"
    done
}

assemble_final() {
    local k str
    local -i l len r c
    local -a tile
    str=${strings[0]%% *}
    len=${#str}
    for ((r=0; r<SQUARE; ++r)); do
        for ((l=0; l<len; ++l)); do # each line
            for ((c=0; c<SQUARE; ++c)); do
                k=${FINAL[$r,$c]}
                # shellcheck disable=SC2206
                tile=(${strings[$k]})
                str=${tile[$l]}
                printf "%s " "$str"
            done
            printf "\n"
        done
        printf "\n"
    done
}
rotate_final() {
    local -i i j len
    local -a new
    local line
    len=${#finalfoo[@]}
    printf "rotate foo. len=%d\n" "$len"
    for ((i=len-1; i>=0; --i)); do
        line=${finalfoo[$i]}
        for ((j=0; j<len; ++j)); do
            new[$j]+=${line:$j:1}
        done
        #dst+=("$str2")
    done
    finalfoo=("${new[@]}")
}

flipv_final() {
    local -i i len
    local -a dst
    len=${#finalfoo[@]}
    for ((i=0; i<len; ++i)); do
        dst[$((len-1-i))]=${finalfoo[$i]}
    done
    finalfoo=("${dst[@]}")
}

fliph_final() {
    local -i i j len
    local str1 str2
    local -a dst
    local str1 str2

    len=${#finalfoo[@]}
    for ((i=0; i<len; ++i)); do
        str1=${finalfoo[$i]}
        str2=""
        for ((j=len-1; j>=0; --j)); do
            str2+=${str1:$j:1}
        done
        dst+=("$str2")
    done
    finalfoo=("${dst[@]}")
}

final_add() {
    local -i start=$1 r t
    # shellcheck disable=SC2206
    local -a src=(${strings[$2]})

    for ((r=1, t=$((start*8)); r<9; ++r, t++)); do
        printf "adding tile %d row %d to final %d\n" "$2" "$r" "$t"
        printf "  %s -> " "${finalfoo[$t]}"
        finalfoo[$t]+=${src[$r]:1:8}
        printf "%s\n" "${finalfoo[$t]}"
    done
}

trim_borders() {
    local k s
    for k in "${!strings[@]}"; do
        s=${strings[$k]}
        s=${s//? ?/ }
        s=${s#?}
        s=${s%?}
        s=${s#* }
        s=${s% * }
        strings[$k]="$s"
        #printf "%s\n" "${strings[$k]}"
    done
}

flip_v() {
    local -i t=$1 i
    # shellcheck disable=SC2206
    local -a dst src=(${strings[$t]})
    for ((i=0; i<10; ++i)); do
        dst[$((9-i))]=${src[$i]}
    done
    strings[$t]="${dst[*]}"
}
flip_h() {
    local -i t=$1 i j
    local str1 str2
    # shellcheck disable=SC2206
    local -a dst src=(${strings[$t]})
    local len=${#src[0]}
    local str1 str2
    for ((i=0; i<10; ++i)); do
        str1=${src[$i]}
        str2=""
        for ((j=9; j>=0; --j)); do
            str2+=${str1:$j:1}
        done
        dst+=("$str2")
    done
    strings[$t]="${dst[*]}"
}
r90() {
    local -i t=$1 i j
    # shellcheck disable=SC2206
    local -a dst src=(${strings[$t]}) str2
    local str1
    for ((i=9; i>=0; --i)); do
        str1=${src[$i]}
        for ((j=0; j<10; ++j)); do
            str2[$j]+=${str1:$j:1}
        done
        #dst+=("$str2")
    done
    strings[$t]="${str2[*]}"
}

r180() {
    local -i t=$1 i j
    # shellcheck disable=SC2206
    local -a dst src=(${strings[$t]})
    local str1 str2
    for ((i=9; i>=0; --i)); do
        str1=${src[$i]}
        str2=""
        for ((j=9; j>=0; --j)); do
            str2+=${str1:$j:1}
        done
        dst+=("$str2")
    done
    strings[$t]="${dst[*]}"
}
r270() {
    local -i t=$1 i j
    # shellcheck disable=SC2206
    local -a dst src=(${strings[$t]}) str2
    local str1
    for ((i=0; i<10; ++i)); do
        str1=${src[$i]}
        for ((j=0; j<10; ++j)); do
            str2[$j]+=${str1:9-$j:1}
        done
        #dst+=("$str2")
    done
    strings[$t]="${str2[*]}"
}
left() {
    local -n _r="$1"
    local -i i
    shift
    # shellcheck disable=SC2206
    local -a tile=($*)
    _r=""
    for ((i=0; i<10; ++i)); do
        _r+=${tile[$i]:0:1}
    done
}
right() {
    local -n _r="$1"
    local -i i
    shift
    # shellcheck disable=SC2206
    local -a tile=($*)
    _r=""
    for ((i=0; i<10; ++i)); do
        _r+=${tile[$i]: -1:1}
    done
}
top() {
    local -n _r="$1"
    shift
    # shellcheck disable=SC2206
    local -a tile=($*)
    _r=${tile[0]}
}
bottom() {
    local -n _r="$1"
    shift
    # shellcheck disable=SC2206
    local -a tile=($*)
    _r=${tile[9]}
}
flip() {
    local -n _r="$1"
    local str="$2"
    local -i i
    _r=""
    for ((i=9; i>=0; --i)); do
        _r+=${str:$i:1}
    done
}
print_all_borders() {
    local k str
    local -i i
    for ((i=0; i<${#strings[@]}; ++i)); do
        top str "${strings[$i]}"
        printf "%s\n" "$str"
        right str "${strings[$i]}"
        printf "%s\n" "$str"
        bottom str "${strings[$i]}"
        printf "%s\n" "$str"
        left str "${strings[$i]}"
        printf "%s\n" "$str"
    done
}
# transform $2 tile to match $1 on left
attach_left() {
    local _l="$1"
    local -i t="$2" m=0
    local s
    printf "matching left %s for tile:\n" "$_l"
    print_tile "$t"
    for s in ${T[$t]} ${R[$t]} ${B[$t]} ${L[$t]} \
                      ${RT[$t]} ${RR[$t]} ${RB[$t]} ${RL[$t]}; do
        printf "%d: %s\n" "$m" "$s"
        if [[ "$s" == "$_l" ]]; then
            echo "attach_left: count=$m"
            break
        fi
        ((m++))
    done
    case $m in
        0)                                        # top
            printf "match=top\n"
            r270 "$t"
            flip_v "$t"
            ;;
        1)                                        # right
            printf "match=right\n"
            flip_h "$t"
            ;;
        2)                                        # bottom
            printf "match=bottom\n"
            r90 "$t"
            ;;
        3)                                        # left
            printf "match=left\n"
            ;;
        4)                                        # rev top
            printf "match=rev top\n"
            r270 "$t"
            ;;
        5)                                        # rev right
            printf "match=rev right\n"
            r180 "$t"
            ;;
        6)                                        # rev bottom
            printf "match=rev bottom\n"
            r90 "$t"
            flip_v "$t"
            ;;
        7)                                        # rev left
            printf "match=rev left\n"
            flip_v "$t"
            ;;
    esac
    echo new orientation:
    print_tile "$t"
}

# transform $2 tile to match $1 on left
attach_top() {
    local _l="$1"
    local -i t="$2" m=0
    local s
    printf "matching top %s for tile:\n" "$_l"
    print_tile "$t"
    for s in ${T[$t]} ${R[$t]} ${B[$t]} ${L[$t]} \
                      ${RT[$t]} ${RR[$t]} ${RB[$t]} ${RL[$t]}; do
        printf "%d: %s\n" "$m" "$s"
        if [[ "$s" == "$_l" ]]; then
            echo "attach_top: count=$m"
            break
        fi
        ((m++))
    done
    case $m in
        0)                                        # top
            printf "match=top\n"
            ;;
        1)                                        # right
            printf "match=right\n"
            r270 "$t"
            ;;
        2)                                        # bottom
            printf "match=bottom\n"
            flip_v "$t"
            ;;
        3)                                        # left
            printf "match=left\n"
            r90 "$t"
            flip_h "$t"
            ;;
        4)                                        # rev top
            printf "match=rev top\n"
            flip_h "$t"
            ;;
        5)                                        # rev right
            printf "match=rev right\n"
            r270 "$t"
            flip_h "$t"
            ;;
        6)                                        # rev bottom
            printf "match=rev bottom\n"
            r180 "$t"
            ;;
        7)                                        # rev left
            printf "match=rev left\n"
            r90 "$t"
            ;;
    esac
    echo new orientation:
    print_tile "$t"
}
