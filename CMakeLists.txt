cmake_minimum_required(VERSION 3.14)
project(vrbsp)

set(CMAKE_CXX_STANDARD 11)
# add_executable(vrbsp code_c++.cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m64 -std=c++11 -g -D_GLIBCXX_USE_CXX11_ABI=0")

# declara header gurobi directory
if ("${CMAKE_SYSTEM}" MATCHES "Linux")
  set(GUROBI_INCLUDE_DIRECTORY "~/Downloads/gurobi900/linux64/include")
elseif (APPLE)
  set(GUROBI_INCLUDE_DIRECTORY "/Library/gurobi811/mac64/include")
endif ()

# adicionar header gurobi
include_directories(${GUROBI_INCLUDE_DIRECTORY})

# declara library gurobi directory
if ("${CMAKE_SYSTEM}" MATCHES "Linux")
  set(GUROBI_LIB_DIRECTORY "~/Downloads/gurobi900/linux64/lib")
elseif (APPLE)
  set(GUROBI_LIB_DIRECTORY "/Library/gurobi811/mac64/lib/")
endif ()

# adicionar library gurobi
link_directories(${GUROBI_LIB_DIRECTORY})

# criar executavel
set(SOURCE_FILES ${GUROBI_INCLUDE_DIRECTORY}/gurobi_c++.h Model.h Model.cpp Structures.h main.cpp)
add_executable(vrbsp ${SOURCE_FILES})

# adicionar libraries
target_link_libraries(vrbsp gurobi_c++)
target_link_libraries(vrbsp gurobi90)