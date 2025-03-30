cmake_minimum_required(VERSION 3.10)
project(coop-3000 VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Option to control mocking
option(USE_MOCK "Use mock for GPIO control" ON)

# Define the MOCK flag based on the USE_MOCK option
if(USE_MOCK)
    add_definitions(-DMOCK)
else()
    find_library(WIRINGPI_LIBRARIES NAMES wiringPi)
    if(NOT WIRINGPI_LIBRARIES)
        message(FATAL_ERROR "wiringPi library not found")
    endif()
endif()

# Find Boost
find_package(Boost REQUIRED COMPONENTS program_options)
if(NOT Boost_FOUND)
    message(FATAL_ERROR "Boost library not found")
endif()

include_directories(include ${Boost_INCLUDE_DIRS})

# Add source files
add_executable(coop-3000
    src/main.cpp
    src/automation.cpp
    src/devices.cpp
    src/timing.cpp
    src/monitoring.cpp
)

add_executable(time
    src/time.cpp
    src/timing.cpp
)

# Link Boost libraries
target_link_libraries(coop-3000 Boost::program_options)

if(NOT USE_MOCK)
    target_link_libraries(coop-3000 ${WIRINGPI_LIBRARIES})
endif()

# Install the executable
install(TARGETS coop-3000
        RUNTIME DESTINATION bin)

# Install configuration files
install(FILES config/config.cfg
        DESTINATION etc/coop-3000)