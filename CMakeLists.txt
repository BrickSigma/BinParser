cmake_minimum_required(VERSION 3.23)

project(BinParser C)

add_executable(binparser)

target_sources(binparser
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