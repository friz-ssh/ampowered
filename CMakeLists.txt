cmake_minimum_required(VERSION 3.14)
project(ampowered LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(ftxui
  GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI
  GIT_TAG v7.0.3)
FetchContent_MakeAvailable(ftxui)

file(GLOB_RECURSE SOURCES src/*.cpp)
add_executable(evcs ${SOURCES})
target_include_directories(evcs PRIVATE include)
target_link_libraries(evcs PRIVATE
  ftxui::screen ftxui::dom ftxui::component)