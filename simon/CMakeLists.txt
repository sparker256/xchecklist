# Xchecklist build script.
#
# Useful command line flags:
#   * -DCMAKE_BUILD_TYPE=XXX, where XXX is one of Debug, Release,
#     RelWithDebInfo and MinSizeRel. Default is Release.
#   

cmake_minimum_required(VERSION 3.9)
project(simon VERSION 1.3.4 DESCRIPTION "Xchecklist X-Plane plugin talker")

# We will enable this when we have a SDK supporting arm
# X-Plane supports OS X 11.0+, so this should ensure Xchecklist can run on
# all supported versions.
set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "Minimum macOS version" FORCE)
set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64" CACHE STRING "Build architectures for mac OS X" FORCE)


# By default, use optimized release configuration.
if (NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "")
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
endif ()

set(CMAKE_CXX_STANDARD 17)

# Set include directories used by our code and dependencies.
include_directories("../src/SDK/CHeaders/XPLM")
include_directories("../src")

message (STATUS "CMAKE_LIBRARY_PATH" = ${CMAKE_LIBRARY_PATH})

add_definitions(-DXPLM200=1 -DXPLM210=1 -DXPLM300=1 -DXPLM301=1 -DCHECKER)

add_definitions(-DAPL=$<BOOL:${APPLE}> -DIBM=$<BOOL:${WIN32}> -DLIN=$<AND:$<BOOL:${UNIX}>,$<NOT:$<BOOL:${APPLE}>>>)

# Force-enable exception support. This is most likely redundant, although for C
# code the default is the opposite. Since we are mixing C++ and C libraries,
# safer to set it on?
add_compile_options(-fexceptions)
# On UNIX systems this makes symbols non-exported by default. On Windows this
# option is simply ignored, since symbol visibility works differently there.
add_compile_options(-fvisibility=hidden)

# Enable stricter warnings and then disable some we are not interested in.
add_compile_options(-Wall -Wpedantic -Wshadow -Wfloat-equal -Wextra)
add_compile_options(-Wno-unused)

# Always use position-independent code and highest optimization level (FPS!).
add_compile_options(-O3 -fPIC)

# Simon
if (APPLE)
	list(APPEND SIMON_SRCS ../src/messages.c simon.m)
else ()
	list(APPEND SIMON_SRCS ../src/messages.c simon.c ../src/speech_sd.c)
endif ()

add_executable(simon ${SIMON_SRCS}) 

target_compile_features(simon PUBLIC cxx_std_11)

if (APPLE)
    # X-Plane supports OS X 10.10+, so this should ensure FlyWithLua can run on
    # all supported versions.
    target_compile_options(simon PUBLIC -mmacosx-version-min=10.10)
    target_link_libraries(simon -mmacosx-version-min=10.10)
endif ()

if (APPLE)
	find_library(APPKIT_LIBRARY AppKit)
endif ()

if (APPLE)
    target_link_libraries(simon -flat_namespace "-undefined suppress" ${APPKIT_LIBRARY})
else ()
    target_link_libraries(simon -ldl -pthread)
endif ()

set_target_properties(simon PROPERTIES PREFIX "")
set_target_properties(simon PROPERTIES OUTPUT_NAME "simon")
if (APPLE)
	set_target_properties(simon PROPERTIES SUFFIX "_mac")
else ()
	if (32BIT)
		set_target_properties(simon PROPERTIES SUFFIX "_lin32")
        else ()
		set_target_properties(simon PROPERTIES SUFFIX "_lin64")
        endif ()
endif ()

