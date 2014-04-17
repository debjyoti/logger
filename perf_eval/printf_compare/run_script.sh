#!/bin/bash
# Compiles and runs the compute_and_print.c with and without
# optimizations.
# With Optimizations: Using the logger.c from src
# Without Optimizations: Using the local file logger_without*

#set -o verbose
set -o nounset
set -o errexit

unset DISABLE_OPTIMIZATIONS
unset DISABLE_PRINTING

echo
echo "Compiling compute_and_print.c without any prints"
gcc -DDISABLE_PRINTING -O0 -g compute_and_print.c -o compute_no_print

echo
echo "Compiling compute_and_print.c with ../../src/logger.c"
gcc -O0 -g -c compute_and_print.c -o compute_and_print.o
gcc -O0 -c ../../src/logger.c -o logger.o
gcc -D_THREAD_SAFE -g compute_and_print.o logger.o -o compute_and_print -lpthread

echo
echo "Compiling compute_and_print.c with logger_without_optimizations.c"
gcc -O0 -DDISABLE_OPTIMIZATIONS -g -c compute_and_print.c -o compute_and_print.o
gcc -O0 -c logger_without_optimizations.c -o logger_without_optimizations.o  
gcc -g compute_and_print.o logger_without_optimizations.o -o compute_and_print_no_opt 

echo
echo "Cleaning the bin and out directories"
rm bin/* out/*

echo
echo "Moving the binaries to bin/ folder"
mv compute_no_print bin/
mv compute_and_print bin/
mv compute_and_print_no_opt bin/

echo
echo "Cleaning the object files"
rm *.o

echo
echo "cd to bin folder"
cd bin/

echo
echo 'Start executing 10 times'

for i in {1..10}; do
echo
echo "=======================Iteration $i========================="

echo
echo "Executing compute_no_print: Just to check computation time"
echo '----------------------------------------------------------'
echo "Executing compute_no_print: Just to check computation time" >> ../out/compute_no_print.out
echo '----------------------------------------------------------' >> ../out/compute_no_print.out
time ./compute_no_print >> ../out/compute_no_print.out

echo
echo "Executing compute_and_print with optimizations"
echo '----------------------------------------------'
echo "Executing compute_and_print with optimizations" >> ../out/compute_and_print.out
echo '----------------------------------------------' >> ../out/compute_and_print.out
time ./compute_and_print >> ../out/compute_and_print.out

echo
echo "Executing compute_and_print without optimizations"
echo '-------------------------------------------------'
echo "Executing compute_and_print without optimizations" >> ../out/compute_and_print_no_opt.out
echo '-------------------------------------------------' >> ../out/compute_and_print_no_opt.out
time ./compute_and_print_no_opt >> ../out/compute_and_print_no_opt.out

done;

cd ../
echo
echo "Done. Check the files in ./out"
echo
