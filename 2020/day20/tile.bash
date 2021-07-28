#!/usr/bin/bash

rotate_final() {
    local -i i j len
    local -a new
    local line
    len=${#final[@]}
    for ((i=len-1; i>=0; --i)); do
        line=${final[$i]}
        for ((j=0; j<len; ++j)); do
            new[$j]+=${line:$j:1}
        done
        #dst+=("$str2")
    done
    final=("${new[@]}")
}

flipv_final() {
    local -i i len
    local -a dst
    len=${#final[@]}
    for ((i=0; i<len; ++i)); do
        dst[$((len-1-i))]=${final[$i]}
    done
    final=("${dst[@]}")
}

fliph_final() {
    local -i i j len
    local str1 str2
    local -a dst
    local str1 str2

    len=${#final[@]}
    for ((i=0; i<len; ++i)); do
        str1=${final[$i]}
        str2=""
        for ((j=len-1; j>=0; --j)); do
            str2+=${str1:$j:1}
        done
        dst+=("$str2")
    done
    final=("${dst[@]}")
}

final_add() {
    local -i start=$1 r t
    # shellcheck disable=SC2206
    local -a src=(${strings[$2]})

    for ((r=1, t=$((start*8)); r<9; ++r, t++)); do
        final[$t]+=${src[$r]:1:8}
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
# transform $2 tile to match $1 on left
attach_left() {
    local _l="$1"
    local -i t="$2" m=0
    local s

    for s in ${T[$t]} ${R[$t]} ${B[$t]} ${L[$t]} \
                      ${RT[$t]} ${RR[$t]} ${RB[$t]} ${RL[$t]}; do
        if [[ "$s" == "$_l" ]]; then
            break
        fi
        ((m++))
    done
    case $m in
        0)                                        # top
            r270 "$t"
            flip_v "$t"
            ;;
        1)                                        # right
            flip_h "$t"
            ;;
        2)                                        # bottom
            r90 "$t"
            ;;
        3)                                        # left
            ;;
        4)                                        # rev top
            r270 "$t"
            ;;
        5)                                        # rev right
            r180 "$t"
            ;;
        6)                                        # rev bottom
            r90 "$t"
            flip_v "$t"
            ;;
        7)                                        # rev left
            flip_v "$t"
            ;;
    esac
}

# transform $2 tile to match $1 on left
attach_top() {
    local _l="$1"
    local -i t="$2" m=0
    local s

    for s in ${T[$t]} ${R[$t]} ${B[$t]} ${L[$t]} \
                      ${RT[$t]} ${RR[$t]} ${RB[$t]} ${RL[$t]}; do
        if [[ "$s" == "$_l" ]]; then
            break
        fi
        ((m++))
    done
    case $m in
        0)                                        # top
            ;;
        1)                                        # right
            r270 "$t"
            ;;
        2)                                        # bottom
            flip_v "$t"
            ;;
        3)                                        # left
            r90 "$t"
            flip_h "$t"
            ;;
        4)                                        # rev top
            flip_h "$t"
            ;;
        5)                                        # rev right
            r270 "$t"
            flip_h "$t"
            ;;
        6)                                        # rev bottom
            r180 "$t"
            ;;
        7)                                        # rev left
            r90 "$t"
            ;;
    esac
}
