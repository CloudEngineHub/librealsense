# License: Apache 2.0. See LICENSE file in root directory.
# Copyright(c) 2020-2024 Intel Corporation. All Rights Reserved.
cmake_minimum_required(VERSION 3.8)

project(RealsenseExamplesHdr)

if(BUILD_GRAPHICAL_EXAMPLES)
    add_executable(rs-hdr rs-hdr.cpp ../example.hpp ${IMGUI_SOURCES})
    set_property(TARGET rs-hdr PROPERTY CXX_STANDARD 11)
    target_link_libraries( rs-hdr ${DEPENDENCIES} tclap )
    include_directories(../ ../../third-party/imgui ../../third-party/glfw/include)
    set_target_properties (rs-hdr PROPERTIES FOLDER "Examples")

    install(TARGETS rs-hdr RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()
