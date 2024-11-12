#!/bin/bash

executing_directory=$(dirname "$(realpath "$0")")
cd "$executing_directory" || exit

git config --local --unset-all safe.directory "$executing_directory"
git config --local --add safe.directory "$executing_directory"
git submodule update --init

cd "$executing_directory/third-party/boost-archive-for-cpprestsdk" || exit
git config --local core.eol lf
git config --local core.autocrlf false
git rm --cached -r . && git reset --hard

cd "$executing_directory/third-party/openssl" || exit
git config --local core.eol lf
git config --local core.autocrlf false
git rm --cached -r . && git reset --hard
