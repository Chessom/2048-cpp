cmake_minimum_required (VERSION 3.30.2)
 
project (2048-cpp)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(include)
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/third-party/ftxui/include
)

aux_source_directory(src DIR_SRCS)

include(FetchContent)

FetchContent_Declare(ftxui
  GIT_REPOSITORY https://github.com/ArthurSonzogni/ftxui
  GIT_TAG main
)

FetchContent_GetProperties(ftxui)
if(NOT ftxui_POPULATED)
  FetchContent_MakeAvailable(ftxui)
endif()

add_executable(main ${DIR_SRCS})
