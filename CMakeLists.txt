cmake_minimum_required(VERSION 3.22)
project(MyProject VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Ustaw katalog wyjściowy dla plików wykonywalnych
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Dodaj podkatalogi (projekty)
message("DULEKC MAKE")
add_subdirectory(DulekC)
message("DUFUNCTION MAKE")
add_subdirectory(DuFunctions)
