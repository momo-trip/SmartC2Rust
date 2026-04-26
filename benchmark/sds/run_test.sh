
#!/bin/bash

# Reformed test cases

failed=0

mkdir -p flow_results

test_cases=(
    "create_and_length"
    "create_with_specified_length"
    "string_concatenation"
    "sdscpy_against_longer_string"
    "sdscpy_against_shorter_string"
    "sdscatprintf_base_case"
    "sdscatprintf_with_null_char"
    "sdscatprintf_large_string"
    "sdscatfmt_base_case"
    "sdscatfmt_unsigned_numbers"
    "sdstrim_all_chars_match"
    "sdstrim_single_char_remains"
    "sdstrim_correctly_trims"
    "sdsrange_single_char"
    "sdsrange_from_start"
    "sdsrange_from_end"
    "sdsrange_empty_result"
    "sdsrange_beyond_length"
    "sdsrange_out_of_bounds"
    "sdscmp_greater"
    "sdscmp_equal"
    "sdscmp_less"
    "sdscatrepr"
    "sdsmakeroomfor"
)

total_tests=${#test_cases[@]}

for i in $(seq 1 $total_tests); do
    test_name="${test_cases[$((i-1))]}"
    binary="./test_sds_t${i}"

    echo "Test ${i} started"

    tmp_log="flow_results/test${i}_tmp.log"
    LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${i}_trace.log "$binary" "$test_name" > "$tmp_log" 2>&1
    exit_code=$?

    if [ $exit_code -eq 0 ]; then
        echo "Test ${i}: $test_name - PASSED"
        mv "$tmp_log" "flow_results/test${i}_success.log"
        echo "Test ${i} passed"
    else
        echo "Test ${i}: $test_name - FAILED" >&2
        mv "$tmp_log" "flow_results/test${i}_fail.log"
        echo "Test ${i} failed" >&2
        failed=1
    fi

    echo "Test ${i} ended"
done

exit $failed

