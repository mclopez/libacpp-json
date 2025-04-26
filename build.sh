#!/bin/bash

#./conan_helper.sh
conan install . --output-folder=build/linux --build=missing -s build_type=Release


cmake -S . -B ./build/linux -DCMAKE_BUILD_TYPE=Release

cmake --build ./build/linux  --config Release -v