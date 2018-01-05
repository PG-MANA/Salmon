cmake_minimum_required(VERSION 3.1)

add_library(Twitter STATIC  Twitter.cpp TwitterJson.cpp TwitterSetting.cpp Streamer.cpp MediaUpload.cpp)
target_link_libraries(Twitter Network)
