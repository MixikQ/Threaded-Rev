cmake_minimum_required(VERSION 3.28)

project(CLI-chat-server)

add_executable(Threaded-Rev main.cpp Consumer.cpp Producer.cpp ThreadedSafeQueue.cpp)
