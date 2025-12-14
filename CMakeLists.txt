cmake_minimum_required(VERSION 3.28)
project(CLI-chat-server)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(OpenCV_DIR "C:/opencv/build")
find_package(OpenCV REQUIRED)

# Для Visual Studio
if(MSVC)
    add_compile_options(/std:c++17 /EHsc)
else()
    add_compile_options(-std=c++17 -pthread)
endif()

add_executable(Threaded-Rev 
    main.cpp 
    Consumer.cpp 
    Producer.cpp 
    ThreadedSafeQueue.cpp
    ConfigManager.cpp  # Добавили ConfigManager
)

target_include_directories(Threaded-Rev PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(Threaded-Rev PRIVATE ${OpenCV_LIBS})

# Для std::filesystem
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_link_libraries(Threaded-Rev stdc++fs)
endif()
