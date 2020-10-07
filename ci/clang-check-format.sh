#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

cd $DIR/../

# DIR dos2unix to clean-up line endings
echo "Applying dos2unix"
find . -iname *.hpp -o -iname *.cpp -o -iname *.tpp -o -iname *.h | grep -vi config.h | grep -vi thirdparty | xargs dos2unix

# Apply clang-format
echo "Applying clang-format"
find . -iname *.hpp -o -iname *.cpp -o -iname *.tpp -o -iname *.h | grep -vi config.h | grep -vi thirdparty | xargs clang-format -i --verbose style=Google
