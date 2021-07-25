#!/bin/bash
#
# ex1.bash: Advent2020 game, day 2/game 1

CMD=${0##*/}

declare -i beg end num found=0 nlines=0

IFS=$':- \t'
while read -r beg end char str; do
	stripped=${str//[^$char]}					  # keep only specified char
	((num = ${#stripped}))						  # occurences of this char
	((num >= beg && num <= end && found++))		  # match ?
	((nlines++))
done
printf "${CMD} : lines: %d matched:%d\n" "$nlines" "$found"

exit 0
