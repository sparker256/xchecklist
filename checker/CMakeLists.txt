# Xchecklist build script.
#
# Useful command line flags:
#   * -DCMAKE_BUILD_TYPE=XXX, where XXX is one of Debug, Release,
#     RelWithDebInfo and MinSizeRel. Default is Release.
#   

cmake_minimum_required(VERSION 3.9)
project(checker VERSION 1.3.4 DESCRIPTION "Xchecklist X-Plane plugin")

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
add_compile_options(-O3)
if (UNIX)
    add_compile_options(-fPIC)
endif ()

# Checker
# FIXME: Split this into individual targets.
list(APPEND CHECKER_SRCS main.cpp ../src/interface.cpp ../src/parser.cpp ../src/utils.cpp ../src/chkl.l ../src/chkl.y)

# Runs flex and bison
find_package(BISON)
find_package(FLEX)
BISON_TARGET(Parser ../src/chkl.y ${CMAKE_CURRENT_BINARY_DIR}/chkl_yacc.cpp COMPILE_FLAGS "-v -p chkl -r state" DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/chkl_yacc.h)
FLEX_TARGET(Scanner ../src/chkl.l ${CMAKE_CURRENT_BINARY_DIR}/chkl_lex.cpp COMPILE_FLAGS "-P chkl")
ADD_FLEX_BISON_DEPENDENCY(Scanner Parser)

message (STATUS "CMAKE_SOURCE_DIR = " )

list(APPEND CHECKER_SRCS ${BISON_Parser_OUTPUTS} ${FLEX_Scanner_OUTPUTS})
add_executable(checker ${CHECKER_SRCS}) 

target_compile_features(checker PUBLIC cxx_std_11)

if (APPLE)
    # X-Plane supports OS X 10.10+, so this should ensure FlyWithLua can run on
    # all supported versions.
    target_compile_options(checker PUBLIC -mmacosx-version-min=10.10)
    target_link_libraries(checker -mmacosx-version-min=10.10)
endif ()

# Link OS X core system libraries.
if (APPLE)
    find_library(IOKIT_LIBRARY IOKit)
    find_library(CORE_FOUNDATION_LIBRARY CoreFoundation)
    find_library(Cocoa_LIBRARY Cocoa)

    target_link_libraries(checker ${IOKIT_LIBRARY} ${CORE_FOUNDATION_LIBRARY} ${Cocoa_LIBRARY})
endif ()

if (WIN32)
    # Unlike OS X and Linux we build standard libraries statically since X-Plane does not provide them.
    target_link_libraries(checker -static-libgcc -static-libstdc++)
    # MSYS2 for some reason always links against winpthread, so we have to work around it by statically linking thecd
    # entire plugin. This allows building the plugin nativaly on Windows.
    target_link_libraries(checker -static)
endif ()

if (WIN32)
	set_target_properties(checker PROPERTIES PREFIX "win_")
	if (32BIT)
		set_target_properties(checker PROPERTIES OUTPUT_NAME "checker_32bit")
	else ()
		set_target_properties(checker PROPERTIES OUTPUT_NAME "checker_64bit")
	endif ()
	set_target_properties(checker PROPERTIES SUFFIX ".exe")
elseif (APPLE)
	set_target_properties(checker PROPERTIES PREFIX "mac_")
	set_target_properties(checker PROPERTIES OUTPUT_NAME "checker")
	set_target_properties(checker PROPERTIES SUFFIX "")
elseif (UNIX)
	set_target_properties(checker PROPERTIES PREFIX "lin_")
	set_target_properties(checker PROPERTIES OUTPUT_NAME "checker_")
	if (32BIT)
		set_target_properties(checker PROPERTIES SUFFIX "32")
        else ()
		set_target_properties(checker PROPERTIES SUFFIX "64")
        endif ()
endif ()

