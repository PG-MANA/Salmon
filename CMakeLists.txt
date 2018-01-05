cmake_minimum_required(VERSION 3.1)
project(Salmon CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 17)#C++17要求
set(CMAKE_CXX_STANDARD_REQUIRED ON)#C++17でなければ中止
set(CMAKE_CXX_EXTENSIONS OFF)#独自拡張のライブラリリンク防止

# Find the Qt library
find_package(Qt5 CONFIG REQUIRED Core Widgets Network)

#Make libraries
add_subdirectory(src/Network)
add_subdirectory(src/Twitter)
add_subdirectory(src/UI)

add_executable(Salmon src/Salmon.cpp src/Resources/icon/icon.qrc src/Resources/twitter/twitter-icon.qrc) 

#Link libraries
target_link_libraries(Salmon UI) #UI <= Twitter <= Networkとつながってる...はず

install(TARGETS Salmon DESTINATION bin)
