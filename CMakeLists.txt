cmake_minimum_required(VERSION 2.8)

project(oceanOfCode)

enable_testing()

set(HEADERS "baseTypes.h" "TurnAction.h" "Detection.h" "PathFinder.h")
add_definitions(-DLOCAL_DEBUG)
add_executable(${PROJECT_NAME} "main.cpp" ${HEADERS})
