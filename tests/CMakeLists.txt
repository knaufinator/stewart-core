cmake_minimum_required(VERSION 3.20)
project(stewart-core-tests LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)

# Build the shared core straight from this repo's src/ (host/SIL build —
# ESP_PLATFORM is undefined, so the NVS functions compile to the stubs).
add_library(stewart_core STATIC
    ${CMAKE_SOURCE_DIR}/../src/InverseKinematics.cpp
    ${CMAKE_SOURCE_DIR}/../src/AxisScaling.cpp
    ${CMAKE_SOURCE_DIR}/../src/MotionCueing.cpp
)
target_include_directories(stewart_core PUBLIC ${CMAKE_SOURCE_DIR}/../include)
target_compile_definitions(stewart_core PRIVATE _USE_MATH_DEFINES)

add_executable(stewart-core-tests test_motion_cueing_output.cpp)
target_link_libraries(stewart-core-tests PRIVATE stewart_core)
target_compile_definitions(stewart-core-tests PRIVATE _USE_MATH_DEFINES)
if(NOT WIN32)
    target_link_libraries(stewart-core-tests PRIVATE m)
endif()

enable_testing()
add_test(NAME output_stage_tests COMMAND stewart-core-tests)
