#!/bin/bash
#
# ex1.bash: Advent2020 game, day 19/game 1.

CMD=${0##*/}
set -o noglob
shopt -s extglob

declare -a RULE=() MATCH=() STRING=()
# shellcheck disable=SC2034
declare var_0
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
                #[[ ! "$t" =~ ^.$ ]] && t="($t)"
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

buildtree 1 var_0 0
printf "RULE ZERO = %s\n" "${MATCH[0]}"

for str in "${STRING[@]}"; do
    [[ "$str" =~ ^${MATCH[0]}$ ]] && ((res++))
done

printf "%s : res=%d\n" "$CMD" "$res"

exit 0
