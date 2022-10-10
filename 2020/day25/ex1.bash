#!/bin/bash
#
# ex1.bash: Advent2020 game, day 25/game 1.

CMD=${0##*/}
shopt -s extglob
set -o noglob

declare -a pub=(1 1) enc=(1 1) key
read -r "key[0]"
read -r "key[1]"

while true; do
    ((pub[0] = (pub[0] * 7)      % 20201227,
      pub[1] = (pub[1] * 7)      % 20201227,
      enc[0] = (enc[0] * key[0]) % 20201227,
      enc[1] = (enc[1] * key[1]) % 20201227))
    ((pub[0] == key[0])) && res=${enc[1]} && break
    ((pub[1] == key[1])) && res=${enc[0]} && break
done
printf "%s: res=%s\n" "$CMD" "$res"
exit 0
