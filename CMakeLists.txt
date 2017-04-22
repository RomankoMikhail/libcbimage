cmake_minimum_required(VERSION 2.8)

project (cbimage)

if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set (CMAKE_INSTALL_PREFIX "/usr" CACHE PATH "default install path" FORCE )
endif()

set(CMAKE_BUILD_TYPE Release CACHE Release "Build type" FORCE)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
file(GLOB_RECURSE sources  ${CMAKE_CURRENT_SOURCE_DIR}/source/*.c)


add_library (${PROJECT_NAME} SHARED ${sources})
install(TARGETS ${PROJECT_NAME} DESTINATION lib)


file(GLOB HEADERS include/*.h)
install(FILES ${HEADERS} DESTINATION include/${PROJECT_NAME})