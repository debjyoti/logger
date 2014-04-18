#!/bin/bash
# Compiles and runs the compute_and_print.c with and without
# optimizations.
# With Optimizations: Using the logger.c from src
# Without Optimizations: Using the local file logger_without*

#set -o verbose
set -o nounset
set -o errexit

unset DISABLE_ZLOG

echo
echo "Compiling compute_and_print.c with ZLOG"
gcc -O0 -g -c compute_and_print.c -o compute_and_print.o -I/usr/local/include/
gcc -g compute_and_print.o -o compute_and_print -L/usr/local/lib/ -lzlog -lm

echo
echo "Compiling compute_and_print.c with normal printf"
gcc -O0 -DDISABLE_ZLOG -g -c compute_and_print.c -o compute_and_print.o
gcc -O0 -c ../logger_without_optimizations.c -o logger_without_optimizations.o  
gcc -g compute_and_print.o logger_without_optimizations.o -o compute_and_print_no_opt  -lm

echo
echo "Cleaning the object files and log files"
rm *.o *.out *.log* || true

echo
echo 'Start executing 10 times'

for i in {1..10}; do

rm *.log* || true

echo
echo "=======================Iteration $i========================="

echo
echo "Executing compute_and_print with ZLOG"
echo '----------------------------------------------'
echo "Executing compute_and_print with ZLOG" >> compute_and_print.out
echo '----------------------------------------------' >> compute_and_print.out
time LD_LIBRARY_PATH=/usr/local/lib/ ./compute_and_print >> compute_and_print.out

echo
echo "Executing compute_and_print without ZLOG"
echo '-------------------------------------------------'
echo "Executing compute_and_print without ZLOG" >> compute_and_print_no_opt.out
echo '-------------------------------------------------' >> compute_and_print_no_opt.out
time ./compute_and_print_no_opt >> compute_and_print_no_opt.out


done;

echo
echo "Done. Check the .out files"
echo
