macro(FetchContent_MakeAvailable_NoChecks NAME)
  FetchContent_GetProperties(${NAME})
  if(NOT ${NAME}_POPULATED)
    FetchContent_Populate(${NAME})

    # exclude from clang-tidy check
    foreach (lang IN ITEMS C CXX)
      set("CMAKE_${lang}_CLANG_TIDY_save" "${CMAKE_${lang}_CLANG_TIDY}")
      set("CMAKE_${lang}_CLANG_TIDY" "")
      set("CMAKE_${lang}_CPPCHECK_save" "${CMAKE_${lang}_CPPCHECK}")
      set("CMAKE_${lang}_CPPCHECK" "")
    endforeach ()

    add_subdirectory(${${NAME}_SOURCE_DIR} ${${NAME}_BINARY_DIR})

    foreach (lang IN ITEMS C CXX)
      set("CMAKE_${lang}_CLANG_TIDY" "${CMAKE_${lang}_CLANG_TIDY_save}")
      set("CMAKE_${lang}_CPPCHECK" "${CMAKE_${lang}_CPPCHECK_save}")
    endforeach ()

  endif()
endmacro()

