cmake_minimum_required(VERSION 3.16)

if (APPLE)
      project(ouzel LANGUAGES C CXX OBJC OBJCXX)
else()
      project(ouzel LANGUAGES C CXX)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_OBJCXX_STANDARD 17)

set(CMAKE_C_FLAGS
      "-Wall"
)

set(CMAKE_CXX_FLAGS
      "-Wall"
)

set(CMAKE_OBJC_FLAGS
      "-Wall"
)

set(CMAKE_OBJCXX_FLAGS
      "-Wall"
)

if(NOT MSVC)
      set(CMAKE_C_FLAGS
            "-Wpedantic -Wextra -Wshadow -Wdouble-promotion -Woverloaded-virtual"
      )
      set(CMAKE_CXX_FLAGS
            "-Wpedantic -Wextra -Wshadow -Wdouble-promotion -Woverloaded-virtual -Wold-style-cast"
      )
      set(CMAKE_OBJC_FLAGS
            "-fno-objc-arc -Wpedantic -Wextra -Wshadow -Wdouble-promotion -Woverloaded-virtual"
      )
      set(CMAKE_OBJCXX_FLAGS
            "-fno-objc-arc -Wpedantic -Wextra -Wshadow -Wdouble-promotion -Woverloaded-virtual -Wold-style-cas"
      )
endif()

if(MSVC)
      set(CMAKE_CXX_FLAGS "/EHca")
      set(CMAKE_OBJCXX_FLAGS "/EHca")
else()
      set(CMAKE_CXX_FLAGS "-fexceptions")
      set(CMAKE_OBJCXX_FLAGS "-fexceptions")
endif()

set(EXTERNAL_LIB_INCLUDE_DIRS
      ${CMAKE_CURRENT_LIST_DIR}/external/khronos
      ${CMAKE_CURRENT_LIST_DIR}/external/smbPitchShift
      ${CMAKE_CURRENT_LIST_DIR}/external/stb
)

set(SHADER_DIRS
      ${CMAKE_CURRENT_LIST_DIR}/shaders
)

add_subdirectory(engine)
add_subdirectory(samples)
add_subdirectory(tools)
add_subdirectory(test)
