#!/bin/bash

get_build_version() {
    local pattern='^### ([0-9]+\.[0-9]+\.[0-9]+)'
    local version=''

    while IFS= read -r line; do
        if [[ $line =~ $pattern ]]; then
            version="${BASH_REMATCH[1]}"
            break
        fi
    done < "$1"

    if [[ -z "${version}" ]]; then
        exit 1
    fi

    echo "$version"
}

version=$(get_build_version "README.md")

if [ $? -ne 0 ]; then
    echo "Invalid Version."
    exit 2
fi

echo "Detected Version: $version"

build_preset() {
    echo ""
}
