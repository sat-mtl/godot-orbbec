#! /usr/bin/bash

# This works on arch linux. it does not work on gcc

if [[ -z "${BUILD_RELEASE}" ]]; then
    # if BUILD_RELEASE has no value, build a debug version
    cmake -B build -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DCMAKE_CXX_COMPILER="clang++" -DCMAKE_C_COMPILER="clang"
else
    # else, build a release version
    cmake -B build -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DCMAKE_CXX_COMPILER="clang++" -DCMAKE_C_COMPILER="clang" -DGODOTCPP_TARGET="template_release" -DCMAKE_BUILD_TYPE=Release
fi
make -sC build -j8

cp build/lib/godot-orbbec.linux.template* ./bin/linux/
