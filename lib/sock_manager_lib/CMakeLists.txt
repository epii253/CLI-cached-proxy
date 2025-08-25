add_library(s_manager sock_manager.cpp)

target_include_directories(s_manager PRIVATE ${PROJECT_SOURCE_DIR})
target_link_libraries(s_manager PRIVATE cache_rdb)