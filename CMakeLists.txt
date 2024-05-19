cmake_minimum_required(VERSION 3.10)

# set the project name
project(TrafficBlaster)

# specify the C++ standard
set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Display current build type (optional)
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# add .cpp files to the executable
file(GLOB cpp_files "src/*.cpp")
add_executable(traffic_blaster ${cpp_files})

# Find Boost libraries
set(Boost_USE_STATIC_LIBS ON)  # Use static Boost libraries
find_package(Boost REQUIRED COMPONENTS asio program_options)

if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    target_link_libraries(traffic_blaster PRIVATE Boost::asio Boost::program_options pthread)
endif()

# Set linker flags for partially static linking
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++ -lpthread")
