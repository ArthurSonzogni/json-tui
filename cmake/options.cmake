function(json_tui_set_option library)
  if(CLANG_TIDY_EXE AND JSON_TUI_CLANG_TIDY)
    set_target_properties(${library}
      PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_EXE};-warnings-as-errors=*"
    )

    # By using "PUBLIC" as opposed to "SYSTEM INTERFACE", the compiler warning
    # are enforced on the headers. This is behind "JSON_TUI_CLANG_TIDY", so that it
    # applies only when developing FTXUI and on the CI. User's of the library
    # get only the SYSTEM INTERFACE instead.
    target_include_directories(${library}
      PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    )
  else()
    target_include_directories(${library} SYSTEM
      INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    )
  endif()

  target_include_directories(${library} SYSTEM
    INTERFACE
      $<INSTALL_INTERFACE:include>
  )

  target_include_directories(${library}
    PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/src>
  )

  # C++17 is used. We require fold expression at least.
  target_compile_features(${library} PUBLIC cxx_std_17)

  # Disable C++ exceptions.
  add_definitions(-DJSON_NOEXCEPTION)
  target_compile_options(json-tui-lib PUBLIC -fno-exceptions)

  # Add as many warning as possible:
  if (NOT WIN32)
    target_compile_options(${library} PRIVATE "-Wall")
    target_compile_options(${library} PRIVATE "-Wextra")
    target_compile_options(${library} PRIVATE "-pedantic")
    target_compile_options(${library} PRIVATE "-Werror")
    target_compile_options(${library} PRIVATE "-Wmissing-declarations")
    target_compile_options(${library} PRIVATE "-Wshadow")
  endif()
endfunction()
