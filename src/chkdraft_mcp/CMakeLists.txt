# Compile the Chkdraft MCP server: a headless map editor on MappingCore that speaks the Model Context Protocol over
# standard input and output. The service is a library so that the tests can drive it without the protocol.
cmake_minimum_required (VERSION 3.20)

cmake_policy(SET CMP0091 NEW)

set(This ChkdraftMcp)

set(ChkdraftMcpPath ${CMAKE_CURRENT_SOURCE_DIR})
cmake_path(GET ChkdraftMcpPath PARENT_PATH SrcPath)
cmake_path(GET SrcPath PARENT_PATH ChkdraftRootPath)
include_directories(${ChkdraftRootPath}/src)

if ( PROJECT_IS_TOP_LEVEL )
  set(CMAKE_BUILD_PARALLEL_LEVEL)
  set(CMAKE_C_STANDARD 99)
  set(CMAKE_CXX_STANDARD 20)
  if ( "${CMAKE_CXX_COMPILER_ID}" MATCHES "MSVC" )
    add_compile_options(/permissive- ${ASAN})
  endif ()
  add_subdirectory(${ChkdraftRootPath}/src/cross_cut ${CMAKE_CURRENT_BINARY_DIR}/cross_cut_lib)
  add_subdirectory(${ChkdraftRootPath}/src/mapping_core ${CMAKE_CURRENT_BINARY_DIR}/mapping_core_lib)
endif()

include(FetchContent)
FetchContent_Declare(
  rarecpp
  GIT_REPOSITORY https://github.com/TheNitesWhoSay/RareCpp.git
  GIT_TAG 67de40e # release-2.6.2
)
FetchContent_MakeAvailable(rarecpp)
include_directories(${rarecpp_SOURCE_DIR}/include)

add_compile_definitions(_UNICODE UNICODE NOMINMAX CASCLIB_UNICODE __STORMLIB_NO_STATIC_LINK__)

set(Headers
  json.h
  map_service.h
  mcp_server.h
)
set(Sources
  json.cpp
  map_service.cpp
  mcp_server.cpp
)

find_package(BZip2 REQUIRED)
find_package(CascLib CONFIG REQUIRED)
find_package(StormLib CONFIG REQUIRED)

add_library(ChkdraftMcpLib STATIC ${Sources} ${Headers})
target_include_directories(ChkdraftMcpLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(ChkdraftMcpLib PUBLIC CrossCut MappingCore BZip2::BZip2 CascLib::casc${STATIC_SUFFIX} StormLib::storm)

# Every consumer of the mapping core defines the hooks in mapping_core_hooks.cpp itself, as the mapping core's own
# tests do; an archive member holding them would not be pulled in, since ChkdraftMcpLib is linked before it.
add_executable(${This} main.cpp mapping_core_hooks.cpp)
if (WIN32)
  target_link_libraries(${This} PRIVATE ChkdraftMcpLib winmm.lib comctl32.lib Kernel32.lib)
else ()
  target_link_libraries(${This} PRIVATE ChkdraftMcpLib)
endif ()

install(TARGETS ${This} RUNTIME DESTINATION bin)
