#!/usr/bin/bash

print_tile() {
    local -i t=$1
    # shellcheck disable=SC2206
    local -a src=(${strings[$t]})
    printf "%s\n" "${src[@]}"
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
