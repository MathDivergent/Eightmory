# [[Root]]
cmake_minimum_required(VERSION 3.5)

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
        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/Binaries/EightestLauncher.app/Contents/Frameworks")
    else()
        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/Binaries")
    endif()
endif()

if(LINUX)
    set(EIGHTMORY_RPATH "$ORIGIN")
elseif(APPLE)
    set(EIGHTMORY_RPATH "@loader_path/../Frameworks")
endif()


# [[Binaries]]
if(EIGHTMORY_BUILD_TEST_LIBS)
    set(PROJECT_LIBS_TYPE SHARED)
else()
    set(PROJECT_LIBS_TYPE STATIC)
endif()

file(GLOB_RECURSE PROJECT_SOURCES_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp")
add_library(Eightmory ${PROJECT_LIBS_TYPE} ${PROJECT_SOURCES_FILES})
target_include_directories(Eightmory PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

if(EIGHTMORY_BUILD_TEST_LIBS)
    set_target_properties(Eightmory PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
endif()
add_compile_options(-fno-rtti)


# [[Tests]]
if(EIGHTMORY_BUILD_TEST_LIBS)
    file(GLOB_RECURSE PROJECT_TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/test/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/test/*.hpp")
    add_library(EightmoryTests ${PROJECT_LIBS_TYPE} ${PROJECT_TEST_SOURCES})

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
