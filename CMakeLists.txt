cmake_minimum_required(VERSION 3.16)
project(MovieTicketBookingSystem VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Core Widgets Sql PrintSupport QUIET)
if(NOT Qt6_FOUND)
    find_package(Qt5 COMPONENTS Core Widgets Sql PrintSupport REQUIRED)
endif()

set(SOURCES
    src/main.cpp
    src/database.cpp
    src/loginwindow.cpp
    src/mainwindow.cpp
    src/adminwindow.cpp
    src/moviemanager.cpp
    src/seatselection.cpp
    src/bookingmanager.cpp
    src/receiptdialog.cpp
    src/reportwindow.cpp
)

set(HEADERS
    include/database.h
    include/loginwindow.h
    include/mainwindow.h
    include/adminwindow.h
    include/moviemanager.h
    include/seatselection.h
    include/bookingmanager.h
    include/receiptdialog.h
    include/reportwindow.h
    include/models.h
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

if(Qt6_FOUND)
    target_link_libraries(${PROJECT_NAME} Qt6::Core Qt6::Widgets Qt6::Sql Qt6::PrintSupport)
else()
    target_link_libraries(${PROJECT_NAME} Qt5::Core Qt5::Widgets Qt5::Sql Qt5::PrintSupport)
endif()

target_include_directories(${PROJECT_NAME} PRIVATE include)
