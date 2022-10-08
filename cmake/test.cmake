enable_testing()

option(FETCHCONTENT_UPDATES_DISCONNECTED TRUE)
option(FETCHCONTENT_QUIET FALSE)
include(FetchContent)

FetchContent_Declare(googletest
  GIT_REPOSITORY "https://github.com/google/googletest"
  GIT_TAG        23ef29555ef4789f555f1ba8c51b4c52975f0907
  GIT_PROGRESS   TRUE
)
FetchContent_GetProperties(googletest)
if(NOT googletest_POPULATED)
  FetchContent_Populate(googletest)
  set(BUILD_GMOCK OFF CACHE INTERNAL "")
  set(INSTALL_GTEST OFF CACHE INTERNAL "")
  set(gtest_force_shared_crt ON CACHE INTERNAL "")
  add_subdirectory(
    ${googletest_SOURCE_DIR}
    ${googletest_BINARY_DIR}
    EXCLUDE_FROM_ALL
  )
endif()

add_executable(tests
  src/expander_test.cpp
)

target_link_libraries(tests
  PRIVATE json-tui-lib
  PRIVATE gtest_main
)
target_include_directories(tests
  PRIVATE src
)
target_compile_features(tests PUBLIC cxx_std_20)

include(GoogleTest)
gtest_discover_tests(tests
  DISCOVERY_TIMEOUT 600
)
