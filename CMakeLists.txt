# CMakeList.txt : CMake project for oko-csp-webapp, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("oko-csp-webapp")

# Set paths for includes, libraries, and binaries
set(CSP_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(CSP_LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib/CSP")
set(CSP_BIN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/bin/CSP")

# Add the include directories 
include_directories(${CSP_INCLUDE_DIR})

# Include CSP library files
link_directories(${CSP_LIB_DIR})

# Configure CSP library and dll files depending on build type
set(CSP_LIB "")
set(CSP_DLL "")

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
	set(CSP_LIB "${CSP_LIB_DIR}/ConnectedSpacesPlatform_D.lib")
	set(CSP_DLL "${CSP_BIN_DIR}/ConnectedSpacesPlatform_D.dll")
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Release")
	set(CSP_LIB "${CSP_LIB_DIR}/ConnectedSpacesPlatform.lib")
	set(CSP_DLL "${CSP_BIN_DIR}/ConnectedSpacesPlatform.dll")
endif()

# Look for crow library. Throw error if not found
find_package(Crow CONFIG REQUIRED)

# CSP source files
file(GLOB_RECURSE CSP_HEADERS "${CSP_INCLUDE_DIR}/CSP/*.h")

# Root files
set(CORE_FILES
	"oko-csp-webapp.cpp"
	"oko-csp-webapp.h"
	"global.cpp"
	"global.h"
	"ComponentManager.cpp"
	"ComponentManager.h"
	"Init.cpp"
	"Init.h"
	"SpaceManager.cpp"
	"SpaceManager.h"
	"ApiClient.cpp"
	"ApiClient.h"
)

# Add source to this project's executable.
add_executable (oko-csp-webapp ${CORE_FILES} ${CSP_HEADERS})

# Link libraries to the project
target_link_libraries(oko-csp-webapp PRIVATE Crow::Crow asio::asio ${CSP_LIB})

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET oko-csp-webapp PROPERTY CXX_STANDARD 20)
endif()

# Copy DLLs to the output directory
add_custom_command(TARGET oko-csp-webapp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CSP_DLL}
        $<TARGET_FILE_DIR:oko-csp-webapp>)


# TODO: Add tests and install targets if needed.
