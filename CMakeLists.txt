CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(lisa CXX)
ADD_EXECUTABLE(lisa
connection.cpp
connection.hpp
header.hpp
lisa.cpp
reply.cpp
reply.hpp
request_handler.cpp
request_handler.hpp
request.hpp
request_parser.cpp
request_parser.hpp
server.cpp
server.hpp
router.hpp
queue.hpp
logger.hpp
globals.hpp)
INCLUDE_DIRECTORIES(/usr/include/soci /usr/include/mysql /usr/include/soci/mysql /usr/include/log4cpp)
TARGET_LINK_LIBRARIES(lisa pthread boost_thread boost_system boost_program_options mysqlclient soci_core-gcc-3_0 soci_mysql-gcc-3_0 log4cpp)

