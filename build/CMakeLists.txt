#最低版本号
cmake_minimum_required(VERSION 3.0)

#项目名称
project(http_framework)

#整合源文件
aux_source_directory(${PROJECT_BINARY_DIR}/../src MAIN_SRC)

#引入头文件路径
include_directories(${PROJECT_BINARY_DIR}/../include)

#查找库文件
#find_library(FUNC_LIB myfunc ${PROJECT_BINARY_DIR}/../lib)

#设置可执行文件生成目录
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/../bin)

#生成可执行文件
add_executable(httpf ${MAIN_SRC})

#链接库文件
#target_link_libraries(httpf ${FUNC_LIB})