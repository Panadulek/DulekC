# Create variables for paths
set(PROJECT_DULEKC_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(INPUT_SRC "${PROJECT_DULEKC_DIR}/src")
set(INPUT_LIB_SRC "${PROJECT_DULEK_DIR}../lib/debug")
# Create variables for bison/flex src
set(BISON_INPUT_FILE ${INPUT_SRC}/parser.ypp)
set(FLEX_INPUT_FILE ${INPUT_SRC}/lexer.l)

include_directories(
    E:\\tools\\llvm\\build\\include
    E:\\GnuWin32\\include
    E:\\tools\\llvm\\llvm\\include
    ${CMAKE_SOURCE_DIR}/DulekC/include
)

file(GLOB BISON_SOURCES "${INPUT_SRC}/*.ypp")
file(GLOB FLEX_SOURCES "${INPUT_SRC}/*.l")
file(GLOB SOURCES "${INPUT_SRC}/*.cpp" "${INPUT_SRC}/*.c")
file(GLOB HEADERS_SRC_H "${INPUT_SRC}/*.h")
file(GLOB HEADERS_SRC_HPP "${INPUT_SRC}/*.hpp")



file(GLOB LLVM_LIBS "${INPUT_LIB_SRC}/*.lib")
message("LIB SRC: ${INPUT_LIB_SRC}")
add_executable(DulekC ${SOURCES} ${HEADERS_SRC_H} ${HEADERS_SRC_HPP} ${BISON_SOURCES} ${FLEX_SOURCES})
target_link_libraries(DulekC PRIVATE ${LLVM_LIBS} ws2_32.lib)

find_package(BISON)
find_package(FLEX)
if(BISON_FOUND AND FLEX_FOUND)   
    set(BISON_OUTPUT ${INPUT_SRC}/parser.cpp)
    set(BISON_HEADER_OUTPUT ${INPUT_SRC}/parser.hpp)
    set(FLEX_OUTPUT_FILE ${INPUT_SRC}/lexer.cpp)
    add_custom_command(   
        TARGET DulekC
        COMMAND ${FLEX_EXECUTABLE}  -o${FLEX_OUTPUT_FILE} ${FLEX_INPUT_FILE}
        COMMAND ${BISON_EXECUTABLE} -o ${BISON_OUTPUT} --defines=${BISON_HEADER_OUTPUT} ${BISON_INPUT_FILE}
        COMMAND ${CMAKE_COMMAND} -E echo "Running custom command for Bison and Flex..."
        DEPENDS ${BISON_INPUT_FILE} ${FLEX_INPUT_FILE}
	PRE_BUILD
        BYPRODUCTS ${BISON_OUTPUT} ${BISON_HEADER_OUTPUT} ${FLEX_OUTPUT_FILE}
    )
endif()
