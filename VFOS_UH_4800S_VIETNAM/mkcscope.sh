#!/bin/bash

rm -rf cscope.files cscope.out cscope.in.out cscope.po.out

echo "make cscope database....(cscope.files)"
echo "make cscope database....(cscope.file cscope.files)"

find . \( -name '*.c' -o -name '*.cpp' -o -name '*.cc' -o -name '*.h' -o -name '*.s' -o -name '*.S' \) -print > cscope.files
cscope -b -q -k -i cscope.files
