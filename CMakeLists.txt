cmake_minimum_required(VERSION 3.5)
project(seeso_windows)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(SEESO_SUPPORT_TESTS_ONLY "Build SDK-independent regression tests" OFF)
if(SEESO_SUPPORT_TESTS_ONLY)
    enable_testing()
    add_executable(seeso_support_test tests/support_test.cpp priority_mutex.cc)
    target_include_directories(seeso_support_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    add_test(NAME support COMMAND seeso_support_test)
    set_tests_properties(support PROPERTIES TIMEOUT 15)
    return()
endif()

foreach(dependency opencv seeso)
    if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${dependency}/CMakeLists.txt")
        message(FATAL_ERROR "Missing ${dependency}/CMakeLists.txt: supply the SDK CMake wrapper described in README.md")
    endif()
endforeach()

add_subdirectory(opencv)
add_subdirectory(seeso)

add_executable(seeso_windows main.cpp
        tracker_manager.cc
        camera_thread.cc
        view.cc
        priority_mutex.cc)

target_link_libraries(seeso_windows PUBLIC opencv seeso)


if(WIN32)
# copy script will work only if cmake version >= 3.5

# copy seeso dlls
add_custom_command(TARGET seeso_windows PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${SEESO_DLL}
        $<TARGET_FILE_DIR:seeso_windows>)

#copy opencv dlls
add_custom_command(TARGET seeso_windows PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${OPENCV_DLL}
        $<TARGET_FILE_DIR:seeso_windows>)
else()
    message(AUTHOR_WARNING "SeeSo : Running on non-Windows is experimental")
endif()
