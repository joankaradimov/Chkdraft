# Compile the Chkdraft MCP test executable
cmake_minimum_required (VERSION 3.20)

cmake_policy(SET CMP0091 NEW)

set(This ChkdraftMcpTest)

project(${This})

set(ChkdraftMcpTestPath ${CMAKE_CURRENT_SOURCE_DIR})
cmake_path(GET ChkdraftMcpTestPath PARENT_PATH TestPath)
cmake_path(GET TestPath PARENT_PATH ChkdraftRootPath)
include_directories(${ChkdraftRootPath}/src)

if ( PROJECT_IS_TOP_LEVEL )
  set(CMAKE_BUILD_PARALLEL_LEVEL)
  set(CMAKE_C_STANDARD 99)
  set(CMAKE_CXX_STANDARD 20)
  if ( "${CMAKE_CXX_COMPILER_ID}" MATCHES "MSVC" )
    add_compile_options(/permissive- ${ASAN})
  endif ()
  add_compile_definitions(_UNICODE UNICODE NOMINMAX _CRT_SECURE_NO_WARNINGS __STORMLIB_NO_STATIC_LINK__ DU_STATIC_IMPLEMENTATION)
  add_subdirectory(${ChkdraftRootPath}/src/cross_cut ${CMAKE_CURRENT_BINARY_DIR}/cross_cut_lib)
  add_subdirectory(${ChkdraftRootPath}/src/mapping_core ${CMAKE_CURRENT_BINARY_DIR}/mapping_core_lib)
  add_subdirectory(${ChkdraftRootPath}/src/chkdraft_mcp ${CMAKE_CURRENT_BINARY_DIR}/chkdraft_mcp_lib)
endif()

set(Sources
  chkdraft_mcp_test.cpp
  chkdraft_mcp_test_main.cpp
  ${ChkdraftRootPath}/src/chkdraft_mcp/mapping_core_hooks.cpp
)

add_executable(${This} ${Sources})

enable_testing()

find_package(GTest CONFIG REQUIRED)
include(GoogleTest)

if (WIN32)
  target_link_libraries(${This} PRIVATE GTest::gtest GTest::gtest_main ChkdraftMcpLib winmm.lib comctl32.lib Kernel32.lib)
else ()
  target_link_libraries(${This} PRIVATE GTest::gtest GTest::gtest_main ChkdraftMcpLib)
endif ()

gtest_discover_tests(${This} DISCOVERY_MODE PRE_TEST)
