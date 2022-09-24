#!/usr/bin/env bash

printf "***** EXAMPLE.txt: input value, then output it\n"
printf "Expected: 1\t"
./aoc-c -i 1 < EXAMPLE.txt
printf "Expected: 5\t"
./aoc-c -i 5 < EXAMPLE.txt

printf "\n\n***** EXAMPLE2.txt: equal test, position mode\n"
printf "Expected: 1\t"
./aoc-c -i 8 < EXAMPLE2.txt
printf "Expected: 0\t"
./aoc-c -i 0 < EXAMPLE.txt

printf "\n\n***** EXAMPLE3.txt: less than test, position mode\n"
printf "Expected: 1\t"
./aoc-c -i 8 < EXAMPLE3.txt
printf "Expected: 0\t"
./aoc-c -i 0 < EXAMPLE3.txt

printf "\n\n***** EXAMPLE4.txt: equal test, immediate mode\n"
printf "Expected: 1\t"
./aoc-c -i 8 < EXAMPLE4.txt
printf "Expected: 0\t"
./aoc-c -i 0 < EXAMPLE4.txt

printf "\n\n***** EXAMPLE5.txt: less than test, immediate mode\n"
printf "Expected: 1\t"
./aoc-c -i 8 < EXAMPLE5.txt
printf "Expected: 0\t"
./aoc-c -i 0 < EXAMPLE5.txt

printf "\n\n***** EXAMPLE6.txt: equal/jump test, position mode\n"
printf "Expected: 1\t"
./aoc-c -i 8 < EXAMPLE6.txt
printf "Expected: 0\t"
./aoc-c -i 0 < EXAMPLE6.txt

printf "\n\n***** EXAMPLE7.txt: equal/jump test, immediate mode\n"
printf "Expected: 1\t"
./aoc-c -i 8 < EXAMPLE7.txt
printf "Expected: 0\t"
./aoc-c -i 0 < EXAMPLE7.txt

printf "\n\n***** EXAMPLE8.txt: equal/less/jump test, mixed mode\n"
printf "Expected:999\t"
./aoc-c -i 7 < EXAMPLE8.txt
printf "Expected: 1000\t"
./aoc-c -i 8 < EXAMPLE8.txt
printf "Expected: 1001\t"
./aoc-c -i 9 < EXAMPLE8.txt
