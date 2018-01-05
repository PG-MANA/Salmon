cmake_minimum_required(VERSION 3.1)

add_library(Network STATIC Network.cpp OAuth.cpp SHA1cc.cpp)
target_link_libraries(Network Qt5::Network)
