cmake_minimum_required(VERSION 3.23)

project(binary-parser C)

add_executable(binary-parser)

target_sources(binary-parser
    PRIVATE
        src/main.c
        src/section.c
        src/attribute.c
    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS include
        FILES
            include/section.h
            include/attribute.h
)