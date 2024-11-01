cmake_minimum_required (VERSION 3.20.0)
 
project (2048-cpp)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

FetchContent_Declare(ftxui
  GIT_REPOSITORY https://github.com/ArthurSonzogni/ftxui
  GIT_TAG main
)

FetchContent_MakeAvailable(ftxui)

include_directories(include)

aux_source_directory(src DIR_SRCS)

add_executable(2048-tui ${DIR_SRCS})
target_link_libraries(2048-tui
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component
)

if (EMSCRIPTEN)
  string(APPEND CMAKE_CXX_FLAGS " -s USE_PTHREADS")
  string(APPEND CMAKE_EXE_LINKER_FLAGS " -s ASYNCIFY")
  string(APPEND CMAKE_EXE_LINKER_FLAGS " -s PROXY_TO_PTHREAD")
  string(APPEND CMAKE_EXE_LINKER_FLAGS " -s ALLOW_MEMORY_GROWTH=1")

  foreach(file "index.html" "run_webassembly.py")
    configure_file("src/${file}" ${file})
  endforeach(file)
endif()