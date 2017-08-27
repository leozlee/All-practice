cmake_minimum_required(VERSION 2.8.4)
project(all)

set(CMAKE_CXX_STANDARD 11)
#查看cmake 生成的makfile的编译过程,可用于查看编译选项
#set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fprofile-arcs -ftest-coverage")

#simple test for use cmake
set(SOURCE_FILES test.cpp)
add_executable(test ${SOURCE_FILES})

#regex
set(SOURCE_FILES regex.cpp)
add_executable(test_regex ${SOURCE_FILES})
target_link_libraries(test_regex -lboost_regex)

#十进制转转16进制函数
#字符串函数
add_executable(string string.cpp)
#MESSAGE(STATUS )
#MESSAGE(STATUS "CMAKE_BINARY_DIR:" ${CMAKE_BINARY_DIR})
#MESSAGE(STATUS "CMAKE_SOURCE_DIR:" ${CMAKE_SOURCE_DIR})

SET(OUTPUT_DIR ${CMAKE_BINARY_DIR}/CMakeFiles/string.dir)
add_custom_target(run_string ALL
        #COMMAND rm ${CMAKE_BINARY_DIR}/CMakeFiles/string.dir/*
        COMMAND ${CMAKE_BINARY_DIR}/string
        COMMAND gcovr  --object-directory=${CMAKE_BINARY_DIR}/CMakeFiles/string.dir -o ${OUTPUT_DIR}/coverage.html
        -r ${CMAKE_SOURCE_DIR} --html --html-details -b -s
        COMMAND firefox ${OUTPUT_DIR}/coverage.html
        DEPENDS string
        )



#add_custom_command(TARGET string
#        POST_BUILD
#        COMMADN echo "test"
#        COMMENT "This command will be executed after build target WIFI_UNITTEST"     // 此处加入注释
#        )

#以下命令一定会执行,类似于makefile的伪目标
#add_custom_target(GCOVR ALL
#        COMMAND echo "test")


#封装popen函数
add_executable(Popen Popen.cpp)

#容器
add_executable(vector vector.cpp)



#option(ENABLE_GCOV "Enable gcov(debug, Linux builds only)" OFF)
#
#if (ENABLE_GCOV)
#    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fprofile-arcs -ftest-coverage")
#    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fprofile-arcs -ftest-coverage")
#    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -fprofile-arcs -ftest-coverage -lgcov")
#endif ()

# Setup testing
add_subdirectory(gtest)

# Include the gtest library. gtest_SOURCE_DIR is available due to
# 'project(gtest)' above.
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

enable_testing()

# End of Setup testing


##google sample test for testing env
set(Gtest_1_source
        ./sample1_unittest.cc
        ./sample1.cc
        ./sample1.h)

add_executable(gtest4test ${Gtest_1_source})
target_link_libraries(gtest4test gtest gtest_main)


#测试怎么添加gtest的目标
set(Gtest_2_source
        ./project1.cpp
        ./project1.h
        ./project1_unittest.cc)

add_executable(test4addtest ${Gtest_2_source})
target_link_libraries(test4addtest gtest gtest_main)


##以下测试用于wifi单元测试
#这个编译选项用于生成gcovr所需的文件

set(WIFI_SOURCE wifi.cpp
                wifi_unittest.cpp
                Popen.cpp)
add_executable(WIFI_UNITTEST ${WIFI_SOURCE})
target_link_libraries(WIFI_UNITTEST boost_regex)
target_link_libraries(WIFI_UNITTEST gtest gtest_main)
#add_custom_command(TARGET WIFI_UNITTEST
#        POST_BUILD
#        COMMADN echo "test"
#        COMMENT "This command will be executed after build target WIFI_UNITTEST"     // 此处加入注释
#        )

#add_custom_target(GCOVR ALL
#        COMMAND echo "test")





