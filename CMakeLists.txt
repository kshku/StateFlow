cmake_minimum_required(VERSION 3.15...3.31)

project(StateFlow VERSION 0.0.0)

set(TARGET_PLATFORM "Desktop" CACHE STRING "Target platform (desktop, android)")
set_property(CACHE TARGET_PLATFORM PROPERTY STRINGS Desktop Android)

message(STATUS "TARGET_PLATFORM = ${TARGET_PLATFORM}")

set(APP_NAME stateflow)

set(gcc_clang "$<COMPILE_LANG_AND_ID:C,Clang,GNU>")
set(msvc_comp "$<COMPILE_LANG_AND_ID:C,MSVC>")


if(TARGET_PLATFORM STREQUAL "Desktop")
    add_executable(${APP_NAME})

    add_custom_command(
        TARGET ${APP_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different ${PROJECT_SOURCE_DIR}/assets ${PROJECT_BINARY_DIR}/assets
        USES_TERMINAL
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
elseif(TARGET_PLATFORM STREQUAL "Android")
    add_library(${APP_NAME} SHARED)
endif()

add_subdirectory(external EXCLUDE_FROM_ALL)

add_subdirectory(src)
