#! /bin/bash
PATH=.:$PATH
IN="$1"
echo IN=$IN
time for i in ex1-c ex2-c; do "$i" < "$IN"; done 2>&1
