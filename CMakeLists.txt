cmake_minimum_required(VERSION 3.12)

project(
    caching-proxy
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)

include(FetchContent)

find_package(cpr QUIET)
if (NOT TARGET cpr::cpr) 
	FetchContent_Declare(
		cpr
		GIT_REPOSITORY https://github.com/libcpr/cpr.git
		GIT_TAG 1.11.0
	)
	FetchContent_MakeAvailable(cpr)
endif()
set(CPR_BUILD_TESTS OFF CACHE BOOL "Disable CPR tests" FORCE)

# ---- hiredis ----
# list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
# find_package(hiredis REQUIRED)
find_package(hiredis QUIET)
	if (NOT TARGET hiredis) 
		FetchContent_Declare(
			hiredis
			GIT_REPOSITORY https://github.com/redis/hiredis.git
			GIT_TAG v1.3.0
		)
		FetchContent_MakeAvailable(hiredis)
endif()
set(HIREDIS_BUILD_TESTS OFF CACHE BOOL "" FORCE)

set(HIREDIS_INSTALL_DIR ${CMAKE_BINARY_DIR}/hiredis_install)

# Сборка и установка hiredis
execute_process(
    COMMAND ${CMAKE_COMMAND} -S ${hiredis_SOURCE_DIR} -B ${hiredis_BINARY_DIR} 
            -DCMAKE_INSTALL_PREFIX=${HIREDIS_INSTALL_DIR} 
            -DBUILD_TESTS=OFF
)
execute_process(
    COMMAND ${CMAKE_COMMAND} --build ${hiredis_BINARY_DIR} --target install
)
# -----------------
set(hiredis_DIR ${HIREDIS_INSTALL_DIR}/share/hiredis/cmake)
set(HIREDIS_INSTALL_DIR ${CMAKE_BINARY_DIR}/hiredis_install)
set(hiredis_DIR ${HIREDIS_INSTALL_DIR}/lib/cmake/hiredis)

find_package(redis++ QUIET)
if (NOT TARGET redis++) 
	FetchContent_Declare(
		redis++
		GIT_REPOSITORY https://github.com/sewenew/redis-plus-plus.git
		GIT_TAG 1.3.15
	)

	FetchContent_MakeAvailable(redis++)
endif()
set(REDIS_PLUS_PLUS_BUILD_TEST OFF CACHE BOOL "" FORCE)
set(REDIS_PLUS_PLUS_BUILD_STATIC OFF CACHE BOOL "" FORCE)
#target_link_libraries(redis++ INTERFACE hiredis_external)


add_subdirectory(bin)
add_subdirectory(lib)

# FetchContent_Declare(
#   cpr
#   GIT_REPOSITORY https://github.com/libcpr/cpr.git
#   GIT_TAG 1.11.1
# )
# FetchContent_MakeAvailable(cpr)
