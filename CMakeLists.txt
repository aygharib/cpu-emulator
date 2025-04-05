cmake_minimum_required(VERSION 3.15...3.31)

# Set CMake's generation step to generate a compile_comands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set CMake's generation step to generate files for a build release
# Can be overwritten in command-line via -DCMAKE_BUILD_TYPE=Release
set(default_build_type "Debug")

# Set the global C++ standard for all targets
# Can be overwritten on a target-basis in `target_compile_features`
# Set the standard to C++23
SET(CMAKE_CXX_STANDARD 23)
# Force CMake to abide by the globally set standard on the previous line
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

project(MyProject VERSION 1.0
                  DESCRIPTION "MyProject"
                  LANGUAGES CXX)

add_executable(my_executable src/main.cpp src/CHIP8.h src/CHIP8.cpp)

## CMake 3.8+: Meta compiler features
# Specifies compiler features required when compiling a given target
# If the feature is not listed in the CMAKE_C_COMPILE_FEATURES, CMAKE_CUDA_COMPILE_FEATURES, or CMAKE_CXX_COMPILE_FEATURES variables, then an error will be reported by CMake.
# If the use of the feature requires an additional compiler flag, such as -std=gnu++11, the flag will be added automatically.
# Here, we are setting the C++ standard for a specific target
target_compile_features(my_executable PUBLIC cxx_std_23)

# Specify that we don't want compiler-specific extension
# For some compilers, this results in adding a flag such as -std=gnu++11 instead of -std=c++11 to the compile line
# By default, this is turned on
set_target_properties(my_executable PROPERTIES CXX_EXTENSIONS OFF)


#set(default_build_type "Release")

# cmake_minimum_required(VERSION 3.10)
# project(CHIP8_EMULATOR)
#
# SET(CMAKE_CXX_STANDARD 20)
# SET(CMAKE_CXX_STANDARD_REQUIRED ON)
#
# add_definitions("-std=c++20")
#
# include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
#
# conan_basic_setup()
#
# file(GLOB SOURCES src/*.cpp)
#
# add_executable(${PROJECT_NAME} ${SOURCES})
#
# target_link_libraries(${PROJECT_NAME} ${CONAN_LIBS})
