project(signal)
cmake_minimum_required(VERSION 2.6)

#依赖源文件
aux_source_directory(./fig10.25.abort.c  SRC_ABORT)
aux_source_directory(./fig10.24.waitsignal.c  SRC_WAIT)
aux_source_directory(.  SRC_HELLO)

##添加包含的头文件
include_directories(. )
##添加库文件路径
link_directories(  /mnt/ipcframe/signal/lib )

#编译库
#add_library(hello  SHARED   ${SRC_HELLO} ${SRC_ABORT}  ${SRC_WAIT})
add_library(hello  SHARED   ${SRC_HELLO} )
#add_library(hello  SHARED hello.c )
#链接库
#target_link_libraries(hello pthread)

#编译可执行文件
#add_executable(20sigjmp fig10.20.sigjmp.c)
add_executable(main main.c)

#链接可执行文件

target_link_libraries(main hello)


#TODO
#添加测试用例
#添加安装
