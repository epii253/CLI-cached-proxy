add_library(cache_rdb cache_rdb.cpp)

target_include_directories(cache_rdb PRIVATE ${PROJECT_SOURCE_DIR})
target_link_libraries(cache_rdb PUBLIC redis++ hiredis cpr::cpr request_proc)