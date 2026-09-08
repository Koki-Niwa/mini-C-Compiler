#include <iostream>
#include <cstdlib>
#include <string>

int main(int argc,char* argv[]){
    //参数个数错误处理
    if(argc!= 2){
        std::cerr<<"The number of parameters is incorrect."<<std::endl;
        return EXIT_FAILURE;
    }
    std::string input_str=argv[1];
    char* end_ptr=nullptr;
    long value =std::strtol(input_str.c_str(),&end_ptr,10);
    if(*end_ptr!='\0'){
        std::cerr<<"The input parameter is not purely numeric."<<std::endl;
        return EXIT_FAILURE;
    }
    std::cout<<".intel_syntax noprefix"<<std::endl;
    std::cout<<".globl main"<<std::endl;
    std::cout << "main:"<<std::endl;
    std::cout<<" mov rax,"<<value<<std::endl;
    std::cout<<" ret"<<std::endl;
}