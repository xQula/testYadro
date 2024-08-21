cmake_minimum_required(VERSION 3.22)

project(yuliy
        VERSION 0.0.1
        DESCRIPTION "test task YDRO TATLIN"
        LANGUAGES C CXX
)

option(INTEGRATION_TESTS "Enable integration tests" ON)

if("${CMAKE_GENERATOR}" MATCHES "^Visual Studio")
    set(CMAKE_GENERATOR_PLATFORM "x64" CACHE STRING "" FORCE)
endif()

if(NOT CMAKE_CXX_STANDARD)
    message(STATUS "[${PROJECT_NAME}] setting c++ standard to c++23")
    set(CMAKE_CXX_STANDARD 23)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)
endif()

add_executable(yuliy)

target_sources(${PROJECT_NAME}
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/impl/common.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/impl/config.cc
)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

message(STATUS "[${PROJECT_NAME}] setting metadata definitions:")
message(STATUS "[${PROJECT_NAME}] - PROJECT_NAME: ${PROJECT_NAME}")
message(STATUS "[${PROJECT_NAME}] - PROJECT_VERSION: ${PROJECT_VERSION}")

message(STATUS "[${PROJECT_NAME}] compile integration tests status: ${INTEGRATION_TESTS}")
if(INTEGRATION_TESTS)
    find_package(GTest REQUIRED)
    enable_testing()
    add_executable(${PROJECT_NAME}-test)
    target_sources(${PROJECT_NAME}-test PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/tests-main.cc
            ${CMAKE_CURRENT_SOURCE_DIR}/src/impl/common.cc
            ${CMAKE_CURRENT_SOURCE_DIR}/src/impl/config.cc)
    target_compile_definitions(${PROJECT_NAME}-test PUBLIC UNIT_TESTS)
    target_include_directories(${PROJECT_NAME}-test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
    target_link_libraries(${PROJECT_NAME}-test
            PRIVATE
            GTest::GTest
            GTest::Main
    )
endif()