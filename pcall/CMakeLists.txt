cmake_minimum_required(VERSION 3.4.1)

add_library(pcall SHARED
            src/main/cpp/scoped_local_ref.h
            src/main/cpp/jvmti.h
            src/main/cpp/jvmti_helper.h
            src/main/cpp/jvmti_helper.cpp
            src/main/cpp/pcall.cpp)

find_library(log-lib log)

set_target_properties(pcall PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(pcall ${log-lib})