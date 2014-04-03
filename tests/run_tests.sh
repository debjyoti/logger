echo 
echo "------make clean------"
make clean
echo 
echo "-------make $@-------"
make $@
if [ $? != 0 ]; then
    echo "ERROR occured during make $@. ABORTING..."
    exit
fi
echo
echo "-----make install-----"
make install
if [ $? != 0 ]; then
    echo "ERROR occured during make install. ABORTING..."
    exit
fi
echo
echo "------Go to bin ------"
echo "cd bin"
cd bin/
echo 
echo "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"
echo "------------------Start Running Tests------------------"
test_count=0
pass_count=0
fail_count=0
for exe in *; do
    if [ "${exe##*.}" == 'exe' ]; then #rm .exe extension in Cygwin
        mv $exe ${exe%.exe}
        exe=${exe%.exe}
    fi
    test_count=$((test_count + 1))
    echo
    echo "Test $test_count : ./$exe &>../out/$exe.out"
    ./$exe &>../out/$exe".out"
    case $exe in
        'file_change' )
            cut -d- -f2- ../out/$exe".log" > ../out/"cut_"$exe".log"
            cut -d- -f2- ../out/$exe".out" > ../out/"cut_"$exe".out"
            cut -d- -f2- ../out/file_change2.log > ../out/cut_file_change2.log
            diff ../correct_out/file_change.log ../out/cut_file_change.log && diff ../correct_out/file_change.out ../out/cut_file_change.out && diff ../correct_out/file_change2.log ../out/cut_file_change2.log
            ;;
        'no_init' )
            grep "pthread_join: No such process" ../out/no_init.out > /dev/null
            ;;
        * )
            cut -d- -f2- ../out/$exe".log" > ../out/"cut_"$exe".log"
            cut -d- -f2- ../out/$exe".out" > ../out/"cut_"$exe".out"
            diff ../correct_out/$exe".log" ../out/"cut_"$exe".log" && diff ../correct_out/$exe".out" ../out/"cut_"$exe".out"
            ;;
    esac
    if [ $? == 0 ]; then
        echo "**************************"
        echo "          PASS"
        echo "**************************"
        pass_count=$((pass_count + 1))
    else
        echo "**************************"
        echo "         !FAIL!"
        echo "**************************"
        fail_count=$((fail_count + 1))
    fi
done
echo
echo "--------------------Tests Completed--------------------"
echo "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"
echo "PASS: $pass_count"
echo "FAIL: $fail_count"
echo
