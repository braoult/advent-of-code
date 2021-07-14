#!/bin/bash
#
# ex1.bash: Advent2020 game, day 18/game 1.

CMD=${0##*/}
shopt -s extglob
set -o noglob

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
function eval_op() {
    local -n lvalop="$1" loptop="$2"
    local v v1 v2 op res=-1
    while ((${#lvalop[@]} > 1)); do
        dequeue lvalop v1
        dequeue lvalop v2
        dequeue loptop op
        v=$((v1 $op v2))
        enqueue lvalop "$v"
        ((res++))
    done
    return $res
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
            var_="$i"
            break
        fi
    done
}

function eval_expr() {
    local -n gstack="$1"
    #local -n expr=(${LINE[@]}) #"$3"
    local -i cur="$2" val len=${#LINE[@]}
    local op
    local -a lstack=() lop=() lstack_=() LINE_=()

    while ((cur < len)); do
        symbol=${LINE[$cur]}
        case "$symbol" in
            +([0-9]))
                enqueue lstack "${LINE[$cur]}"
                unset 'LINE[$cur]'
                ;;
            \))
                break
                ;;
            +([\+\*]))
                enqueue lop "${LINE[$cur]}"
                unset 'LINE[$cur]'
                ;;
            *)
                exit 1
                ;;
        esac
        eval_op lstack lop

        ((cur++))
    done
    dequeue lstack val
    LINE[$cur]="$val"
    LINE=(${LINE[@]})                   # pack
}

declare -i res=0 num=0 paren=0

while read -r line; do
    # cleanup input line, force split around operators and parens
    line=${line//\*/ * }
    line=${line//+/ + }
    line=${line//-/ - }
    line=${line//\// \/ }
    line=${line//\(/ \( }
    line=${line//\)/ \) }
    LINE=($line)
    paren=0
    while ((${#LINE[@]} > 1 && paren >= 0)); do
        find_paren LINE paren
        if (( paren >= 0 )); then
            unset LINE[$paren]
            LINE=(${LINE[@]})                     # pack
            eval_expr STACK "$paren"
            LINE=(${LINE[@]})                     # pack
        fi
    done
    if ((${#LINE[@]} > 1)); then
        eval_expr STACK 0
    fi
    ((res+=${LINE[0]}))
done

printf "%s : res=%d\n" "$CMD" "$res"

exit 0
