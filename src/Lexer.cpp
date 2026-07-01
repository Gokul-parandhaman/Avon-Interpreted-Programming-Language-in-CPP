#include "Lexer.h"
#include <cctype>
#include <sstream>
#include <iostream>

const std::unordered_map<std::string, TokenType> Lexer::keywords_ = {
    {"var", TokenType::VAR}, {"fun", TokenType::FUN}, {"return", TokenType::RETURN},
    {"if", TokenType::IF}, {"else", TokenType::ELSE}, {"loop", TokenType::LOOP},
    {"from", TokenType::FROM}, {"to", TokenType::TO}, {"print", TokenType::PRINT},
    {"true", TokenType::BOOL}, {"false", TokenType::BOOL}, {"null", TokenType::NULL_TYPE},
    {"and", TokenType::AND}, {"or", TokenType::OR}, {"not", TokenType::NOT},
    {"break", TokenType::BREAK}, {"continue", TokenType::CONTINUE},
    {"try", TokenType::TRY}, {"catch", TokenType::CATCH}, {"throw", TokenType::THROW},
    {"finally", TokenType::FINALLY}, {"in", TokenType::IN}, {"step", TokenType::STEP},
    {"where", TokenType::WHERE}, {"debug", TokenType::DEBUG},
    {"end", TokenType::END}, {"sep", TokenType::SEP},
    {"number", TokenType::NUMBER_TYPE}, {"string", TokenType::STRING_TYPE},
    {"bool", TokenType::BOOL_TYPE}, {"bunch", TokenType::BUNCH_TYPE},
    {"map", TokenType::MAP_TYPE}, {"stack", TokenType::STACK_TYPE},
    {"queue", TokenType::QUEUE_TYPE}, {"heap", TokenType::HEAP_TYPE},
    {"linkedlist", TokenType::LINKEDLIST_TYPE},
};

Lexer::Lexer(std::string source) : source_(std::move(source)), pos_(0), line_(1) {}

char Lexer::current() const { return (pos_ < source_.size()) ? source_[pos_] : '\0'; }

char Lexer::peek(int offset) const {
    size_t idx = pos_ + offset;
    return (idx < source_.size()) ? source_[idx] : '\0';
}

char Lexer::advance() {
    char ch = source_[pos_++];
    if (ch == '\n') ++line_;
    return ch;
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;

    while (current() != '\0') {
        char ch = current();

        if (ch == ' ' || ch == '\t' || ch == '\r') {
            advance();
            continue;
        }

        if (ch == '#') {
            while (current() != '\n' && current() != '\0') advance();
            continue;
        }

        if (ch == '\n') {
            if (!tokens.empty() && tokens.back().type != TokenType::NEWLINE)
                tokens.emplace_back(TokenType::NEWLINE, "\n", line_);
            advance();
            continue;
        }

        if (std::isdigit(ch)) {
            tokens.push_back(readNumber());
            continue;
        }

        if (ch == '"') {
            tokens.push_back(readString());
            continue;
        }

        if (std::isalpha(ch) || ch == '_') {
            tokens.push_back(readIdentOrKeyword());
            continue;
        }

        if (ch == '*' && peek() == '*') { advance(); advance(); tokens.emplace_back(TokenType::POWER, "**", line_); continue; }
        if (ch == '=' && peek() == '=') { advance(); advance(); tokens.emplace_back(TokenType::EQEQ, "==", line_); continue; }
        if (ch == '!' && peek() == '=') { advance(); advance(); tokens.emplace_back(TokenType::NEQ, "!=", line_); continue; }
        if (ch == '<' && peek() == '=') { advance(); advance(); tokens.emplace_back(TokenType::LTE, "<=", line_); continue; }
        if (ch == '>' && peek() == '=') { advance(); advance(); tokens.emplace_back(TokenType::GTE, ">=", line_); continue; }
        if (ch == '.' && peek() == '.') { advance(); advance(); tokens.emplace_back(TokenType::COLON, ":", line_); tokens.emplace_back(TokenType::COLON, ":", line_); continue; }

        TokenType tt;
        switch (ch) {
            case '.': tt = TokenType::DOT; break;
            case '+': tt = TokenType::PLUS; break;
            case '-': tt = TokenType::MINUS; break;
            case '*': tt = TokenType::STAR; break;
            case '/': tt = TokenType::SLASH; break;
            case '%': tt = TokenType::PERCENT; break;
            case '=': tt = TokenType::EQ; break;
            case '<': tt = TokenType::LT; break;
            case '>': tt = TokenType::GT; break;
            case '(': tt = TokenType::LPAREN; break;
            case ')': tt = TokenType::RPAREN; break;
            case '{': tt = TokenType::LBRACE; break;
            case '}': tt = TokenType::RBRACE; break;
            case ',': tt = TokenType::COMMA; break;
            case ';': tt = TokenType::SEMICOLON; break;
            case ':': tt = TokenType::COLON; break;
            case '[': tt = TokenType::LBRACKET; break;
            case ']': tt = TokenType::RBRACKET; break;
            default:
                throw std::runtime_error("[Avon SyntaxError line " + std::to_string(line_) + "] Unexpected character: '" + ch + "'");
        }
        advance();
        tokens.emplace_back(tt, std::string(1, ch), line_);
    }

    tokens.emplace_back(TokenType::EOF_, "", line_);
    return tokens;
}

Token Lexer::readNumber() {
    std::string num;
    bool hasDot = false;

    while (std::isdigit(current()) || current() == '.') {
        if (current() == '.') {
            if (hasDot) throw std::runtime_error("[Avon SyntaxError line " + std::to_string(line_) + "] Invalid number literal");
            hasDot = true;
        }
        num += advance();
    }
    return Token(TokenType::NUMBER, std::stod(num), line_);
}

Token Lexer::readString() {
    advance();
    std::string str;
    int startLine = line_;
    
    while (true) {
        char ch = current();
        if (ch == '\0') throw std::runtime_error("[Avon SyntaxError line " + std::to_string(startLine) + "] Unterminated string");
        if (ch == '"') {
            advance();
            return Token(TokenType::STRING, str, startLine);
        }
        if (ch == '\n') {
            line_++;
            throw std::runtime_error("[Avon SyntaxError line " + std::to_string(line_) + "] Unterminated string");
        }
        if (ch == '\\') {
            advance();
            char next = current();
            switch (next) {
                case 'n': str += '\n'; break;
                case 't': str += '\t'; break;
                case '"': str += '"'; break;
                case '\\': str += '\\'; break;
                default: throw std::runtime_error("[Avon SyntaxError line " + std::to_string(line_) + "] Unknown escape: \\" + std::string(1, next));
            }
            advance();
        } else {
            str += ch;
            advance();
        }
    }
}

Token Lexer::readIdentOrKeyword() {
    std::string word;
    while (std::isalnum(current()) || current() == '_') word += advance();

    auto it = keywords_.find(word);
    if (it != keywords_.end()) {
        TokenType tt = it->second;
        if (tt == TokenType::BOOL) return Token(TokenType::BOOL, word == "true", line_);
        if (tt == TokenType::NULLVAL) return Token(TokenType::NULLVAL, word, line_);
        return Token(tt, word, line_);
    }
    return Token(TokenType::IDENT, word, line_);
}