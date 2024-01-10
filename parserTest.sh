#!/bin/bash
rm -f -- a.out
gcc driver.c parser.c vm.o lex.o
if [ $? -ne 0 ]; then
    echo "Compile failed"
    exit 1
fi
EXE="./a.out"
echo "Compile Succeeded"

echo "Case 1: example.txt"
echo "1" | eval $EXE example.txt -c -s > user_example_output.txt
diff -w -B example_out.txt user_example_output.txt

echo "Case 2: arithmetic.txt"
eval $EXE arithmetic.txt -c -s > user_arithmetic_output.txt
diff -w -B arithmetic_out.txt user_arithmetic_output.txt

echo "Case 3: mult.txt"
echo "7 2" | eval $EXE mult.txt -c -s > user_mult_output.txt
diff -w -B mult_out.txt user_mult_output.txt

echo "Case 4: velocity.txt"
echo "4 6 3" | eval $EXE velocity.txt -c -s > user_velocity_output.txt
diff -w -B velocity_out.txt user_velocity_output.txt

echo "Case 5: errors.txt"
eval $EXE errors.txt -c -s > user_error_output.txt
diff -w -B errors_out.txt user_error_output.txt

echo "Case 6: fizzbuzz.txt"
echo "1" | eval $EXE fizzbuzz.txt -c -s > user_fizzbuzz_output.txt
diff -w -B fizzbuzz_out.txt user_fizzbuzz_output.txt

echo "Parser Testing Completed"

