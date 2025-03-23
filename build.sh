#!/bin/bash

#./conan_helper.sh
conan install . --output-folder=build --build=missing -s build_type=Release


cmake -S . -B ./build -DCMAKE_BUILD_TYPE=Release

cmake --build ./build  --config Release -v