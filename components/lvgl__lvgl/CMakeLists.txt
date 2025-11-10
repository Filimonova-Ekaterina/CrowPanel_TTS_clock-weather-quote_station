# components/lvgl__lvgl/CMakeLists.txt
cmake_minimum_required(VERSION 3.16)

file(GLOB_RECURSE LVGL_SRCS
    "${CMAKE_CURRENT_LIST_DIR}/src/*.c"
    "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp"
)

set(LVGL_INC "")
if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/include")
    list(APPEND LVGL_INC "include")
endif()
list(APPEND LVGL_INC ".")
list(APPEND LVGL_INC "src")

idf_component_register(
    SRCS ${LVGL_SRCS}
    INCLUDE_DIRS ${LVGL_INC}
)
