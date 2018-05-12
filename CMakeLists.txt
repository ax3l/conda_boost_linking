# Preamble ####################################################################
#
cmake_minimum_required(VERSION 3.10.0)

project(TestBoost VERSION 1.0.0 LANGUAGES CXX)


# Project structure ###########################################################
#
# temporary build directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
# install directories
set(CMAKE_INSTALL_BINDIR bin)
set(CMAKE_INSTALL_LIBDIR lib)
set(CMAKE_INSTALL_INCLUDEDIR include)
set(CMAKE_INSTALL_CMAKEDIR lib/cmake/TestBoost)


# Options and Variants ########################################################
#
set(CMAKE_CONFIGURATION_TYPES "Release;Debug;MinSizeRel;RelWithDebInfo")
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING
        "Choose the build type, e.g. Release or Debug." FORCE)
endif()
if (CMAKE_BUILD_TYPE STREQUAL Debug)
    # TODO: add directly to targets
    add_definitions(-DDEBUG)
endif()

include(CTest)
# automatically defines: BUILD_TESTING, default is ON


# Warnings ####################################################################
#
# TODO: LEGACY! Use CMake TOOLCHAINS instead!
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address,memory,undefined")
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Weverything")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
    string(CONCAT CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} "
                  "-Wno-weak-vtables -Wno-padded -Wno-switch-enum "
                  "-Wno-undefined-func-template -Wno-c++98-compat-pedantic")
    # silence HDF5 (FIXME: -isystem should be enough)
    string(CONCAT CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} "
                  "-Wno-reserved-id-macro -Wno-deprecated -Wno-old-style-cast "
                  "-Wno-cpp")
    # older clangs: silence unknown warnings
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unknown-warning-option")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
    # silence BOOST filesystem
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-cpp")
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    # Warning C4503: "decorated name length exceeded, name was truncated"
    # Symbols longer than 4096 chars are truncated
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -wd4503")
endif ()


# Dependencies ################################################################
#
# external library: Boost (mandatory)
find_package(Boost 1.62.0 REQUIRED
    COMPONENTS system filesystem)

# export symbols in libraries
if(WIN32)
    set(TestBoost_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()


# Targets #####################################################################
#
set(CORE_SOURCE
    src/myLib.cpp
)

# library
add_library(myLib ${CORE_SOURCE})

# properties
target_compile_features(myLib
    PUBLIC cxx_std_11
)
set_target_properties(myLib PROPERTIES
    CXX_EXTENSIONS OFF
    CXX_STANDARD_REQUIRED ON
    POSITION_INDEPENDENT_CODE ON
    WINDOWS_EXPORT_ALL_SYMBOLS ON
)
#if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
#    # boost linking with unicode support
#    set_target_properties(myLib
#                          PROPERTIES COMPILE_FLAGS "/DUNICODE /D_UNICODE /Zc:wchar_t-")
#endif()
if(WIN32)
    # boost linker error with filesystem
    #target_compile_definitions(myLib PUBLIC "-DBOOST_ALL_NO_LIB=1")
    # generate a the myLib_export.h header with export defines we can use for
    # static members and other global data
    include(GenerateExportHeader)
    generate_export_header(myLib)
    # todo: use in files
endif()

# own headers
target_include_directories(myLib PUBLIC
    $<BUILD_INTERFACE:${TestBoost_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

if(TARGET Boost::filesystem)
    target_link_libraries(myLib PUBLIC
        Boost::boost Boost::system Boost::filesystem)
else()
    target_link_libraries(myLib PUBLIC
        ${Boost_LIBRARIES})
    target_include_directories(myLib SYSTEM PUBLIC ${Boost_INCLUDE_DIRS})
endif()

# app
add_executable(app src/app.cpp)

target_link_libraries(app PRIVATE myLib)


# Generate Files with Configuration Options ###################################
#
# TODO configure a version.hpp
configure_file(
    ${TestBoost_SOURCE_DIR}/TestBoostConfig.cmake.in
    ${TestBoost_BINARY_DIR}/TestBoostConfig.cmake
    @ONLY
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file("TestBoostConfigVersion.cmake"
    VERSION ${TestBoost_VERSION}
    COMPATIBILITY SameMajorVersion
)


# Installs ####################################################################
#
# headers, libraries and exectuables
install(TARGETS myLib app EXPORT TestBoostTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)
install(DIRECTORY "${TestBoost_SOURCE_DIR}/include"
    DESTINATION include
)

# CMake package file for find_package(TestBoost::TestBoost) in depending projects
install(EXPORT TestBoostTargets
    FILE TestBoostTargets.cmake
    NAMESPACE TestBoost::
    DESTINATION lib/cmake/TestBoost
)
install(
    FILES
        ${TestBoost_BINARY_DIR}/TestBoostConfig.cmake
        ${TestBoost_BINARY_DIR}/TestBoostConfigVersion.cmake
    DESTINATION lib/cmake/TestBoost
)


# Tests #######################################################################
#
if(BUILD_TESTING)
    enable_testing()

    add_test(NAME App.LsFile
        COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/app app
        WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    )
endif()


# Status Message for Build Options ############################################
#
message("")
message("TestBoost build configuration:")
message("  library Version: ${TestBoost_VERSION}")
message("  C++ Compiler: ${CMAKE_CXX_COMPILER_ID} "
                        "${CMAKE_CXX_COMPILER_VERSION} "
                        "${CMAKE_CXX_COMPILER_WRAPPER}")
message("    ${CMAKE_CXX_COMPILER}")
message("")
message("  Installation prefix: ${CMAKE_INSTALL_PREFIX}")
message("        bin: ${CMAKE_INSTALL_BINDIR}")
message("        lib: ${CMAKE_INSTALL_LIBDIR}")
message("    include: ${CMAKE_INSTALL_INCLUDEDIR}")
message("      cmake: ${CMAKE_INSTALL_CMAKEDIR}")
message("")
message("  Build Type: ${CMAKE_BUILD_TYPE}")
message("  Testing: ${BUILD_TESTING}")
message("")
