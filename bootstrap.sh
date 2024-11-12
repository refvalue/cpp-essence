#!/bin/bash

executing_directory=$(dirname "$(realpath "$0")")
cd "$executing_directory"

git submodule update --init --recursive

cd "$executing_directory/third-party/boost-archive-for-cpprestsdk"
git config --local core.eol lf
git config --local core.autocrlf false
git rm --cached -r . && git reset --hard

cd "$executing_directory/third-party/openssl"
git config --local core.eol lf
git config --local core.autocrlf false
git rm --cached -r . && git reset --hard
