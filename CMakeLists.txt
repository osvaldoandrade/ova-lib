cmake_minimum_required(VERSION 3.10)
project(ova_lib C)
set(CMAKE_C_STANDARD 11)
set(LIB_VERSION 0.0.1)

if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(
        -Wall -Wextra -Wpedantic
        -Wcast-align -Wcast-qual
        -Wconversion -Wsign-conversion
        -Wdouble-promotion
        -Wformat=2 -Wformat-security
        -Wnull-dereference
        -Wshadow
        -Wstrict-overflow=2
        -Wswitch-default -Wswitch-enum
        -Wundef
        -Wuninitialized
        -Wwrite-strings
        -Werror
    )
endif()

find_program(VALGRIND_PROGRAM valgrind)
if(VALGRIND_PROGRAM)
    # Configure CTest memcheck to use Valgrind when available.
    set(MEMORYCHECK_COMMAND "${VALGRIND_PROGRAM}")
    set(MEMORYCHECK_COMMAND_OPTIONS "--leak-check=full --show-leak-kinds=all --track-origins=yes --error-exitcode=1")
    set(MEMORYCHECK_TYPE "Valgrind")
endif()

include(CTest)
option(OVA_BUILD_FUZZERS "Build libFuzzer harnesses" OFF)

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
        src/list/sorted_list.c
        src/bloom_filter/bloom_filter.c
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
        src/graph/graph.c
        src/graph/graph_algorithms.c
        src/graph/adjacency_list.c
        src/graph/adjacency_matrix.c
        src/tree/tree.c
        src/tree/avl_tree.c
        src/tree/red_black_tree.c
        src/trie/trie.c
        src/set/set.c
        src/set/hash_set.c
        src/set/tree_set.c
        src/solver/solver.c
        src/solver/simplex.c
        src/deque/deque.c
        src/memory_pool/memory_pool.c
        src/skip_list/skip_list.c
)
target_compile_options(ova_lib_static PRIVATE -fPIC)
set_target_properties(ova_lib_static PROPERTIES OUTPUT_NAME "ova_lib")
target_link_libraries(ova_lib_static PUBLIC m)

add_library(ova_lib_shared SHARED
        src/queue/queue.c
        src/queue/heap_queue.c
        src/queue/linked_queue.c
        src/list/list.c
        src/list/array_list.c
        src/list/linked_list.c
        src/list/sorted_list.c
        src/bloom_filter/bloom_filter.c
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
        src/graph/graph.c
        src/graph/graph_algorithms.c
        src/graph/adjacency_list.c
        src/graph/adjacency_matrix.c
        src/tree/tree.c
        src/tree/avl_tree.c
        src/tree/red_black_tree.c
        src/trie/trie.c
        src/set/set.c
        src/set/hash_set.c
        src/set/tree_set.c
        src/solver/solver.c
        src/solver/simplex.c
        src/deque/deque.c
        src/memory_pool/memory_pool.c
        src/skip_list/skip_list.c
)
set_target_properties(ova_lib_shared PROPERTIES OUTPUT_NAME "ova_lib")
target_link_libraries(ova_lib_shared PUBLIC m)

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

foreach(TEST IN ITEMS test_queue test_priority_queue test_binary_heap test_fibonacci_heap test_hash test_array_list test_linked_list test_sorted_list test_sorter test_linked_stack test_array_stack test_matrix test_matrix_extra test_solver test_graph test_graph_algorithms test_avl_tree test_red_black_tree test_set test_trie test_bloom_filter test_deque test_memory_pool test_skip_list test_user_data)
    add_executable(${TEST} test/${TEST}.c test/base_test.c)
    target_link_libraries(${TEST} ova_lib_static m)
    add_test(NAME ${TEST} COMMAND ${TEST})
endforeach()

if(OVA_BUILD_FUZZERS)
    if(NOT CMAKE_C_COMPILER_ID MATCHES "Clang")
        message(FATAL_ERROR "OVA_BUILD_FUZZERS=ON requires Clang with libFuzzer support (set CMAKE_C_COMPILER=clang).")
    endif()

    add_executable(fuzz_hash_map fuzz/fuzz_hash_map.c)
    add_executable(fuzz_sort fuzz/fuzz_sort.c)
    add_executable(fuzz_matrix fuzz/fuzz_matrix.c)

    set(FUZZ_TARGETS fuzz_hash_map fuzz_sort fuzz_matrix)
    foreach(FUZZ_TARGET IN LISTS FUZZ_TARGETS)
        target_link_libraries(${FUZZ_TARGET} PRIVATE ova_lib_static m)
        target_compile_options(${FUZZ_TARGET} PRIVATE -fsanitize=fuzzer,address)
        target_link_options(${FUZZ_TARGET} PRIVATE -fsanitize=fuzzer,address)
    endforeach()
endif()

if(VALGRIND_PROGRAM)
    add_custom_target(memcheck
            COMMAND ${CMAKE_CTEST_COMMAND}
            --force-new-ctest-process
            --test-action memcheck
            --output-on-failure
            COMMAND sh -c "cat \"${CMAKE_BINARY_DIR}/Testing/Temporary/MemoryChecker.*.log\" || true"
            WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
            USES_TERMINAL
    )
endif()

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

# pkg-config support
include(GNUInstallDirs)
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/ova-lib.pc.in
    ${CMAKE_BINARY_DIR}/ova-lib.pc
    @ONLY
)
install(FILES ${CMAKE_BINARY_DIR}/ova-lib.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
