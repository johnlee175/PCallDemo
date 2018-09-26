cmake_minimum_required(VERSION 3.4.1)

find_library(log-lib log)
find_library(z-lib z)

set(slicer_src
        src/main/cpp/slicer/bytecode_encoder.cc
        src/main/cpp/slicer/code_ir.cc
        src/main/cpp/slicer/common.cc
        src/main/cpp/slicer/control_flow_graph.cc
        src/main/cpp/slicer/debuginfo_encoder.cc
        src/main/cpp/slicer/dex_bytecode.cc
        src/main/cpp/slicer/dex_format.cc
        src/main/cpp/slicer/dex_ir.cc
        src/main/cpp/slicer/dex_ir_builder.cc
        src/main/cpp/slicer/dex_utf8.cc
        src/main/cpp/slicer/instrumentation.cc
        src/main/cpp/slicer/reader.cc
        src/main/cpp/slicer/tryblocks_encoder.cc
        src/main/cpp/slicer/writer.cc)

add_library(slicer SHARED ${slicer_src})
set_target_properties(slicer PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(slicer ${z-lib})

add_library(slicer_static STATIC ${slicer_src})
set_target_properties(slicer_static PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(slicer_static ${z-lib})

add_library(pcall SHARED
            src/main/cpp/scoped_local_ref.h
            src/main/cpp/jvmti.h
            src/main/cpp/jvmti_helper.h
            src/main/cpp/jvmti_helper.cpp
            src/main/cpp/pcall.cpp)

set_target_properties(pcall PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(pcall ${log-lib} slicer_static)