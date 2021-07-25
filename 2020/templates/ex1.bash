#!/bin/bash
#
# ex1.bash: Advent2020 game, day 3/game 1.

CMD=${0##*/}

XMOVE=3											  # 3 right

declare -i xpos=0 ypos=0 count=0 linelen=0 modpos=0

read -r line									  # ignore 1st line

while read -r line; do
	(( ypos++ ))
	(( xpos += XMOVE ))							  # move right
	(( linelen = ${#line} ))
	(( modpos = (xpos % linelen) ))
	[[ ${line:modpos:1} = \# ]] && ((count++))
done
printf "%s : lines:%d pos=%d found:%d\n" "$CMD" $ypos $xpos $count
exit 0
