cmake_minimum_required(VERSION 2.8)

PROJECT(cleartextParser)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fPIC")

ADD_SUBDIRECTORY(third-party)
ADD_SUBDIRECTORY(lib)

include_directories(BEFORE 
      ${CMAKE_CURRENT_SOURCE_DIR}/include
	  ${CMAKE_CURRENT_SOURCE_DIR}/third-party
	  ${CMAKE_CURRENT_SOURCE_DIR}/config)


ADD_EXECUTABLE(cleartextParserMain main.cpp)

target_link_libraries (cleartextParserMain cleartextParser pcre_static)

install(TARGETS cleartextParserMain DESTINATION bin)