#!/bin/bash

output=""

generate_overload() {
    for (( i=$1; i>=0; i-- )); do
        output="${output}ES_MAKE_DATA_MEMBER_BINDINGS($i, "

        for (( j=1; j<i; j++ )); do
            output="${output}m$j,"
        done

        output="${output}m$j);\n\n"
    done
}

generate_overload 100

echo -e "$output"
