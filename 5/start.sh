#!/bin/bash

echo "=========================================="
echo "start test"
echo "=========================================="

if [[ ! -f "kbest_measure.sh" ]]; then
    echo "error: cannot find kbest_measure.sh in current directory"
    exit 1
fi

if [[ ! -x "kbest_measure.sh" ]]; then
    chmod +x kbest_measure.sh
fi

programs=("./1" "./2" "./3l" "./3" "./3lc" "./3c" "./2a" "./3ca")

declare -A results

echo ""
echo "compiling..."
echo "------------------------------------------"
make
if [[ $? -ne 0 ]]; then
    echo "compile error: please check Makefile"
    exit 1
fi
echo "compile complete"

echo ""
echo "checking file..."
echo "------------------------------------------"
missing_programs=()
for prog in "${programs[@]}"; do
    if [[ ! -x "$prog" ]]; then
        missing_programs+=("$prog")
    fi
done

if [[ ${#missing_programs[@]} -gt 0 ]]; then
    echo "these cannot be executed:"
    for prog in "${missing_programs[@]}"; do
        echo "  $prog"
    done
    echo "please check compling"
    exit 1
fi
echo "all prepared"

echo ""
echo "start measuring..."
echo "=========================================="

for prog in "${programs[@]}"; do
    echo ""
    echo "testing: $prog"
    echo "------------------------------------------"

    ./kbest_measure.sh -- "$prog"
    avg_time=$?
    
    if [[ -z "$avg_time" ]] || ! [[ "$avg_time" =~ ^[0-9.]+$ ]]; then
        echo "warning: cannot get average time of $prog , using N/A"
        avg_time="N/A"
    fi
    
    results["$prog"]="$avg_time"
    
    echo "done: $prog"
    echo "------------------------------------------"
done

echo ""
echo "=========================================="
echo "result"
echo "=========================================="
printf "%-10s %-12s\n" "program" "average time(ms)"
printf "%-10s %-12s\n" "------" "----------"

for prog in "${programs[@]}"; do
    printf "%-10s %-12s\n" \
        "$prog" \
        "${results[$prog]}"
done

make clean

echo ""
timestamp=$(date +%Y%m%d_%H%M%S)
csv_file="performance_results_${timestamp}.csv"

{
    echo "program,average time(ms)"
    for prog in "${programs[@]}"; do
        echo "$prog,${results[$prog]}"
    done
} > "$csv_file"

echo "result is in: $csv_file"

echo ""
echo "=========================================="
echo "all done"
echo "=========================================="
