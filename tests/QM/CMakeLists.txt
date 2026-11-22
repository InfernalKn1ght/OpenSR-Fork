add_custom_target(qm_tests COMMENT "Building all QM tests")

find_package(Qt6 REQUIRED COMPONENTS Test Core)

add_executable(qm_parser_test
    TestParser.h
    TestParser.cpp
)

target_link_libraries(qm_parser_test PRIVATE
    Qt6::Test
    Qt6::Core
    QM
)

add_test(NAME qm_parser_test COMMAND qm_parser_test WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

add_dependencies(qm_tests qm_parser_test)

if (ENABLE_COVERAGE)
    add_dependencies(run_all_tests qm_tests)
endif()
