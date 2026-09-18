#include <string>
#include <iostream>
#include <vector>
#include <cctype>
#include <utility>

namespace calc {
    //token的种类有保留字，数字，结尾标识
    enum class TokenKind{
        Reserved,
        Number,
        End,
    };
    class Token {
        private:
            TokenKind kind;//token种类
            std::string_view text;//token本身的文本,reserved和num就是他们本身，end为空
            long value;//存num的值，其他类型默认为0
            std::size_t offset =0 ;//这个标识tokenize后的位置，比如"12+ 34",内容为34的token的offset为3（空格被tokenize处理掉了，从0开始）
        public:
            Token(TokenKind k,std::string_view t,long v,std::size_t off)
                : kind(k),text(t),value(v),offset(off){}
            TokenKind get_kind()const {return kind;}
            std::string_view get_text() const{return text;}
            long get_value() const {return value;}
            std::size_t get_offset() const { return offset; }
            std::size_t get_len()const {return text.size();}//token的text的长度
    };
    class Tokenizer{
        private:
            std::string_view source_code_;//保留输入的src
            std::size_t position_{0};//当前tokenize进行到的位置
            std::size_t token_offset_{0};
            //跳过空白
            void skip_space(){
                while(position_<source_code_.size() && std::isspace(static_cast<unsigned char>(source_code_[position_]))){
                    position_++;
                }
            }
            Token tokenize_number(){
                const auto start_position = position_;
                while(position_<source_code_.size() && std::isdigit(static_cast<unsigned char>(source_code_[position_]))){
                    position_++;
                }//循环后position_被更新到下一个不是数字的字符处
                auto number_string = source_code_.substr(start_position,position_-start_position);//获取数字字符串
                long number_value = std::stol(static_cast<std::string>(number_string));//获取数值
                Token token(TokenKind::Number,number_string,number_value,token_offset_);//新建token
                token_offset_+=token.get_len();//offset向后移动token的len长度
                return token;
            }
            Token tokenize_plus_and_sub (){
                Token token(TokenKind::Reserved,source_code_.substr(position_,1),0,token_offset_);
                position_++;
                token_offset_++;//offset向后移动一个长度
                return token;
            }
        public:
            explicit Tokenizer(std::string_view src): source_code_(src){}
            //把输入的文本tokenize
            std::vector<Token> run(){
                std::vector<Token> token_list ;
                //循环处理放入list中
                while(position_<source_code_.size()){
                    //处理空白
                    skip_space();
                    if(position_>=source_code_.size()){
                        break;
                    }
                    const char current_char =source_code_[position_];
                    //处理数字
                    if(isdigit(static_cast<unsigned char>(current_char))){
                        token_list.push_back(tokenize_number());
                        continue;
                    }
                    //处理加减
                    if(current_char=='+' || current_char=='-'){
                        token_list.push_back(tokenize_plus_and_sub());
                        continue;
                    }
                    //其他情况的处理
                    std::cerr<<"tokenize error: invalid char:"<<current_char<<std::endl ;
                    position_++;
                }
                //在结尾加end
                token_list.emplace_back(TokenKind::End, std::string_view{}, 0, token_offset_);
                return token_list;
            }
            //调试用的dump函数
            void dump_token_list(const std::vector<Token>&  token_list){
                std::cout<<"token list :"<<std::endl;
                for (const auto & token:token_list){
                    std::cout <<token.get_offset()<<":\t"<<token.get_text()<<"\t";
                    switch (token.get_kind()) {
                        case TokenKind::Reserved:
                            std::cout << "Reserved\t";
                            break;
                        case TokenKind::Number:
                            std::cout << "Number\t\t";
                            break;
                        case TokenKind::End:
                            std::cout << "End\t\t";
                            break;
                        }
                    std::cout <<"len:"<<token.get_len()<<'\t'<<"value:"<<token.get_value()<<'\t'<<std::endl;          
                }
            }
    };
    class Parser {
        private:
            std::vector<Token> token_list_ ;
            int position_{0};
        public:
            explicit Parser(std::vector<Token>token_list):token_list_(std::move(token_list)){}
            //对保留字有两种操作，一种是expect，表示下一个必须是这个，另一种是consume，尝试处理这个符号，如果不是这个符号则返回
            //例子:我们对num +|— num这样的表达式，处理到中间的符号时一般consume +，else expect -
            bool is_reserved (char op ){
                return token_list_[position_].get_kind()==TokenKind::Reserved
                    && token_list_[position_].get_text()[0]==op
                    && token_list_[position_].get_text().size()==1;
            }
            bool consume (char op ){
                if(is_reserved(op)){
                    position_++;
                    return true;
                }else{
                    return false;
                }
            }
            bool expect (char op ){
                if(is_reserved(op)){
                    position_++;
                    return true;
                }else{
                    return false;//TODO:这里应该是报错，后续我会为编译器添加清晰的报错功能
                                //暂时这么处理，这样expect和consume暂时是一样的，但是他们在main函数中的后续处理以及抽象的含义是不同的
                } 
            }
            //当前的逻辑只需要写一个expect number就可以
            long expect_number(){
                if(token_list_[position_].get_kind()!=TokenKind::Number){
                    std::cerr<<"parse error: expect a number";//TODO:这里应该是报错，后续补充
                }
                return token_list_[position_++].get_value();//++这个运算还是很有用的
            }
            //处理字符的循环需要一个判断是否是end的函数
            bool at_end (){
                return token_list_[position_].get_kind()== TokenKind::End;
            }
    };
}//namespace of calc
int main( int argc ,char* argv[]){
    //处理错误的参数数量
    if(argc != 2 ){
        std::cerr<<"wrong number of arguments\n";
        return EXIT_FAILURE;
    }
    //接收输入
    const std::string_view source_code{argv[1]};
    //tokenize
    calc::Tokenizer Tokenizer(source_code);
    auto token_list =Tokenizer.run();
    // //调试用，dump token
    // Tokenizer.dump_token_list(token_list);
    //parse
    //内部逻辑：先expect一个数字，之后进入只要不是end的循环，先consume+后expect-，然后expect一个数字
    calc::Parser parser(std::move(token_list));
    std::cout<<".intel_syntax noprefix\n"
                ".globl main\n"
                "main:\n";
    std::cout<<"    mov rax,"<<parser.expect_number()<<'\n';
    while(!parser.at_end()){
            if(parser.consume('+')){
                std::cout<<"    add rax,"<<parser.expect_number()<<'\n';
                continue;
            }
            parser.expect('-');
            std::cout<<"    sub rax,"<<parser.expect_number()<<'\n';
        }
        std::cout<<"    ret"<<'\n';
    return EXIT_SUCCESS;
}
