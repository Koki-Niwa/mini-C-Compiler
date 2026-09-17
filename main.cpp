#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>
#include <charconv>
#include <cctype>
#include <string_view>
#include <stdexcept>
#include <cstring>
#include <utility>
#include <iomanip>

namespace calc{
    enum class TokenKind{
        Reserved,
        Number,
        End,
    };
    struct Token{
        TokenKind kind;
        std::string_view text;
        long value =0;
        std::size_t offset =0 ;//用于记录每个token的起始位置，用于报错
    };
    class CompileError :public std::runtime_error{
        private: 
            std::size_t offset_=0;
            std::size_t len_=0;
        public: 
            CompileError(const std::string& message,std::size_t offset,std::size_t len):
                std::runtime_error(message),offset_(offset),len_(len){}
            std::size_t offset()const{return offset_;}
            std::size_t len()const{return len_;}
    };
    //修改打印错误信息，包含offset和len
    [[noreturn]]inline void error(const std::string& message,std::size_t offset,std::size_t len){
        throw CompileError{message,offset,len};
    }
    //调试开关：CALC_DEBUG 未设置、为空串、或正好等于 "0" 时关闭，其它值一律打开
    inline bool debug_enabled(){
        const char* v =std::getenv("CALC_DEBUG");
        return v !=nullptr && v[0] !='\0' && std::strcmp(v,"0") !=0;
    }
    inline const char* kind_name(TokenKind k){
        switch(k){
            case TokenKind::Number:   return "Number";
            case TokenKind::Reserved: return "Reserved";
            case TokenKind::End:      return "End";
        }
        return "?";//故意不写 default：将来新增 TokenKind 时，编译器会在这里报未覆盖
    }
    class Tokenizer{
        public:
            explicit Tokenizer(std::string_view src,bool debug=false):src_(src),debug_(debug){}
            std::vector<Token> run(){
                std::vector<Token> tokens;
                if(debug_){
                    std::cerr<<"[CALC_DEBUG] tokens:\n";
                }
                std::size_t pos=0;
                while(pos<src_.size()){
                    const char c =src_[pos];
                    if(std::isspace(static_cast<unsigned char>(c))){
                        pos++;
                        continue;
                    }
                    if(c=='+' || c== '-'){
                        add(tokens,Token{TokenKind::Reserved,src_.substr(pos,1),0,pos});
                        pos++;
                        continue;
                    }
                    if(std::isdigit(static_cast<unsigned char>(c))){
                        const std::size_t begin = pos ;
                        const char*first=src_.data()+pos;
                        const char*last=src_.data()+src_.size();
                        long value =0 ;
                        const auto [ptr,ec]=std::from_chars(first,last,value,10);
                        pos=static_cast<size_t>(ptr-src_.data());
                        if(ec==std::errc::result_out_of_range){
                            error("number is too large",begin,pos-begin);
                        }
                        add(tokens,Token{TokenKind::Number,src_.substr(begin,pos-begin),value,begin});
                        continue;   
                    }   
                    error("Cannot tokenize",pos,1);
                }
                add(tokens,Token{TokenKind::End,src_.substr(pos),0,pos});
                return tokens;
            }
        private:
            //所有 token 唯一的入口：先按需打印，再入队
            void add(std::vector<Token>& out,Token t){
                if(debug_){
                    print_token(out.size(),t);
                }
                out.push_back(std::move(t));
            }
            void print_token(std::size_t index,const Token& t)const{
                //定宽列在前，变宽的 text 放最后 —— 这样既对齐又不影响流式打印
                std::cerr<<"  ["<<std::setw(3)<<std::right<<index<<"] "
                         <<std::setw(8)<<std::left<<kind_name(t.kind)
                         <<"  offset="<<std::setw(4)<<std::right<<t.offset
                         <<"  len="<<std::setw(3)<<t.text.size()
                         <<"  value="<<std::setw(6)<<t.value
                         <<"  text=["<<t.text<<"]\n";
            }
            std::string_view src_;
            bool debug_ =false;
    };
    inline std::vector<Token> tokenize(std::string_view src,bool debug=false){
        return Tokenizer{src,debug}.run();
    }
    class Parser{
        private:
            std::vector<Token> tokens_;
            std::size_t pos_ =0 ;
            bool is_reserved(char op)const{
                const Token & t =tokens_[pos_];
                return t.kind == TokenKind::Reserved
                        && t.text.size()==1
                        && t.text[0]==op;
            }
        public:
            explicit Parser(std::vector<Token>tokens) : tokens_(std::move(tokens)){}
            bool consume(char op){
                if(!is_reserved(op)){
                    return false;
                }
                pos_++;
                return true;
            }
            void expect(char op){
                if(!is_reserved(op)){
                    error("expected '"+std::string(1,op)+"'",tokens_[pos_].offset,tokens_[pos_].text.size());
                }
                pos_++;
            }
            long expect_number(){
                if(tokens_[pos_].kind !=TokenKind::Number){
                    error("expected a number",tokens_[pos_].offset,tokens_[pos_].text.size());
                }
                return tokens_[pos_++].value;
            }
            bool at_end() const {
                return tokens_[pos_].kind == TokenKind::End;
            }
        };
} // namespace calc

int main( int argc ,char* argv[]){
    if(argc != 2 ){
        std::cerr<<"wrong number of arguments\n";
        return EXIT_FAILURE;
    }
    const bool debug =calc::debug_enabled();
    const std::string_view source{argv[1]};
    try{
        auto tokens =calc::tokenize(source,debug);

        calc::Parser parser{std::move(tokens)};
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

        }catch(const calc::CompileError& e){      
        std::cerr << "[offset=" << e.offset()
                  << " len=" << e.len() << "] "
                  << e.what() << '\n';        
        return EXIT_FAILURE;
    }catch(const std::exception&e){           
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
