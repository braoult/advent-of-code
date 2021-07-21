#!/bin/bash
#
# ex2.bash: Advent2020 game, day 19/game 2.

CMD=${0##*/}
set -o noglob
shopt -s extglob

declare -a RULE=() MATCH=() STRING=()
# shellcheck disable=SC2034
declare var_0 rule8 rule11
declare -i res=0

# build a regexp use-able by bash =~ operator.
# Recursively replace rules, until we get final text values.
# To avoid running subshell, we must create local variables w/ different
# names at each recursion level (to avoid "circular name reference" error).
function buildtree {
    local -i prof="$1"                            # to allow ≠ local names
    local -n upname="$2"                          # caller variable name
    local name="var_$prof"                        # local var name, w/ depth
    eval "local $name"                            # ... declared here
    shift 2
    local args=$* res="" arg t

    for arg in $args; do
        if [[ -z "${arg/[|ab]}" ]]; then
            res+="$arg"
        else
            if [[ ! -v MATCH[$arg] ]]; then
                buildtree "$((prof+1))" "$name" "${RULE[$arg]}"
                t="${!name}"
                if [[ ! "$t" =~ ^[ab]+$ ]] && [[ "${t:0:1}${t: -1}" != "()" ]]; then
                    t="($t)"
                fi
                MATCH[$arg]="$t"
            fi
            res+=${MATCH[$arg]}
        fi
    done
    # shellcheck disable=SC2034
    upname="$res"
}

while read -r line; do
    case ${line:0:1} in
        [0-9]*)                                   # rule
            rule=${line%:*}
            val=${line#*: }
            RULE[$rule]="${val//\"}"
            ;;
        [a-z]*)                                   # string
            STRING+=("$line")
            ;;
    esac
done

# Old rules:
# 8: 42
# 11: 42 31
# New rules:
# 8: 42 | 42 8
# --> (${M[42]}+)
# 11: 42 31 | 42 11 31
# --> (${M[42]}${M[31]} | ${M[42]}${M[42]}${M[31]}${M[31]} etc...
# --> (OR (${M[42]}{n})${M[31]}{n})), with 1 <= n <= SOME_LIMIT


# hack:
# 1) build rules 42 and 31
# 2) calculate rule 8 and 11 with some "guessed" minimum possible value
# 3) build rule 0

# result is "stable" from this value
SOME_LIMIT=5

buildtree 1 var_0 42
buildtree 1 var_0 31
rule8="${MATCH[42]}+"
rule11="${MATCH[42]}${MATCH[31]}"
for ((i=2; i<SOME_LIMIT; ++i)); do
    rule11+="|${MATCH[42]}{$i}${MATCH[31]}{$i}"
done
MATCH[8]="$rule8"
MATCH[11]="($rule11)"
buildtree 1 var_0 0
for str in "${STRING[@]}"; do
    [[ "$str" =~ ^${MATCH[0]}$ ]] && ((res++))
done

printf "%s : res=%d\n" "$CMD" "$res"
