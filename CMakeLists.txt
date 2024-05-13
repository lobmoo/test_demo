cmake_minimum_required(VERSION 3.12)

project(box)


# 设置C++标准
set(CMAKE_CXX_STANDARD   17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#set(CMAKE_C_COMPILER aarch64-linux-gnu-gcc)
#set(CMAKE_CXX_COMPILER aarch64-linux-gnu-g++)


# 添加包含目录
# include_directories命令用于添加头文件的搜索路径
include_directories(
        ./include
    )

 #命令用于指定库文件的搜索路径
#link_directories(../lib)


# 查找目录下的所有源文件
# 并将名称保存到 DIR_SRCS 变量
aux_source_directory(${CMAKE_SOURCE_DIR}  DIR_SRCS)
aux_source_directory(./src  DIR_SRCS)

#可以自己枚举
#set(DIR_SRCS
#    main.cc
#)

#指定文件生成路径 目前是当前cmakelist文件所在根目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

# 添加可执行文件目标
add_executable(${PROJECT_NAME} ${DIR_SRCS})

# 添加依赖的库
target_link_libraries(${PROJECT_NAME}
            pthread
       )

# 添加编译选项
# target_compile_options命令用于添加编译选项
# target_compile_options(YourExecutableName PRIVATE -Wall -Wextra)



