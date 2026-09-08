#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>
#include <charconv>

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
    };
    class CompileError :public std::runtime_error{
        using std::runtime_error::runtime_error;
    };
    [[noreturn]]inline void error(const std::string& message){
        throw CompileError{message};
    }
    class Tokenizer{
        public:
            explicit Tokenizer(std::string_view src):src_(src){}
            std::vector<Token> run(){
                std::vector<Token> tokens;
                std::size_t pos=0;
                while(pos<src_.size()){
                    const char c =src_[pos];
                    if(std::isspace(static_cast<unsigned char>(c))){
                        pos++;
                        continue;
                    }
                    if(c=='+' || c== '-'){
                        tokens.push_back({TokenKind::Reserved,src_.substr(pos,1),0});
                        pos++;
                        continue;
                    }
                    if(std::isdigit(static_cast<unsigned char>(c))){
                        const std::size_t begin = pos ;
                        const char*first=src_.data()+pos;
                        const char*last=src_.data()+src_.size();
                        long value =0 ;
                        const auto [ptr,ec]=std::from_chars(first,last,value,10);
                        if(ec==std::errc::result_out_of_range){
                            error("number is too large");
                        }
                        pos=static_cast<size_t>(ptr-src_.data());
                        tokens.push_back({TokenKind::Number,src_.substr(begin,pos-begin),value});
                        continue;   
                    }   
                    error("Cannot tokenize");
                }
                tokens.push_back({TokenKind::End,src_.substr(pos),0});
                return tokens;
            }
        private:
        std::string_view src_;
    };
    inline std::vector<Token> tokenize(std::string_view src){
        return Tokenizer{src}.run();
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
                    error("expected '"+std::string(1,op)+"'");
                }
                pos_++;
            }
            long expect_number(){
                if(tokens_[pos_].kind !=TokenKind::Number){
                    error("expected a number");
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
    try{
        const std::string_view source{argv[1]};
        auto tokens =calc::tokenize(source);
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
    }catch(const std::exception&e){
        std::cerr<<e.what()<<'\n';
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}