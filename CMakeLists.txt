cmake_minimum_required(VERSION 3.16)
project(ail)

option(AIL_EXAMPLES "Build examples" TRUE)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/bin")

function(add_driver DNAME FILES)
    add_library(${DNAME} SHARED ${FILES})
    target_link_libraries(${DNAME} PRIVATE ail32)
endfunction()

add_subdirectory(ail32)
add_subdirectory(drivers)

if (AIL_EXAMPLES)
    add_subdirectory(examples)
endif()
