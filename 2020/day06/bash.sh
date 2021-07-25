#! /bin/bash
PATH=.:$PATH
IN="$1"
time { ex1.bash < $IN; ex2.bash < $IN; } 2>&1
