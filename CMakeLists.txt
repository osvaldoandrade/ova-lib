cmake_minimum_required(VERSION 3.25)
project(ova_lib C)
set(CMAKE_C_STANDARD 11)
set(LIB_VERSION 0.0.1)

enable_testing()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_library(ova_lib_static STATIC
        src/queue/queue.c
        src/queue/heap_queue.c
        src/queue/linked_queue.c
        src/list/list.c
        src/list/array_list.c
        src/list/linked_list.c
        src/map/hash_map.c
        src/map/map.c
        src/heap/binary_heap.c
        src/heap/heap.c
        src/heap/fibonacci_heap.c
        src/sort/sort.c
        src/stack/linked_stack.c
        src/stack/stack.c
        src/stack/array_stack.c
        src/matrix/matrix.c
        src/solver/solver.c

        src/solver/simplex.c
)
set_target_properties(ova_lib_static PROPERTIES OUTPUT_NAME "ova_lib")

add_library(ova_lib_shared SHARED
        src/queue/queue.c
        src/queue/heap_queue.c
        src/queue/linked_queue.c
        src/list/list.c
        src/list/array_list.c
        src/list/linked_list.c
        src/map/hash_map.c
        src/map/map.c
        src/heap/binary_heap.c
        src/heap/heap.c
        src/heap/fibonacci_heap.c
        src/sort/sort.c
        src/stack/linked_stack.c
        src/stack/stack.c
        src/stack/array_stack.c
        src/matrix/matrix.c
        src/solver/solver.c

        src/solver/simplex.c
)
set_target_properties(ova_lib_shared PROPERTIES OUTPUT_NAME "ova_lib")

target_include_directories(ova_lib_static PUBLIC include)

target_include_directories(ova_lib_shared PUBLIC include)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/include/ DESTINATION ${CMAKE_BINARY_DIR}/include/)

file(GLOB_RECURSE HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")

install(FILES ${HEADERS} DESTINATION include)

install(TARGETS ova_lib_static ova_lib_shared
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
)

file(WRITE ${CMAKE_BINARY_DIR}/install.sh
        "#!/bin/bash\n"
        "echo 'Installing ova_lib...'\n"
        "mkdir -p /usr/local/lib\n"
        "mkdir -p /usr/local/include/ova_lib\n"
        "cp -r ${CMAKE_BINARY_DIR}/lib/* /usr/local/lib/\n"
        "cp -r ${CMAKE_BINARY_DIR}/include/* /usr/local/include/ova_lib\n"
        "echo 'Installation complete!'\n"
)
install(PROGRAMS ${CMAKE_BINARY_DIR}/install.sh DESTINATION .)

foreach(TEST IN ITEMS test_queue test_priority_queue test_binary_heap test_fibonacci_heap test_hash test_array_list test_linked_list test_sorter test_linked_stack test_array_stack test_matrix test_solver)
    add_executable(${TEST} test/${TEST}.c test/base_test.c)
    target_link_libraries(${TEST} ova_lib_static m)
    add_test(NAME ${TEST} COMMAND ${TEST})
endforeach()

set(CPACK_PACKAGE_NAME "ova_lib")
set(CPACK_PACKAGE_VENDOR "Osvaldo Andrade")
set(CPACK_PACKAGE_VERSION ${LIB_VERSION})
set(CPACK_PACKAGE_CONTACT "osvaldo.andrade@gmail.com")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A compact toolkit offering data structures in ANSI C, designed for simplicity and portability across POSIX platforms.")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")

set(CPACK_GENERATOR "TGZ")

include(CPack)
