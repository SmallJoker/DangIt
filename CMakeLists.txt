# Set up project
cmake_minimum_required(VERSION 2.6)
project(DangIt)

# Source files
set(SRC_FILES
	main.cpp
	element_simplifier.cpp
	opcodes.cpp
	script_parser.cpp
)

# Include Directories
include_directories(${PROJECT_SOURCE_DIR})


# Compiler specific configurations
if (MSVC)

else ()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pg")
endif ()


add_executable(${PROJECT_NAME} ${SRC_FILES})

target_link_libraries(
	${PROJECT_NAME}
)
