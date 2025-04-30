cmake_minimum_required(VERSION 3.31)
project(ZWOCamera)

find_package(Java REQUIRED)
find_package(JNI REQUIRED)

include_directories() #include ASICamera and OpenCV2

add_library(ZWOLibrary SHARED ZWOCamera.cpp)
