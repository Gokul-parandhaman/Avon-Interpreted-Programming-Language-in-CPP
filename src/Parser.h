#pragma once
#include "Lexer.h"
#include "AST.h"
#include <memory>
#include <stdexcept>

class Parser {
public:
    explicit Parser(std::vector<Token> tokens);
    std::unique_ptr<ProgramNode> parse();

private:
    std::vector<Token> tokens_;
    size_t pos_;

    const Token& current() const;
    const Token& peek(int offset = 1) const;
    Token advance();
    Token expect(TokenType type, const std::string& msg = "");
    bool match(TokenType type);
    void skipNewlines();
    void expectEndOfStatement();
    NodePtr parseTypeDeclaration();   
    bool isTypeKeyword(TokenType tt); 
    
    NodePtr parseStatement();
    NodePtr parseVarDecl();
    NodePtr parseAssign();
    NodePtr parseFunDecl();
    NodePtr parseIf();
    NodePtr parseLoop();
    NodePtr parseWhile();
    NodePtr parsePrint();
    NodePtr parseReturn();
    NodePtr parseTryCatch();
    NodePtr parseThrow();
    NodePtr parseBreakContinue();
    NodeList parseBlock();
    NodePtr parseInitializerList(const std::string& typeName = "");
    NodePtr parseExpression();
    NodePtr parseOr();
    NodePtr parseAnd();
    NodePtr parseNot();
    NodePtr parseComparison();
    NodePtr parseAddition();
    NodePtr parseMultiplication();
    NodePtr parsePower();
    NodePtr parseUnary();
    NodePtr parseCall();
    NodePtr parsePrimary();
};