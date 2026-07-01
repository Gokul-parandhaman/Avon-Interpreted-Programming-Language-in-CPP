#include "Parser.h"
#include <sstream>

static std::runtime_error parseErr(const std::string& msg, int line) {
    return std::runtime_error("[Avon ParseError line " + std::to_string(line) + "] " + msg);
}

Parser::Parser(std::vector<Token> tokens) : tokens_(std::move(tokens)), pos_(0) {}

const Token& Parser::current() const { return tokens_[pos_]; }
const Token& Parser::peek(int offset) const {
    size_t idx = pos_ + offset;
    return (idx < tokens_.size()) ? tokens_[idx] : tokens_.back();
}
Token Parser::advance() {
    Token t = tokens_[pos_];
    if (t.type != TokenType::EOF_) ++pos_;
    return t;
}
Token Parser::expect(TokenType type, const std::string& msg) {
    if (current().type != type) {
        std::string err = msg.empty() ? "Expected token but got '" + current().lexeme + "'" : msg;
        throw parseErr(err, current().line);
    }
    return advance();
}
bool Parser::match(TokenType type) {
    if (current().type == type) { advance(); return true; }
    return false;
}
void Parser::skipNewlines() {
    while (current().type == TokenType::NEWLINE) advance();
}
void Parser::expectEndOfStatement() {
    auto tt = current().type;
    if (tt == TokenType::SEMICOLON) { advance(); return; }
    if (tt != TokenType::NEWLINE && tt != TokenType::EOF_ && tt != TokenType::RBRACE)
        throw parseErr("Expected end of statement, got '" + current().lexeme + "'", current().line);
}

bool Parser::isTypeKeyword(TokenType tt) {
    return tt == TokenType::NUMBER_TYPE || tt == TokenType::STRING_TYPE ||
           tt == TokenType::BOOL_TYPE || tt == TokenType::NULL_TYPE ||
           tt == TokenType::BUNCH_TYPE || tt == TokenType::MAP_TYPE ||
           tt == TokenType::STACK_TYPE || tt == TokenType::QUEUE_TYPE ||
           tt == TokenType::HEAP_TYPE || tt == TokenType::LINKEDLIST_TYPE;
}

std::unique_ptr<ProgramNode> Parser::parse() {
    NodeList stmts;
    skipNewlines();
    while (current().type != TokenType::EOF_) {
        stmts.push_back(parseStatement());
        skipNewlines();
    }
    return std::make_unique<ProgramNode>(std::move(stmts));
}

NodePtr Parser::parseStatement() {
    skipNewlines();
    auto tt = current().type;
    
    if (tt == TokenType::PRINT) return parsePrint();
    if (tt == TokenType::DEBUG) {
        advance();
        auto printNode = parsePrint();
        if (auto* print = dynamic_cast<PrintNode*>(printNode.get())) print->isDebug = true;
        return printNode;
    }
    
    if (isTypeKeyword(tt)) return parseTypeDeclaration();
    if (tt == TokenType::VAR) return parseVarDecl();
    if (tt == TokenType::FUN) return parseFunDecl();
    if (tt == TokenType::IF) return parseIf();
    if (tt == TokenType::LOOP) return parseLoop();
    if (tt == TokenType::RETURN) return parseReturn();
    if (tt == TokenType::TRY) return parseTryCatch();
    if (tt == TokenType::THROW) return parseThrow();
    if (tt == TokenType::BREAK || tt == TokenType::CONTINUE) return parseBreakContinue();

    if (tt == TokenType::IDENT && peek().type == TokenType::EQ) return parseAssign();

    return parseExpression();
}

NodePtr Parser::parseTypeDeclaration() {
    std::string typeName = current().lexeme;
    advance();
    std::string name = expect(TokenType::IDENT, "Expected variable name").lexeme;
    NodePtr initializer = nullptr;

    if (current().type == TokenType::EQ) {
        advance();
        initializer = (current().type == TokenType::LBRACE) ? parseInitializerList(typeName) : parseExpression();
    } else if (current().type == TokenType::LBRACE) {
        initializer = parseInitializerList(typeName);
    } else {
        if (typeName == "number" || typeName == "NUMBER_TYPE") initializer = std::make_unique<NumberNode>(0.0);
        else if (typeName == "string" || typeName == "STRING_TYPE") initializer = std::make_unique<StringNode>("");
        else if (typeName == "bool" || typeName == "BOOL_TYPE") initializer = std::make_unique<BoolNode>(false);
        else if (typeName == "bunch" || typeName == "BUNCH_TYPE") initializer = std::make_unique<CallNode>("bunch", NodeList());
        else if (typeName == "map" || typeName == "MAP_TYPE") initializer = std::make_unique<CallNode>("map", NodeList());
        else if (typeName == "stack" || typeName == "STACK_TYPE") initializer = std::make_unique<CallNode>("stack", NodeList());
        else if (typeName == "queue" || typeName == "QUEUE_TYPE") initializer = std::make_unique<CallNode>("queue", NodeList());
        else if (typeName == "heap" || typeName == "HEAP_TYPE") initializer = std::make_unique<CallNode>("heap", NodeList());
        else if (typeName == "linkedlist" || typeName == "LINKEDLIST_TYPE") initializer = std::make_unique<CallNode>("linkedlist", NodeList());
        else initializer = std::make_unique<NullNode>();
    }

    expectEndOfStatement();
    return std::make_unique<VarDeclNode>(name, typeName, std::move(initializer));
}

