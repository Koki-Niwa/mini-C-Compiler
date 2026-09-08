# C++编译参数：C++17、调试符号、静态链接
CXXFLAGS=-std=c++17 -g -static

# 目标main，依赖main.cpp
main: main.cpp
	g++ $(CXXFLAGS) -o main main.cpp

# test伪目标：跑测试，依赖main；main过期会自动先编译
test: main
	./test.sh

# 清理：删除编译产物、临时汇编、测试生成文件
clean:
	rm -f main *.o *~ tmp tmp.s

# 声明伪目标，test、clean不是磁盘文件
.PHONY: test clean
