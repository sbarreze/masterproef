cmake_minimum_required(VERSION 3.23)
project(masterrepo2)

set(CMAKE_CXX_STANDARD 17)

# Find the fmt library
find_package(fmt REQUIRED)
find_package(spdlog REQUIRED)

set(PROJECT_HEADERS
        src/util/common.h
        src/util/TSPLIB95_parser.h
        src/util/Route.h
        src/bee_colony/bee_colony.h
        src/GELSGA/gelsga.h
        src/PSA/PSA.h
        src/Clarck-Wright/ClassicCW.h
        src/Clarck-Wright/ImprovedCW.h
        src/Experiments/GELGA_not_random_init.h
)

set(PROJECT_SOURCES
        main.cpp
        src/util/common.cpp
        src/util/TSPLIB95_parser.cpp
        src/util/Route.cpp
        src/bee_colony/bee_colony.cpp
        src/GELSGA/gelsga.cpp
        src/PSA/PSA.cpp
        src/Clarck-Wright/ClassicCW.cpp
        src/Clarck-Wright/ImprovedCW.cpp
        src/Experiments/GELGA_not_random_init.cpp
)

add_executable(masterproef ${PROJECT_SOURCES} ${PROJECT_HEADERS})

# set optimization to maximum
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")

# Link against the fmt library
target_link_libraries(masterproef PRIVATE fmt::fmt)
target_link_libraries(masterproef PRIVATE spdlog::spdlog)