NodePtr Parser::parseVarDecl() {
    advance();
    std::string name = expect(TokenType::IDENT, "Expected variable name").lexeme;
    expect(TokenType::EQ, "Expected '='");
    auto value = parseExpression();
    expectEndOfStatement();
    return std::make_unique<VarDeclNode>(std::move(name), std::move(value));
}

NodePtr Parser::parseAssign() {
    std::string name = advance().lexeme;
    expect(TokenType::EQ, "Expected '='");
    auto value = parseExpression();
    expectEndOfStatement();
    return std::make_unique<AssignNode>(std::move(name), std::move(value));
}

NodePtr Parser::parseFunDecl() {
    advance();
    std::string name = expect(TokenType::IDENT, "Expected function name").lexeme;
    expect(TokenType::LPAREN, "Expected '('");

    std::vector<std::string> params;
    if (current().type != TokenType::RPAREN) {
        params.push_back(expect(TokenType::IDENT, "Expected parameter name").lexeme);
        while (current().type == TokenType::COMMA) {
            advance();
            params.push_back(expect(TokenType::IDENT, "Expected parameter name").lexeme);
        }
    }
    expect(TokenType::RPAREN, "Expected ')'");
    auto body = parseBlock();
    return std::make_unique<FunDeclNode>(std::move(name), std::move(params), std::move(body));
}

NodePtr Parser::parseIf() {
    advance();
    auto condition = parseExpression();
    skipNewlines();
    auto body = parseBlock();
    
    NodeList elseBody;
    skipNewlines();
    
    while (current().type == TokenType::ELSE) {
        advance();
        if (current().type == TokenType::IF) {
            NodeList block;
            block.push_back(parseIf());
            elseBody = std::move(block);
            break;
        } else {
            skipNewlines();
            elseBody = parseBlock();
            break;
        }
    }
    
    return std::make_unique<IfNode>(std::move(condition), std::move(body), std::move(elseBody));
}

NodePtr Parser::parseLoop() {
    advance();
    expect(TokenType::LPAREN, "Expected '('");

    NodeList init;
    if (current().type != TokenType::SEMICOLON) {
        auto expr = parseExpression();
        if (auto* assign = dynamic_cast<AssignNode*>(expr.get())) {
            init.push_back(std::make_unique<VarDeclNode>(assign->name, std::move(assign->value)));
        } else {
            init.push_back(std::move(expr));
        }
    }
    expect(TokenType::SEMICOLON, "Expected ';'");

    NodePtr condition = nullptr;
    if (current().type != TokenType::SEMICOLON) condition = parseExpression();
    expect(TokenType::SEMICOLON, "Expected ';'");

    NodeList update;
    if (current().type != TokenType::RPAREN) update.push_back(parseExpression());
    expect(TokenType::RPAREN, "Expected ')'");

    skipNewlines();
    return std::make_unique<LoopNode>(std::move(init), std::move(condition), std::move(update), parseBlock());
}

NodePtr Parser::parseWhile() {
    advance();
    auto condition = parseExpression();
    skipNewlines();
    return std::make_unique<WhileNode>(std::move(condition), parseBlock());
}

NodePtr Parser::parsePrint() {
    if (current().type != TokenType::PRINT && current().type != TokenType::DEBUG)
        throw parseErr("Expected 'print' or 'debug'", current().line);
    
    bool isDebug = (current().type == TokenType::DEBUG);
    advance();
    expect(TokenType::LPAREN, "Expected '('");

    NodeList args;
    NodePtr end = nullptr, sep = nullptr;

    while (current().type != TokenType::RPAREN && current().type != TokenType::EOF_) {
        if (current().type == TokenType::SEP && current().lexeme == "sep") {
            if (pos_ + 1 < tokens_.size() && tokens_[pos_ + 1].type == TokenType::EQ) {
                advance(); advance();
                sep = parseExpression();
                if (current().type == TokenType::COMMA) advance();
                continue;
            }
        }
        
        if (current().type == TokenType::END && current().lexeme == "end") {
            if (pos_ + 1 < tokens_.size() && tokens_[pos_ + 1].type == TokenType::EQ) {
                advance(); advance();
                end = parseExpression();
                if (current().type == TokenType::COMMA) advance();
                continue;
            }
        }
        
        args.push_back(parseExpression());
        if (current().type == TokenType::COMMA) advance();
    }

    expect(TokenType::RPAREN, "Expected ')'");
    expectEndOfStatement();
    return std::make_unique<PrintNode>(std::move(args), std::move(end), std::move(sep), isDebug);
}

