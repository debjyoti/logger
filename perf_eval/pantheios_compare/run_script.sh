#!/bin/bash
# Compiles and runs the compute_and_print.c with and without
# optimizations.
# With Optimizations: Using the logger.c from src
# Without Optimizations: Using the local file logger_without*

#set -o verbose
set -o nounset
set -o errexit

unset DISABLE_PANTHEIOS

PANTHEIOS_PATH=/cygdrive/C/Debjyoti/code/pantheios-1.0.1-beta214/
STLSOFT_PATH=/cygdrive/C/Debjyoti/code/stlsoft-1.9.117/

echo
echo "Compiling compute_and_print.c with pantheios"
gcc -O0 -g -c compute_and_print.c -o compute_and_print.o -I$PANTHEIOS_PATH/include -I$STLSOFT_PATH/include
g++ -g compute_and_print.o -o compute_and_print -L$PANTHEIOS_PATH/lib/  -lpantheios.1.core.gcc46 -lpantheios.1.be.file.gcc46 -lpantheios.1.bec.file.gcc46 -lpantheios.1.be.fprintf.gcc46 -lpantheios.1.bec.fprintf.gcc46 -lpantheios.1.fe.all.gcc46 -lpantheios.1.util.gcc46 -lm

echo
echo "Compiling compute_and_print.c with normal printf"
gcc -O0 -DDISABLE_PANTHEIOS -g -c compute_and_print.c -o compute_and_print.o
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
echo "Executing compute_and_print with pantheios"
echo '----------------------------------------------'
echo "Executing compute_and_print with pantheios" >> compute_and_print.out
echo '----------------------------------------------' >> compute_and_print.out
time ./compute_and_print >> compute_and_print.out

echo
echo "Executing compute_and_print without pantheios"
echo '-------------------------------------------------'
echo "Executing compute_and_print without pantheios" >> compute_and_print_no_opt.out
echo '-------------------------------------------------' >> compute_and_print_no_opt.out
time ./compute_and_print_no_opt >> compute_and_print_no_opt.out


done;

echo
echo "Done. Check the .out files"
echo
