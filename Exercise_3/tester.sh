#!/bin/bash

# Exit on any error
set -e

# Remove only if they exist
[ -d logs ] && rm -r logs
[ -d test2/user ] && rm -r test2/user
[ -f test1/output.txt ] && rm test1/output.txt

# Create necessary directories
mkdir -p logs
mkdir -p test2/user


# Build
echo "Compiling..."
if ! gcc -o ex3 ex3.c 2> logs/compile.log; then
    echo "Compilation failed. See logs/compile.log"
    exit 1
fi

# === Part 1: Focus-Mode Test ===
echo "Running Focus-Mode test..."
./ex3 Focus-Mode 3 4 < test1/input.txt > test1/output.txt 2> logs/test1

if diff -q test1/output.txt test1/expected_output.txt > logs/test1; then
    echo -e "✅ \e[32mFocus-Mode test PASSED\e[0m"
    part1_passed=true
else
    echo -e "❌ \e[31mFocus-Mode test FAILED\e[0m — Check logs/test1"
    part1_passed=false
fi


# === Part 2: CPU-Scheduler Tests ===
echo "Running CPU-Scheduler tests..."
declare -a time_quantums=(2 20 3 1 5)
all_passed=true

for i in {1..5}; do
    input_file="test2/input/processes${i}.csv"
    expected_output="test2/expected_output/output${i}.txt"
    user_output="test2/user/user_output${i}.txt"
    log_file="logs/test2.${i}"
    tq=${time_quantums[$((i-1))]}

    echo "  Test $i with TQ=$tq..."
   if ./ex3 CPU-Scheduler "$input_file" "$tq" > "$user_output" 2> "$log_file"; then
        if diff -q "$user_output" "$expected_output" > "$log_file"; then
            echo -e "  ✅ \e[32mTest $i PASSED\e[0m"
        else
            echo -e "  ❌ \e[31mTest $i FAILED\e[0m — Output mismatch. See $log_file"
            all_passed=false
        fi
    else
        echo -e "  ❌ \e[31mTest $i FAILED\e[0m — Runtime error. See $log_file"
        all_passed=false
    fi

done

# === Cleanup if all passed ===
if $part1_passed && $all_passed; then
    echo -e "\n🎉 \e[32mALL TESTS PASSED!\e[0m Cleaning up...\n"
    rm -r logs
    rm -r test2/user
    rm test1/output.txt

    echo " (\(\ "
    echo "( -.-)"
    echo " o_(\")(\")"
    echo -e "🐰 \e[32mHoppy days! All tests passed!\e[0m"

else
    echo -e "\n💥 \e[31mSome tests FAILED\e[0m. Check the logs above."
    exit 1
fi
