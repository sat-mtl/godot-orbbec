#! /usr/bin/bash

# This works on arch linux. it does not work on gcc

cmake -B build -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DCMAKE_CXX_COMPILER="clang++" -DCMAKE_C_COMPILER="clang"
make -sC build -j8

cp build/lib/godot-orbbec.linux.template* ./bin/linux/
echo "aaaaaaaaaa"
