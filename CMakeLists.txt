project(nucpackgen)
cmake_minimum_required(VERSION 2.8)
#set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/../)
add_definitions(-Wall -O2 -ggdb)
add_executable(${PROJECT_NAME} src/nucpackgen.c src/nucpack.c)
target_link_libraries(${PROJECT_NAME})
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
install(DIRECTORY sys_cfg DESTINATION share/${PROJECT_NAME}/)

