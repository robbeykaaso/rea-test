cmake_minimum_required(VERSION 3.5)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#if (CMAKE_BUILD_TYPE STREQUAL Debug)
#set(${CMAKE_BINARY_DIR} install)
if (NOT DEFINED MS)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/../build-app-Desktop_Qt_5_12_2_MSVC2017_64bit-${CMAKE_BUILD_TYPE}/plugin)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/install/lib)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/../build-app-Desktop_Qt_5_12_2_MSVC2017_64bit-${CMAKE_BUILD_TYPE}/plugin)
else()
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${MS})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/install/lib)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${MS})
endif()

project(REA_TEST)

#add_definitions(-DEXPORTDLL)

find_package(Qt5 COMPONENTS Widgets Core Quick SerialPort SerialBus REQUIRED)
set(REA_DIR ${CMAKE_CURRENT_LIST_DIR}/../frm/cmake)
find_package(REA)

# 根据库文件代码生成动态库
add_library(REA_TEST SHARED
    "qml.qrc"
    "test_aop.cpp"
    "test_modbus.cpp"
    "test_qml.cpp"
    "test_qsg.cpp"
    "test_rea.cpp"
    "test_storage.cpp"
    "test_tcp.cpp"
    )
# 创建动态库别名

#add_library(hello::library ALIAS REA_TEST)

target_link_libraries(REA_TEST PRIVATE
    Qt5::Core Qt5::Quick Qt5::Widgets Qt5::SerialPort Qt5::SerialBus
    ${REA_LIBS})

include_directories(${REA_INCLUDE_DIRS})
