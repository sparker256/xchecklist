# Xchecklist build script.
#
# Useful command line flags:
#   * -DCMAKE_BUILD_TYPE=XXX, where XXX is one of Debug, Release,
#     RelWithDebInfo and MinSizeRel. Default is Release.
#   

cmake_minimum_required(VERSION 3.9)
project(checker VERSION 1.3.4 DESCRIPTION "Xchecklist X-Plane plugin")

option(SDK2 "Build with SDK2" OFF)
option(32BIT "Build with 32 Bit" OFF)

if (32BIT)
	if (UNIX)
		message ("32BIT option is on")
		SET(CMAKE_C_FLAGS "-m32" CACHE STRING "" FORCE)
		SET(CMAKE_CXX_FLAGS "-m32" CACHE STRING "" FORCE)
		Set(FIND_LIBRARY_USE_LIB64_PATHS OFF)
		message ("CMAKE_C_FLAGS = " ${CMAKE_C_FLAGS} "   CMAKE_CXX_FLAGS = " ${CMAKE_CXX_FLAGS})
		find_library(DL_LIBRARY dl)
		message(STATUS DL_LIBRARY = ${DL_LIBRARY})
	endif ()
endif()


# By default, use optimized release configuration.
if (NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "")
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
endif ()

set(CMAKE_CXX_STANDARD 14)

# Set include directories used by our code and dependencies.
include_directories("../src/XPSDK301/CHeaders/Widgets")
include_directories("../src/XPSDK301/CHeaders/Wrappers")
include_directories("../src/XPSDK301/CHeaders/XPLM")
include_directories("../src/WinSDK/Include")
include_directories("../src")
include_directories("..")
message ("CMAKE_SOURCE_DIR = " ${CMAKE_SOURCE_DIR}"/src/SDK/CHeaders/Widgets")

if (WIN32)
	include_directories("${CMAKE_SOURCE_DIR}/WinSDK/Include")
endif ()


# Specify library search locations.
if (WIN32)
    list(APPEND CMAKE_LIBRARY_PATH "../src/XPSDK301/Libraries/Win")
	if (32BIT)
		list(APPEND CMAKE_LIBRARY_PATH "../src/WinSDK/Lib")
	else ()
		list(APPEND CMAKE_LIBRARY_PATH "../src/WinSDK/Lib/x64")
	endif ()
elseif (APPLE)
    list(APPEND CMAKE_FRAMEWORK_PATH "../src/XPSDK301/Libraries/Mac")
elseif (UNIX)

endif ()

message (STATUS "CMAKE_LIBRARY_PATH" = ${CMAKE_LIBRARY_PATH})

# Enable all X-Plane SDK APIs up to the newest version.
if (SDK2)
	add_definitions(-DXPLM200=1)
else ()
	add_definitions(-DXPLM200=1 -DXPLM210=1 -DXPLM300=1 -DXPLM301=1)
endif ()

# Define platform macros.
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

add_library(checker SHARED ${CHECKER_SRCS})

target_compile_features(checker PUBLIC cxx_std_11)

if (APPLE)
    # X-Plane supports OS X 10.10+, so this should ensure FlyWithLua can run on
    # all supported versions.
    target_compile_options(checker PUBLIC -mmacosx-version-min=10.10)
    target_link_libraries(checker -mmacosx-version-min=10.10)
endif ()

# Link OpenGL and OpenAL related libraries.
find_package(OpenGL REQUIRED)  # apt install freeglut3-dev
find_library(OPENAL_LIBRARY NAMES openal OpenAL OpenAL32)  # apt install libopenal-dev
find_library(GLUT_LIBRARY NAMES glut GLUT glut64)  # apt install freeglut3-dev
# target_link_libraries(checker ${OPENGL_LIBRARIES} ${OPENAL_LIBRARY} ${GLUT_LIBRARY})

# Link X-Plane plugin system libraries. They are only provided for OS X and Windows.
if (WIN32 OR APPLE)
    find_library(XPLM_LIBRARY NAMES XPLM XPLM_64.lib)
	find_library(XPWIDGETS_LIBRARY NAMES XPWidgets XPWidgets_64.lib)
endif ()

# Find Windows Speech API
if (WIN32)
	find_library(SAPI_LIBRARY NAMES sapi.lib SetupAPI.Lib) 
endif ()

# create target libraries for Windows or Mac
if (WIN32)
	target_link_libraries(checker ${XPLM_LIBRARY} ${XPWIDGETS_LIBRARY} ${SAPI_LIBRARY})
elseif (APPLE)
	target_link_libraries(checker ${XPLM_LIBRARY} ${XPWIDGETS_LIBRARY})
endif ()

# Link library for dynamic loading of shared objects on UNIX systems.
if (UNIX)
#    find_library(DL_LIBRARY dl)
#    target_link_libraries(checker ${DL_LIBRARY})
endif ()

# Link OS X core system libraries.
if (APPLE)
    find_library(IOKIT_LIBRARY IOKit)
    find_library(CORE_FOUNDATION_LIBRARY CoreFoundation)
    find_library(GLUT_LIBRARY GLUT)
    find_library(OpenGL_LIBRARY OpenGL)
    find_library(Cocoa_LIBRARY Cocoa)

    target_link_libraries(checker ${IOKIT_LIBRARY} ${CORE_FOUNDATION_LIBRARY} ${GLUT_LIBRARY})
    target_link_libraries(checker ${OpenGL_LIBRARY} ${Cocoa_LIBRARY})
endif ()

if (WIN32)
    # Unlike OS X and Linux we build standard libraries statically since X-Plane does not provide them.
    target_link_libraries(checker -static-libgcc -static-libstdc++ opengl32 sapi)
    # MSYS2 for some reason always links against winpthread, so we have to work around it by statically linking thecd
    # entire plugin. This allows building the plugin nativaly on Windows.
    target_link_libraries(checker -static)
elseif (APPLE)
    # Restrict set of symbols exported from the plugin. This reduces changes of
    # conflict with other plugins, in particular ones with Lua interpreter
    # embedded.
	target_link_libraries(checker "-framework XPLM -framework XPWidgets -ldl")
elseif (UNIX)
    # Specify additional runtime search laths for dynamically-linked libraries.
    # Restrict set of symbols exported from the plugin. This reduces changes of
    # conflict with other plugins, in particular ones with Lua interpreter
    # embedded.
	target_link_libraries(checker -ldl)
endif ()


if (WIN32)
	set_target_properties(checker PROPERTIES PREFIX "win_")
	set_target_properties(checker PROPERTIES OUTPUT_NAME "checker_64bit")
	set_target_properties(checker PROPERTIES SUFFIX ".exe")
elseif (APPLE)
	set_target_properties(checker PROPERTIES PREFIX "mac_")
	set_target_properties(checker PROPERTIES OUTPUT_NAME "checker")
	set_target_properties(checker PROPERTIES SUFFIX "")
elseif (UNIX)
	set_target_properties(checker PROPERTIES PREFIX "lin_")
	set_target_properties(checker PROPERTIES OUTPUT_NAME "checker_")
	set_target_properties(checker PROPERTIES SUFFIX "64")
endif ()
