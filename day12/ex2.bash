#!/bin/bash
#
# ex2.bash: Advent2020 game, day 12/game 2.

CMD=${0##*/}
shopt -s extglob

declare -i X=0 Y=0 WX=10 WY=1
declare -i DIRECTION=90

while read -r line; do
	C=${line:0:1}
	A=${line:1}
	DIRECTION=0

	case "$C" in
		R) ((DIRECTION=A))
		   ;;
		L) ((DIRECTION=360-A))
		   ;;
		F) for ((i=0; i<A; ++i)); do
			   ((X+=WX))
			   ((Y+=WY))
		   done
		   ;;
		N) ((WY+=A))
		   ;;
		S) ((WY-=A))
		   ;;
		E) ((WX+=A))
		   ;;
		W) ((WX-=A))
		   ;;
	esac
	if ((DIRECTION > 0)); then
		for ((i=90; i<=DIRECTION; i+=90)); do
			((tx=WY))
			((WY=-WX))
			((WX=tx))
		done
	fi
done

((X<0)) && ((X=-X))
((Y<0)) && ((Y=-Y))
printf "%s : res=%d\n" "$CMD" $((X+Y))

exit 0
