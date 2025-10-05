#最低版本号
cmake_minimum_required(VERSION 3.0)

#项目名称
project(http_framework)

#整合源文件
aux_source_directory(${PROJECT_BINARY_DIR}/../src MAIN_SRC)

#引入头文件路径
include_directories(${PROJECT_BINARY_DIR}/../include /usr/include/mysql)

#链接mysql库
# 查找 MySQL 客户端库
find_library(MYSQL_LIB 
    NAMES 
        mysqlclient 
        mysqlclient_r 
        mariadb
    PATHS
        /usr/lib64
        /usr/lib
        /usr/local/lib
        /usr/lib64/mysql
)

if(MYSQL_LIB)
    message(STATUS "Found MySQL library: ${MYSQL_LIB}")
else()
    message(FATAL_ERROR "MySQL client library not found! Please install mysql-devel or mariadb-devel")
endif()

#查找库文件
#find_library(FUNC_LIB myfunc ${PROJECT_BINARY_DIR}/../lib)

#设置可执行文件生成目录
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/../bin)

#设置编译选项
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

#生成可执行文件
add_executable(httpf ${MAIN_SRC})

#链接库文件
target_link_libraries(httpf pthread ${MYSQL_LIB})

#添加编译选项
add_compile_options(-g -lpthread -std=c++11)# -Wall 