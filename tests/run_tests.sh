make clean all install
if [ $? != 0 ]; then
    echo
    echo "ERROR occured during make. ABORTING..."
    echo
    exit
fi
cd bin/
echo 
echo "---Start Tests---"
test_count=0
pass_count=0
fail_count=0
for exe in *; do
    test_count=$((test_count + 1))
    echo
    echo "Test $test_count : $exe"
    ./$exe 2>&1>../out/$exe".out"
    cut -d- -f2- ../out/$exe".log" > ../out/"cut_"$exe".log"
    cut -d- -f2- ../out/$exe".out" > ../out/"cut_"$exe".out"
    diff ../correct_out/$exe".log" ../out/"cut_"$exe".log" && diff ../correct_out/$exe".out" ../out/"cut_"$exe".out"
    if [ $? == 0 ]; then
        echo "          PASS"
        pass_count=$((pass_count + 1))
    else
        echo "          FAIL"
        fail_count=$((fail_count + 1))
    fi
done
echo
echo "---Tests Completed---"
echo "PASS: $pass_count"
echo "FAIL: $fail_count"
echo
