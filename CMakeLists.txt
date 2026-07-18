# stewart-core — shared Stewart-platform math (IK, axis scaling, motion cueing).
# Single source of truth, consumed as a git submodule by:
#   - 6DOF-Rotary-Stewart-Motion-Simulator (desktop app, plain CMake)
#   - Mini-6DOF (ESP32 firmware, ESP-IDF)
cmake_minimum_required(VERSION 3.16)
set(STEWART_CORE_SRCS src/InverseKinematics.cpp src/AxisScaling.cpp src/MotionCueing.cpp)
if(ESP_PLATFORM)
    idf_component_register(SRCS ${STEWART_CORE_SRCS} INCLUDE_DIRS include REQUIRES nvs_flash)
else()
    project(stewart_core LANGUAGES C CXX)
    add_library(stewart_core STATIC ${STEWART_CORE_SRCS})
    target_include_directories(stewart_core PUBLIC include)
endif()
