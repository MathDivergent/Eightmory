# [[Module][Root]]
cmake_minimum_required(VERSION 3.16)

project(Eightmory)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_SHARED_LIBRARY_PREFIX "")


# [[Module][Options]]
option(EIGHTMORY_BUILD_SHARED_LIBS "Build shared libraies by Default" ON)
option(EIGHTMORY_BUILD_TEST_LIBS "Build testing libraies by Default" OFF)


# [[Module][Defaults]]
if(EIGHTMORY_BUILD_SHARED_LIBS)
    set(PROJECT_LIBS_TYPE SHARED)
else()
    set(PROJECT_LIBS_TYPE STATIC)
endif()

if(WIN32)
    set(EIGHTMORY_MODULE_EXPORT "__declspec(dllexport)")
    set(EIGHTMORY_MODULE_IMPORT "__declspec(dllimport)")
elseif(LINUX OR APPLE)
    set(EIGHTMORY_MODULE_EXPORT "__attribute__((visibility(\"default\")))")
    set(EIGHTMORY_MODULE_IMPORT "")
endif()

if(LINUX OR APPLE)
    set(EIGHTMORY_VISIBILITY_HIDDEN C_VISIBILITY_PRESET hidden CXX_VISIBILITY_PRESET hidden VISIBILITY_INLINES_HIDDEN ON)
endif()


# [[Module][Binaries]]
file(GLOB_RECURSE PROJECT_SOURCES_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp")
add_library(Eightmory ${PROJECT_LIBS_TYPE} ${PROJECT_SOURCES_FILES})
target_include_directories(Eightmory PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")


# [[Module][Configurations]]
if(EIGHTMORY_BUILD_SHARED_LIBS)
    target_compile_definitions(Eightmory PRIVATE "EIGHTMORY_API=${EIGHTMORY_MODULE_EXPORT}" INTERFACE "EIGHTMORY_API=${EIGHTMORY_MODULE_IMPORT}")

    if(LINUX OR APPLE)
        set_target_properties(Eightmory PROPERTIES ${EIGHTMORY_VISIBILITY_HIDDEN})
    endif()
else()
    target_compile_definitions(Eightmory PUBLIC "EIGHTMORY_API=")
endif()

if(NOT EIGHTMORY_SEGMENT_ALIGN)
    set(EIGHTMORY_SEGMENT_ALIGN "sizeof(std::size_t)")
endif()

target_compile_definitions(Eightmory PUBLIC "EIGHTMORY_SEGMENT_ALIGN=${EIGHTMORY_SEGMENT_ALIGN}")


# [[Tests][Defaults]]
if(EIGHTMORY_BUILD_TEST_LIBS)
    if(LINUX)
        set(EIGHTMORY_RPATH "$ORIGIN")
    elseif(APPLE)
        set(EIGHTMORY_RPATH "@loader_path/../Frameworks")
    endif()
endif()


# [[Tests][Binaries]]
if(EIGHTMORY_BUILD_TEST_LIBS)
    file(GLOB_RECURSE PROJECT_TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/test/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/test/*.hpp")
    add_library(EightmoryTests SHARED ${PROJECT_TEST_SOURCES})

    target_link_libraries(EightmoryTests PUBLIC Eightmory Eightest)
    target_include_directories(EightmoryTests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/test")
endif()


# [[Tests][Configurations]]
if(EIGHTMORY_BUILD_TEST_LIBS)
    if(LINUX OR APPLE)
        set_target_properties(EightmoryTests PROPERTIES BUILD_WITH_INSTALL_RPATH TRUE INSTALL_RPATH "${EIGHTMORY_RPATH}" ${EIGHTMORY_VISIBILITY_HIDDEN})
    endif()

    target_compile_definitions(Eightmory PUBLIC "EIGHTMORY_DEBUG")
endif()


# [[Launcher][Defaults]]
if(PROJECT_IS_TOP_LEVEL AND EIGHTMORY_BUILD_TEST_LIBS)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/Binaries")

    if(WIN32 OR LINUX)
        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/Binaries")
    elseif(APPLE)
        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/Frameworks")
    endif()
endif()


# [[Launcher][Binaries]]
if(PROJECT_IS_TOP_LEVEL AND EIGHTMORY_BUILD_TEST_LIBS)
    # you should manually download Eightest if not
    set(EIGHTEST_RUN_MODULE "EightmoryTests")
    add_subdirectory("Eightest")
endif()