# CMake build of RedHotTools, equivalent to xmake.lua: the same sources, defines, flags and runtime,
# with the packages xmake resolves (the newest versions in xmake-repo on 2026-10-20) fetched at
# configure time and checked against xmake-repo's SHA-256 values. Generated files go to the build
# directory. The build stamp is the commit date instead of the build time, so builds are
# reproducible.
#
#   cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release [-DCMAKE_TOOLCHAIN_FILE=...]
#   cmake --build build
cmake_minimum_required(VERSION 3.24)
project(RedHotTools VERSION 1.3.0 LANGUAGES C CXX RC)

include(FetchContent)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Downloads and verifies a package without adding its own CMake project (SOURCE_SUBDIR names a
# directory that does not exist); the targets below build what is needed.
function(fetch name url hash)
  FetchContent_Declare(${name} URL ${url} URL_HASH SHA256=${hash} DOWNLOAD_EXTRACT_TIMESTAMP TRUE
                       SOURCE_SUBDIR no-cmake-project)
  FetchContent_MakeAvailable(${name})
  set(${name}_SOURCE_DIR "${${name}_SOURCE_DIR}" PARENT_SCOPE)
endfunction()

fetch(hopscotch_map https://github.com/Tessil/hopscotch-map/archive/v2.4.0.zip
      209decbd03b37aa898952d33d0450000a996be8cefba165abf0ac9a7dba42aff)
fetch(spdlog https://github.com/gabime/spdlog/archive/refs/tags/v1.17.0.zip
      b11912a82d149792fef33fabd0503b13d54aeac25c1464755461d4108ea71fc2)
fetch(minhook https://github.com/TsudaKageyu/minhook/archive/v1.3.4.tar.gz
      1aebeae4ca898330c507860acc2fca2eb335fe446a3a2b8444c3bf8b2660a14e)
fetch(mimalloc https://github.com/microsoft/mimalloc/archive/refs/tags/v2.2.4.zip
      664667a48c9f101d979bbe4e41ee631da49d2024e30d66b7779b6ba4279af367)
fetch(tiltedcore https://github.com/tiltedphoques/TiltedCore/archive/v0.2.9.zip
      aa8dd5aee40c53fd57da91ef5193458b62f3dec420e78fac9dc8293a987b9758)

# spdlog and hopscotch-map are header-only; spdlog uses its bundled fmt, as xmake's default does.
add_library(hopscotch-map INTERFACE)
target_include_directories(hopscotch-map SYSTEM INTERFACE "${hopscotch_map_SOURCE_DIR}/include")
add_library(spdlog INTERFACE)
target_include_directories(spdlog SYSTEM INTERFACE "${spdlog_SOURCE_DIR}/include")

add_library(minhook STATIC
  "${minhook_SOURCE_DIR}/src/buffer.c" "${minhook_SOURCE_DIR}/src/hook.c"
  "${minhook_SOURCE_DIR}/src/trampoline.c" "${minhook_SOURCE_DIR}/src/hde/hde64.c")
target_include_directories(minhook SYSTEM PUBLIC "${minhook_SOURCE_DIR}/include")

# mimalloc as xmake-repo builds it for TiltedCore: static, no override, RtlGenRandom.
add_library(mimalloc STATIC "${mimalloc_SOURCE_DIR}/src/static.c")
target_include_directories(mimalloc SYSTEM PUBLIC "${mimalloc_SOURCE_DIR}/include")
target_compile_definitions(mimalloc PRIVATE MI_USE_RTLGENRANDOM MI_STATIC_LIB)
target_compile_definitions(mimalloc PUBLIC MI_STATIC_LIB)
target_link_libraries(mimalloc PUBLIC advapi32 bcrypt psapi)

# TiltedCore's headers are included as <TiltedCore/...>, its xmake prefix for Code/core/include.
file(COPY "${tiltedcore_SOURCE_DIR}/Code/core/include/" DESTINATION "${CMAKE_BINARY_DIR}/tiltedcore/TiltedCore")
file(GLOB tiltedcore_sources CONFIGURE_DEPENDS "${tiltedcore_SOURCE_DIR}/Code/core/src/*.cpp")
add_library(TiltedCore STATIC ${tiltedcore_sources})
set_target_properties(TiltedCore PROPERTIES CXX_STANDARD 17)
target_include_directories(TiltedCore SYSTEM PUBLIC "${tiltedcore_SOURCE_DIR}/Code/core/include" "${CMAKE_BINARY_DIR}/tiltedcore")
target_compile_definitions(TiltedCore PRIVATE NDEBUG)
target_link_libraries(TiltedCore PUBLIC mimalloc hopscotch-map)

# Values xmake substitutes into the config templates.
set(NAME RedHotTools)
set(AUTHOR psiberx)
set(VERSION ${PROJECT_VERSION})
set(VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(VERSION_ALTER ${PROJECT_VERSION_PATCH})
string(TOLOWER "${CMAKE_BUILD_TYPE}" MODE)
set(DEBUG "")
execute_process(COMMAND git log -1 --format=%cd --date=format:%y%m%d%H%M
                WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}" OUTPUT_VARIABLE VERSION_BUILD OUTPUT_STRIP_TRAILING_WHITESPACE)
set(generated "${CMAKE_BINARY_DIR}/generated")
configure_file(config/Project.hpp.in "${generated}/App/Project.hpp")
# A VERSIONINFO number field is 16 bits. MSVC's rc.exe truncates the stamp silently (the release
# reads 1.3.0.33239 for 2610201047); llvm-rc rejects it, so the numeric field is truncated here.
# The string fields keep the full stamp.
math(EXPR VERSION_BUILD_16 "${VERSION_BUILD} & 0xFFFF")
file(READ config/Version.rc.in version_rc)
string(REPLACE "#define VER_FILEVERSION        \${VERSION_MAJOR},\${VERSION_MINOR},\${VERSION_ALTER},\${VERSION_BUILD}"
               "#define VER_FILEVERSION        \${VERSION_MAJOR},\${VERSION_MINOR},\${VERSION_ALTER},\${VERSION_BUILD_16}"
               version_rc "${version_rc}")
file(WRITE "${CMAKE_BINARY_DIR}/Version.rc.in" "${version_rc}")
configure_file("${CMAKE_BINARY_DIR}/Version.rc.in" "${generated}/App/Version.rc")
configure_file(config/version.lua.in "${generated}/cet/version.lua")

# Some includes use a case that only resolves on case-insensitive filesystems: the SDK's rtti/ for
# RTTI/ and AnimKeyFrames.hpp for animKeyFrames.hpp, and FileWatch's Pathcch.h for the Windows
# SDK's PathCch.h. When building on another host, a shim directory provides those spellings: a
# symlink for the directory, forwarding headers for the files. On Windows a forwarding header could
# resolve to itself, so none is made there.
set(case_shim "${CMAKE_BINARY_DIR}/case-shim")
if(NOT CMAKE_HOST_WIN32)
  set(sdk "${CMAKE_SOURCE_DIR}/vendor/RED4ext.SDK/include/RED4ext")
  file(MAKE_DIRECTORY "${case_shim}/RED4ext/Scripting/Natives")
  if(NOT EXISTS "${case_shim}/RED4ext/rtti")
    file(CREATE_LINK "${sdk}/RTTI" "${case_shim}/RED4ext/rtti" SYMBOLIC)
  endif()
  file(WRITE "${case_shim}/RED4ext/Scripting/Natives/AnimKeyFrames.hpp"
       "#include <RED4ext/Scripting/Natives/animKeyFrames.hpp>\n")
  file(WRITE "${case_shim}/Pathcch.h" "#include <PathCch.h>\n")
endif()

file(GLOB_RECURSE plugin_sources CONFIGURE_DEPENDS src/*.cpp lib/*.cpp)
add_library(RedHotTools SHARED ${plugin_sources} "${generated}/App/Version.rc")
set_target_properties(RedHotTools PROPERTIES PREFIX "" OUTPUT_NAME RedHotTools)
target_precompile_headers(RedHotTools PRIVATE src/pch.hpp)
target_include_directories(RedHotTools PRIVATE "${generated}" src lib vendor/filewatch)
target_include_directories(RedHotTools SYSTEM PRIVATE
  vendor/RED4ext.SDK/include "${case_shim}" vendor/nameof/include vendor/semver/include vendor/wil/include)
target_compile_definitions(RedHotTools PRIVATE
  WINVER=0x0601 WIN32_LEAN_AND_MEAN NOMINMAX _DISABLE_EXTENDED_ALIGNED_STORAGE)
target_compile_options(RedHotTools PRIVATE /GR- /EHsc $<$<CONFIG:Release>:/Ob2>)
# MSVC allows SSE3 intrinsics (CameraSystem.hpp's _mm_hadd_ps) at its default SSE2 target; clang
# needs the feature enabled. SSE3 dates from 2004, so every CPU that runs the game has it.
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  target_compile_options(RedHotTools PRIVATE -msse3)
endif()
target_link_libraries(RedHotTools PRIVATE spdlog hopscotch-map minhook TiltedCore version user32)
