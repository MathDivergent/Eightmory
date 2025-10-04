# [[Root]]
cmake_minimum_required(VERSION 3.16)

project(Eightmory)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_SHARED_LIBRARY_PREFIX "")


# [[Options]]
option(EIGHTMORY_BUILD_SHARED_LIBS "Build shared libraies by Default" ON)
option(EIGHTMORY_BUILD_TEST_LIBS "Build testing libraies by Default" OFF)


# [[Defaults]]
if(PROJECT_IS_TOP_LEVEL)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/Binaries")
    if(APPLE)
        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/Frameworks")
    else()
        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/Binaries")
    endif()
endif()

if(LINUX)
    set(EIGHTMORY_RPATH "$ORIGIN")
elseif(APPLE)
    set(EIGHTMORY_RPATH "@loader_path/../Frameworks")
endif()

if(WIN32)
    set(EIGHTMORY_MODULE_EXPORT "__declspec(dllexport)")
    set(EIGHTMORY_MODULE_IMPORT "__declspec(dllimport)")
else()
    set(EIGHTMORY_MODULE_EXPORT "__attribute__((visibility(\"default\")))")
    set(EIGHTMORY_MODULE_IMPORT "")
endif()


# [[Binaries]]
if(EIGHTMORY_BUILD_SHARED_LIBS)
    set(PROJECT_LIBS_TYPE SHARED)
else()
    set(PROJECT_LIBS_TYPE STATIC)
endif()

file(GLOB_RECURSE PROJECT_SOURCES_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp")
add_library(Eightmory ${PROJECT_LIBS_TYPE} ${PROJECT_SOURCES_FILES})
target_include_directories(Eightmory PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

if(EIGHTMORY_BUILD_SHARED_LIBS)
    target_compile_definitions(Eightmory PRIVATE "EIGHTMORY_API=${EIGHTMORY_MODULE_EXPORT}" INTERFACE "EIGHTMORY_API=${EIGHTMORY_MODULE_IMPORT}")
else()
    target_compile_definitions(Eightmory PUBLIC "EIGHTMORY_API=")
endif()

if(NOT EIGHTMORY_SEGMENT_ALIGN)
    set(EIGHTMORY_SEGMENT_ALIGN "sizeof(std::size_t)")
endif()

target_compile_definitions(Eightmory PUBLIC "EIGHTMORY_SEGMENT_ALIGN=${EIGHTMORY_SEGMENT_ALIGN}")

# [[Tests]]
if(EIGHTMORY_BUILD_TEST_LIBS)
    file(GLOB_RECURSE PROJECT_TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/test/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/test/*.hpp")
    add_library(EightmoryTests SHARED ${PROJECT_TEST_SOURCES})

    if(PROJECT_IS_TOP_LEVEL)
        # you should manually download Eightest if not
        set(EIGHTEST_RUN_MODULE "EightmoryTests")
        add_subdirectory("Eightest")
    endif()

    target_link_libraries(EightmoryTests PUBLIC Eightmory Eightest)
    target_include_directories(EightmoryTests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/test")
    set_target_properties(EightmoryTests PROPERTIES BUILD_WITH_INSTALL_RPATH TRUE INSTALL_RPATH "${EIGHTMORY_RPATH}")

    target_compile_definitions(Eightmory PUBLIC "EIGHTMORY_DEBUG")
endif()
