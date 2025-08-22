cmake_minimum_required(VERSION 3.12)

project(
    caching-proxy
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)

add_subdirectory(bin)
add_subdirectory(lib)

include(FetchContent)

find_package(cpr QUIET)
if (NOT TARGET cpr::cpr) 
	FetchContent_Declare(
		cpr
		GIT_REPOSITORY https://github.com/libcpr/cpr.git
		GIT_TAG 1.12.0
	)
	FetchContent_MakeAvailable(cpr)
endif()

set(CPR_BUILD_TESTS OFF CACHE BOOL "Disable CPR tests" FORCE)