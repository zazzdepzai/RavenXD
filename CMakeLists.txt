cmake_minimum_required(VERSION 3.16)
project(RavenXDLauncher LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Network)

add_executable(RavenXDLauncher
    src/main.cpp
    src/MainWindow.h
    src/MainWindow.cpp
    src/Updater.h
    src/Updater.cpp
)

target_link_libraries(RavenXDLauncher PRIVATE Qt6::Widgets Qt6::Network)

# Windows: build as GUI app (no console window)
if(WIN32)
    set_target_properties(RavenXDLauncher PROPERTIES WIN32_EXECUTABLE TRUE)
endif()
