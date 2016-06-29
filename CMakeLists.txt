# Set up project
cmake_minimum_required(VERSION 2.6)
project(DangIt)

# Source files
set(SRC_FILES
	main.cpp
	scriptParser.cpp
)

# Include Directories
include_directories(
	${PROJECT_SOURCE_DIR}
)

add_executable(${PROJECT_NAME} ${SRC_FILES})

target_link_libraries(
	${PROJECT_NAME}
	${LUA_LIBRARY}
)