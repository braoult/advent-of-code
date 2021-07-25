#!/bin/bash
#
# ex2.bash: Advent2020 game, day 18/game 2.

CMD=${0##*/}
set -o noglob

# shellcheck disable=2034
declare -a STACK=()
declare -a LINE=()

function enqueue() {
    local -n stack="$1"
    local var="$2"
    stack=("$var" "${stack[@]}")
}
function dequeue() {
    # shellcheck disable=2178
    local -n stack="$1"
    local -n var="$2"
    local last=$((${#stack[@]} - 1))
    var=${stack[$last]}
    unset 'stack[$last]'
    stack=( "${stack[@]}" )                       # pack
}
function getqueue() {
    # shellcheck disable=2178
    local -n stack="$1"
    local -n var="$2"
    local pos="$3"

    var=${stack[$pos]}
    unset 'stack[$pos]'
    #stack=( "${stack[@]}" )                       # DO NOT pack !
}
function setqueue() {
    local -n stack="$1"
    local val="$2"
    local pos="$3"

    stack[$pos]="$val"
}
function print_stack() {
    # shellcheck disable=2178
    local -n stack="$1"
    local k
    printf "stack [%s:%d]: " "$1" "${#stack[@]}"
    for k in "${!stack[@]}"; do
        printf "%s:%s " "$k" "${stack[$k]}"
    done
    printf "\n"
}

function eval_plus() {
    local -n lvalop="$1" loptop="$2"
    local -i v v1 v2 pos last="${#loptop[@]}"

    for ((pos=0; pos<last; ++pos)); do
        if [[ "${loptop[$pos]}" == '+' ]]; then
            getqueue lvalop v1 "$pos"
            getqueue lvalop v2 "$((pos+1))"
            getqueue loptop op "$pos"
            # shellcheck disable=SC1102
            v=$((v1 + v2))
            setqueue lvalop "$v" $((pos+1))
        fi
    done
    lvalop=("${lvalop[@]}")                     # pack arrays
    loptop=("${loptop[@]}")
}

function eval_op() {
    # shellcheck disable=2034,2178
    local -n lvalop="$1" loptop="$2"
    # shellcheck disable=2034
    local v v1 v2 op res=-1
    while ((${#lvalop[@]} > 1)); do
        dequeue lvalop v1
        dequeue lvalop v2
        dequeue loptop op
        # shellcheck disable=SC1102
        v=$((v1 "$op" v2))
        enqueue lvalop "$v"
        ((res++))
    done
    return "$res"
}

function find_paren() {
    local -n expr_="$1"
    local -n var_="$2"
    local -i i
    local k val
    var_=-1
    for k in "${!expr_[@]}"; do
        val=${expr_[$k]}
        [[ $val == '(' ]] && i="$k"
        if [[ $val == ')' ]]; then
            # shellcheck disable=SC2034
            var_="$i"
            break
        fi
    done
}

function eval_expr() {
    local -i cur="$2" val len=${#LINE[@]}
    local op
    # shellcheck disable=SC2034
    local -a lstack=() lop=() LINE_=()

    while ((cur < len)); do
        symbol=${LINE[$cur]}
        case "$symbol" in
            [0-9]*)
                enqueue lstack "${LINE[$cur]}"
                unset 'LINE[$cur]'
                ;;
            \))
                break
                ;;
            [\+\*]*)
                enqueue lop "${LINE[$cur]}"
                unset 'LINE[$cur]'
                ;;
            *)
                exit 1
                ;;
        esac
        ((cur++))
    done
    eval_plus lstack lop
    eval_op lstack lop

    dequeue lstack val
    LINE[$cur]="$val"
    LINE=("${LINE[@]}")                   # pack
}

declare -i res=0 paren=0

while read -r line; do
    # cleanup input line, force split around operators and parens
    line=${line//\*/ * }
    line=${line//+/ + }
    line=${line//-/ - }
    line=${line//\// \/ }
    line=${line//\(/ \( }
    line=${line//\)/ \) }
    read -ra LINE <<< "$line"
    paren=0
    while ((${#LINE[@]} > 1 && paren >= 0)); do
        find_paren LINE paren
        if (( paren >= 0 )); then
            unset 'LINE[$paren]'
            LINE=("${LINE[@]}")                     # pack
            eval_expr STACK "$paren"
        fi
    done
    if ((${#LINE[@]} > 1)); then
        eval_expr STACK 0
    fi
    ((res+=LINE[0]))
done

printf "%s : res=%d\n" "$CMD" "$res"

exit 0