NodePtr Parser::parseReturn() {
    advance();
    if (current().type == TokenType::NEWLINE || current().type == TokenType::RBRACE || current().type == TokenType::EOF_) {
        expectEndOfStatement();
        return std::make_unique<ReturnNode>(nullptr);
    }
    auto value = parseExpression();
    expectEndOfStatement();
    return std::make_unique<ReturnNode>(std::move(value));
}

NodePtr Parser::parseTryCatch() {
    advance();
    auto tryBlock = parseBlock();

    std::string catchVar;
    NodeList catchBlock, finallyBlock;

    skipNewlines();
    if (current().type == TokenType::CATCH) {
        advance();
        expect(TokenType::LPAREN, "Expected '('");
        catchVar = expect(TokenType::IDENT, "Expected error variable").lexeme;
        expect(TokenType::RPAREN, "Expected ')'");
        catchBlock = parseBlock();
    }

    skipNewlines();
    if (current().type == TokenType::FINALLY) {
        advance();
        finallyBlock = parseBlock();
    }

    return std::make_unique<TryCatchNode>(std::move(tryBlock), std::move(catchVar), std::move(catchBlock), std::move(finallyBlock));
}

NodePtr Parser::parseThrow() {
    advance();
    auto value = parseExpression();
    expectEndOfStatement();
    return std::make_unique<ThrowNode>(std::move(value));
}

NodePtr Parser::parseBreakContinue() {
    bool isBreak = (current().type == TokenType::BREAK);
    advance();
    std::string label;
    if (current().type == TokenType::IDENT) {
        label = current().lexeme;
        advance();
    }
    expectEndOfStatement();
    return isBreak ? static_cast<NodePtr>(std::make_unique<BreakNode>(std::move(label))) 
                   : static_cast<NodePtr>(std::make_unique<ContinueNode>(std::move(label)));
}

NodeList Parser::parseBlock() {
    skipNewlines();
    expect(TokenType::LBRACE, "Expected '{'");
    skipNewlines();

    NodeList stmts;
    while (current().type != TokenType::RBRACE && current().type != TokenType::EOF_) {
        stmts.push_back(parseStatement());
        skipNewlines();
    }
    expect(TokenType::RBRACE, "Expected '}'");
    return stmts;
}

NodePtr Parser::parseInitializerList(const std::string& typeName) {
    expect(TokenType::LBRACE, "Expected '{'");
    NodeList elements;
    
    if (current().type != TokenType::RBRACE) {
        elements.push_back(parseExpression());
        while (current().type == TokenType::COMMA) {
            advance();
            if (current().type == TokenType::RBRACE) break;
            elements.push_back(parseExpression());
        }
    }
    expect(TokenType::RBRACE, "Expected '}'");
    
    std::string callName = !typeName.empty() ? typeName : "bunch";
    if (callName == "map" || callName == "MAP_TYPE") return std::make_unique<CallNode>("map", NodeList());
    return std::make_unique<CallNode>(callName, std::move(elements));
}

NodePtr Parser::parseExpression() {
    if (current().type == TokenType::IDENT && peek().type == TokenType::EQ) {
        std::string name = current().lexeme;
        advance(); advance();
        return std::make_unique<AssignNode>(std::move(name), parseExpression());
    }
    return parseOr();
}

NodePtr Parser::parseOr() {
    auto left = parseAnd();
    while (current().type == TokenType::OR) {
        advance();
        left = std::make_unique<BinOpNode>(std::move(left), "or", parseAnd());
    }
    return left;
}

NodePtr Parser::parseAnd() {
    auto left = parseNot();
    while (current().type == TokenType::AND) {
        advance();
        left = std::make_unique<BinOpNode>(std::move(left), "and", parseNot());
    }
    return left;
}

NodePtr Parser::parseNot() {
    if (current().type == TokenType::NOT) {
        advance();
        return std::make_unique<UnaryOpNode>("not", parseNot());
    }
    return parseComparison();
}

NodePtr Parser::parseComparison() {
    auto left = parseAddition();
    while (true) {
        std::string op;
        switch (current().type) {
            case TokenType::EQEQ: op = "=="; break;
            case TokenType::NEQ: op = "!="; break;
            case TokenType::LT: op = "<"; break;
            case TokenType::GT: op = ">"; break;
            case TokenType::LTE: op = "<="; break;
            case TokenType::GTE: op = ">="; break;
            default: return left;
        }
        advance();
        left = std::make_unique<BinOpNode>(std::move(left), op, parseAddition());
    }
}

