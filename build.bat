
conan install . --output-folder=build\win --build=missing -s build_type=Release

mkdir .\build\win
cmake -S . -B .\build\win -DCMAKE_BUILD_TYPE=Release -G "Visual Studio 17 2022"

cmake --build .\build\win  --config Release -v