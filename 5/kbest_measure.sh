#!/bin/bash

K=10
EPSILON=0.02
M=200

MEASURE_COMMAND=()
PARSE_COMMAND=0

while [[ $# -gt 0 ]]; do
    case $1 in
        -k|--k-best)
            K="$2"
            shift 2
            ;;
        -e|--epsilon)
            EPSILON="$2"
            shift 2
            ;;
        -m|--max-measurements)
            M="$2"
            shift 2
            ;;
        --)
            PARSE_COMMAND=1
            shift
            ;;
        *)
            if [[ $PARSE_COMMAND -eq 1 ]]; then
                MEASURE_COMMAND+=("$1")
            else
                echo "unknown option: $1"
                echo "usage: $0 [OPTIONS] -- <command>"
                echo "OPTIONS:"
                echo "  -k, --k-best K          set K (默认: 10)"
                echo "  -e, --epsilon EPSILON   set EPSILON (默认: 0.02)"
                echo "  -m, --max-measurements M set M (默认: 200)"
                exit 1
            fi
            shift
            ;;
    esac
done

if [[ ${#MEASURE_COMMAND[@]} -eq 0 ]]; then
    echo "error: command not found"
    echo "usage: $0 [OPTIONS] -- <command>"
    exit 1
fi

echo "K-Best measuring start"
echo "parameters: K=$K, ε=$EPSILON, M=$M"
echo "command: ${MEASURE_COMMAND[*]}"
echo "----------------------------------------"

get_time() {
    date +%s.%N
}

measure_once() {
    local start_time end_time duration
    start_time=$(get_time)
    "${MEASURE_COMMAND[@]}" > /dev/null 2>&1
    end_time=$(get_time)
    duration=$(echo "$end_time - $start_time" | bc -l)
    echo "$duration"
}

declare -a KTEST

check_convergence() {
    local i
    local vk
    
    if [[ ${#KTEST[@]} -lt $K ]]; then
        return 1
    fi
    
    vk="${KTEST[$((K-1))]}"
    
    for ((i=0; i<K; i++)); do
        local vi="${KTEST[$i]}"
        local threshold=$(echo "scale=10; (1 + $EPSILON) * $vi" | bc -l)
        
        if (( $(echo "$threshold < $vk" | bc -l) )); then
            return 1
        fi
    done
    
    return 0
}

insert_into_ktest() {
    local new_value="$1"
    
    if [[ ${#KTEST[@]} -lt $K ]]; then
        KTEST+=("$new_value")
        IFS=$'\n' KTEST=($(sort -n <<< "${KTEST[*]}"))
        unset IFS
        return
    fi
    
    local slowest="${KTEST[$((K-1))]}"
    if (( $(echo "$new_value < $slowest" | bc -l) )); then
        KTEST[$((K-1))]="$new_value"
        IFS=$'\n' KTEST=($(sort -n <<< "${KTEST[*]}"))
        unset IFS
    fi
}

show_ktest() {
    printf "KTEST: [" ${#KTEST[@]}
    for ((i=0; i<${#KTEST[@]}; i++)); do
        printf "%.6f" "${KTEST[$i]}"
        if [[ $i -lt $((${#KTEST[@]}-1)) ]]; then
            printf ", "
        fi
    done
    printf "]\n"
}

converged=0
measurement_count=0

echo "start measuring..."
while [[ $measurement_count -lt $M ]] && [[ $converged -eq 0 ]]; do
    duration=$(measure_once)
    measurement_count=$((measurement_count + 1))
    insert_into_ktest "$duration"
    
    if [[ $((measurement_count % $K)) -eq 0 ]]; then
        echo "progress: $measurement_count/$M"
        show_ktest
    fi
    
    if check_convergence; then
        converged=1
        echo "----------------------------------------"
        echo "end at $measurement_count/$M"
    fi
done

echo "----------------------------------------"

if [[ $converged -eq 0 ]]; then
    echo "warning: divergent"
fi

if [[ ${#KTEST[@]} -eq 0 ]]; then
    echo "error: KTEST is empty"
    exit 1
fi

sum=0
for value in "${KTEST[@]}"; do
    sum=$(echo "$sum + $value" | bc -l)
done

average=$(echo "scale=6; $sum / ${#KTEST[@]}" | bc -l)

echo "========================================"
echo "command: ${MEASURE_COMMAND[*]}"
echo "measurement count: $measurement_count"
echo "is convergent: $(if [[ $converged -eq 1 ]]; then echo "yes"; else echo "no"; fi)"

echo "----------------------------------------"
echo "average time: $(printf "%.6f" "$average")"
echo "========================================"

ms_average=$(echo "scale=0; $average * 1000 / 1" | bc)
exit $ms_average