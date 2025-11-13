cmake_minimum_required(VERSION 4.0)
project(APDproiect1 C)

set(CMAKE_C_STANDARD 11)

add_executable(APDproiect1 main.c)
find_package(OpenMP REQUIRED)
target_link_libraries(APDproiect1 PRIVATE OpenMP::OpenMP_C)