NodePtr Parser::parseAddition() {
    auto left = parseMultiplication();
    while (current().type == TokenType::PLUS || current().type == TokenType::MINUS) {
        std::string op = (current().type == TokenType::PLUS) ? "+" : "-";
        advance();
        left = std::make_unique<BinOpNode>(std::move(left), op, parseMultiplication());
    }
    return left;
}

NodePtr Parser::parseMultiplication() {
    auto left = parsePower();  
    while (true) {
        std::string op;
        switch (current().type) {
            case TokenType::STAR: op = "*"; break;
            case TokenType::SLASH: op = "/"; break;
            case TokenType::PERCENT: op = "%"; break;
            default: return left;
        }
        advance();
        left = std::make_unique<BinOpNode>(std::move(left), op, parsePower());
    }
}

NodePtr Parser::parsePower() {
    auto base = parseUnary();
    if (current().type == TokenType::POWER) {
        advance();
        return std::make_unique<BinOpNode>(std::move(base), "**", parsePower());
    }
    return base;
}

NodePtr Parser::parseUnary() {
    if (current().type == TokenType::MINUS) {
        advance();
        return std::make_unique<UnaryOpNode>("-", parseUnary());
    }
    return parseCall();
}

NodePtr Parser::parseCall() {
    auto expr = parsePrimary();
    while (current().type == TokenType::LPAREN) {
        auto* ident = dynamic_cast<IdentNode*>(expr.get());
        if (!ident) throw parseErr("Can only call named functions", current().line);

        std::string name = ident->name;
        if (name == "print" || name == "debug") throw parseErr("print should be handled by parseStatement()", current().line);
        
        advance();
        NodeList args;
        if (current().type != TokenType::RPAREN) {
            args.push_back(parseExpression());
            while (current().type == TokenType::COMMA) {
                advance();
                args.push_back(parseExpression());
            }
        }
        expect(TokenType::RPAREN, "Expected ')'");
        expr = std::make_unique<CallNode>(std::move(name), std::move(args));
    }
    return expr;
}

NodePtr Parser::parsePrimary() {
    const Token& tok = current();
    NodePtr expr;

    if (tok.type == TokenType::NUMBER) {
        advance(); expr = std::make_unique<NumberNode>(tok.numVal);
    } else if (tok.type == TokenType::STRING) {
        advance(); expr = std::make_unique<StringNode>(tok.lexeme);
    } else if (tok.type == TokenType::BOOL) {
        advance(); expr = std::make_unique<BoolNode>(tok.boolVal);
    } else if (tok.type == TokenType::NULL_TYPE || tok.type == TokenType::NULLVAL) {
        advance(); expr = std::make_unique<NullNode>();
    } else if (tok.type == TokenType::IDENT) {
        advance(); expr = std::make_unique<IdentNode>(tok.lexeme);
    } else if (tok.type == TokenType::LPAREN) {
        advance();
        expr = parseExpression();
        expect(TokenType::RPAREN, "Expected ')'");
    } else if (tok.type == TokenType::LBRACE) {
        expr = parseInitializerList("");
    } else {
        throw parseErr("Unexpected token '" + tok.lexeme + "' in expression", tok.line);
    }

    while (current().type == TokenType::LBRACKET) {
        advance();
        bool isSlice = false;
        size_t save_pos = pos_;
        
        try {
            if (current().type == TokenType::COLON) isSlice = true;
            else {
                if (current().type != TokenType::RBRACKET) parseExpression();
                if (current().type == TokenType::COLON) isSlice = true;
            }
            pos_ = save_pos;
        } catch (...) { pos_ = save_pos; }

        if (isSlice) {
            NodePtr start = nullptr, end = nullptr, step = nullptr;
            if (current().type != TokenType::COLON) start = parseExpression();
            expect(TokenType::COLON, "Expected ':'");
            if (current().type != TokenType::COLON && current().type != TokenType::RBRACKET) end = parseExpression();
            
            if (current().type == TokenType::COLON) {
                advance();
                step = (current().type != TokenType::RBRACKET) ? parseExpression() : std::make_unique<NumberNode>(1.0);
            } else step = std::make_unique<NumberNode>(1.0);

            expect(TokenType::RBRACKET, "Expected ']'");
            if (!start) start = std::make_unique<NumberNode>(0.0);
            expr = std::make_unique<SubpartNode>(std::move(expr), std::move(start), std::move(end), std::move(step));
        } else {
            auto index = parseExpression();
            expect(TokenType::RBRACKET, "Expected ']'");
            expr = std::make_unique<IndexAccessNode>(std::move(expr), std::move(index));
        }
    }
    return expr;
}