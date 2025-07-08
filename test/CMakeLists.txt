# [[Root]]
project(MemoryManagerTesting)


# [[Subdirectories]]
# add_subdirectory("Automation")


# [[Binaries]]
file(GLOB_RECURSE PROJECT_TEST_SOURCES "${CMAKE_CURRENT_LIST_DIR}/*.cpp" "${CMAKE_CURRENT_LIST_DIR}/*.hpp")
add_executable("${PROJECT_NAME}" ${PROJECT_TEST_SOURCES})


# [[Dependencies]]
target_link_libraries("${PROJECT_NAME}" PRIVATE #[[Automation]] MemoryManager)
target_include_directories("${PROJECT_NAME}" PRIVATE "${CMAKE_CURRENT_LIST_DIR}")
