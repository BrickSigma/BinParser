cmake_minimum_required(VERSION 3.23)

project(BinParser 
    VERSION 0.1.0
    LANGUAGES CXX
)

add_executable(binparser)

target_sources(binparser
    PRIVATE
        src/main.cpp
        src/bpsparser.cpp
        src/section.cpp
        src/attribute.cpp
    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS include
        FILES
            include/bpsparser.hpp
            include/section.hpp
            include/attribute.hpp
)

# Used to install the application globally
install(TARGETS binparser)