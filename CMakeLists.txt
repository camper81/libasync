cmake_minimum_required(VERSION 3.8)

project(async)

add_executable(test main.cpp threadsafe_queue.h)

add_library(async SHARED async.cpp async.h)