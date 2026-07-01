#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <stdexcept>

enum class TokenType {
    NUMBER, STRING, BOOL, NULLVAL,
    IDENT, VAR, FUN, RETURN, IF, ELSE, LOOP, FROM, TO, PRINT,
    AND, OR, NOT,
    PLUS, MINUS, STAR, SLASH, PERCENT, POWER,
    EQEQ, NEQ, LT, GT, LTE, GTE, EQ,
    LPAREN, RPAREN, LBRACE, RBRACE, COMMA, DOT,
    NEWLINE, EOF_,
    BREAK, CONTINUE, TRY, CATCH, THROW, FINALLY,
    SEMICOLON, COLON, LBRACKET, RBRACKET,
    END, SEP, DEBUG, IN, STEP, WHERE,  
    NUMBER_TYPE, STRING_TYPE, BOOL_TYPE, NULL_TYPE,
    BUNCH_TYPE, MAP_TYPE, STACK_TYPE, QUEUE_TYPE,
    HEAP_TYPE, LINKEDLIST_TYPE
};

struct Token {
    TokenType type;
    std::string lexeme;
    double numVal;
    bool boolVal;
    int line;
    
    Token(TokenType t, std::string lex, int ln) : type(t), lexeme(std::move(lex)), numVal(0), boolVal(false), line(ln) {}
    Token(TokenType t, double num, int ln) : type(t), lexeme(""), numVal(num), boolVal(false), line(ln) {}
    Token(TokenType t, bool b, int ln) : type(t), lexeme(""), numVal(0), boolVal(b), line(ln) {}
};

class Lexer {
public:
    explicit Lexer(std::string source);
    std::vector<Token> tokenize();

private:
    std::string source_;
    size_t pos_;
    int line_;
    static const std::unordered_map<std::string, TokenType> keywords_;
    
    char current() const;
    char peek(int offset = 1) const;
    char advance();
    Token readNumber();
    Token readString();
    Token readIdentOrKeyword();
};