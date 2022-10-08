cmake_minimum_required (VERSION 3.11)

# Dependencies -----------------------------------------------------------------

include(FetchContent)
set(FETCHCONTENT_UPDATES_DISCONNECTED TRUE)
set(FETCHCONTENT_QUIET FALSE)

FetchContent_Declare(ftxui
  GIT_REPOSITORY https://github.com/ArthurSonzogni/ftxui
  GIT_TAG 26d63bc56fd7ff23dbd2f57c57349cdffe5016f8
  GIT_PROGRESS TRUE
  GIT_SHALLOW FALSE
)

FetchContent_Declare(json
  URL https://github.com/nlohmann/json/releases/download/v3.10.5/json.tar.xz
)

FetchContent_Declare(args
  GIT_REPOSITORY https://github.com/Taywee/args
  GIT_TAG a48e1f880813b367d2354963a58dedbf2b708584
  GIT_PROGRESS TRUE
  GIT_SHALLOW FALSE
)

FetchContent_GetProperties(ftxui)
if(NOT ftxui_POPULATED)
  FetchContent_Populate(ftxui)
  add_subdirectory(${ftxui_SOURCE_DIR} ${ftxui_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

FetchContent_GetProperties(json)
if(NOT json_POPULATED)
  FetchContent_Populate(json)
  add_subdirectory(${json_SOURCE_DIR} ${json_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

FetchContent_GetProperties(args)
if(NOT args_POPULATED)
  FetchContent_Populate(args)
  add_subdirectory(${args_SOURCE_DIR} ${args_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

# Build ------------------------------------------------------------------------

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/src/version.hpp.in
  ${CMAKE_CURRENT_BINARY_DIR}/src/version.hpp
)

project(json-tui
  LANGUAGES CXX
  VERSION 1.3.0
)

option(JSON_TUI_BUILD_TESTS "Set to ON to build tests" OFF)
option(JSON_TUI_CLANG_TIDY "Set to ON to use clang tidy" OFF)

add_library(json-tui-lib
  src/button.cpp
  src/button.hpp
  src/expander.cpp
  src/expander.hpp
  src/main_ui.cpp
  src/main_ui.hpp
  src/keybinding.cpp
  src/keybinding.hpp
  src/mytoggle.cpp
  src/mytoggle.hpp
)

add_executable(json-tui
  src/main.cpp
)

target_link_libraries(json-tui-lib
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component
  PUBLIC nlohmann_json::nlohmann_json
)

target_link_libraries(json-tui
  PRIVATE json-tui-lib
  PRIVATE args
)

include(cmake/options.cmake)
json_tui_set_option(json-tui-lib)
json_tui_set_option(json-tui)

# Tests

if (JSON_TUI_BUILD_TESTS AND ${CMAKE_VERSION} VERSION_GREATER "3.11.4")
  include(cmake/test.cmake)
endif()

# Install ----------------------------------------------------------------------

install(TARGETS json-tui RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# Package ----------------------------------------------------------------------

if (UNIX AND NOT APPLE)
  set(CPACK_GENERATOR "DEB;External;RPM;STGZ;TBZ2;TGZ;TXZ;TZ;TZST;ZIP")
elseif (UNIX AND APPLE)
  set(CPACK_GENERATOR "DragNDrop;NuGet;TGZ;ZIP")
elseif (WIN32)
  set(CPACK_GENERATOR "DEB;NuGet;TGZ;ZIP")
else()
  set(CPACK_GENERATOR "ZIP")
endif()

set(CPACK_DEBIAN_PACKAGE_DEPENDS " ")
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE_URL "https://github.com/ArthurSonzogni/json-tui/")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Arthur Sonzogni")
set(CPACK_DEBIAN_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A JSON terminal UI")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/ArthurSonzogni/json-tui/")
set(CPACK_PACKAGE_NAME "json-tui")
set(CPACK_PACKAGE_VENDOR "Arthur Sonzogni")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")

include(CPack)
