# same cmake min required version for QuEST v4.2.0
cmake_minimum_required(VERSION 3.21)

project(quest_lab LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# update QuEST version and/or repo as required 
set(QUEST_REPO "https://github.com/QuEST-Kit/QuEST.git" CACHE STRING "QuEST remote")
set(QUEST_TAG  "v4.2.0" CACHE STRING "QuEST tag/branch/commit")

# config QuEST build options as required
set(ENABLE_DISTRIBUTION ON CACHE BOOL "" FORCE)

include(FetchContent)
FetchContent_Declare(QuEST
    GIT_REPOSITORY ${QUEST_REPO}
    GIT_TAG        ${QUEST_TAG}
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(QuEST)

# set root experiments dir
set(EXPERIMENTS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/experiments)

# compile all circuit libraries available
file(GLOB CIRCUIT_SOURCES CONFIGURE_DEPENDS ${EXPERIMENTS_DIR}/circuits/*.c)
if(CIRCUIT_SOURCES)
    add_library(circuits STATIC ${CIRCUIT_SOURCES})
    target_include_directories(circuits PUBLIC ${EXPERIMENTS_DIR}/circuits)
    target_link_libraries(circuits PUBLIC QuEST::QuEST)
    target_compile_options(circuits PRIVATE -Wall -Wextra)
endif()


# compiles all benchmark executables available 
file(GLOB BENCHMARK_SOURCES CONFIGURE_DEPENDS ${EXPERIMENTS_DIR}/benchmarks/*.c)
foreach(src ${BENCHMARK_SOURCES})
    get_filename_component(name ${src} NAME_WE)
    add_executable(${name} ${src})
    target_link_libraries(${name} PRIVATE QuEST::QuEST)
    target_compile_options(${name} PRIVATE -Wall -Wextra)
    if(TARGET circuits)
        target_link_libraries(${name} PRIVATE circuits)
    endif()
endforeach()