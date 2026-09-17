# C++编译参数：C++17、调试符号、静态链接
CXXFLAGS=-std=c++17 -g -static

# 目标main，依赖main.cpp
main: main.cpp
	g++ $(CXXFLAGS) -o main main.cpp

# test伪目标：跑测试，依赖main；main过期会自动先编译
test: main
	./test.sh

# dbg伪目标：带调试输出跑一次，用法 make dbg ARGS='1--1'；依赖main保证用的是最新代码
dbg: main
	CALC_DEBUG=1 ./main $(ARGS)

# 所有"生成出来的"文件（不是源码，删了随时能重建）
# main/a.out 编译器本体，tmp/tmp.s 测试中间产物，*.o/*~ 编译与编辑器临时文件，*.log/log 输出重定向留下的
GENERATED=main a.out tmp tmp.s *.o *~ put.log debug.log log

# 清理：只删生成物，main.cpp / Makefile / test.sh 三个源码文件不受影响
clean:
	rm -f $(GENERATED)
	@echo "已清理: $(GENERATED)"

# 声明伪目标，test、dbg、clean不是磁盘文件
.PHONY: test dbg clean
