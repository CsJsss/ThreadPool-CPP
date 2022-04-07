cmake_minimum_required(VERSION 3.16)
project(ThreadPool CXX)

set(CMAKE_CXX_STANDARD 20)

#add_compile_options(-pthread)
if(CMAKE_COMPILER_IS_GNUCXX)
	add_compile_options(-pthread)
	message(STATUS "optional: -pthread")
endif(CMAKE_COMPILER_IS_GNUCXX)

add_executable(ThreadPool multiProducers..cpp ThreadPool.cpp)
