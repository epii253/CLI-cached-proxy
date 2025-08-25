add_library(request_proc request_process.cpp)

target_include_directories(request_proc PRIVATE ${PROJECT_SOURCE_DIR})
target_link_libraries(request_proc PRIVATE cpr)