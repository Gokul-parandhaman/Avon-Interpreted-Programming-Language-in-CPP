#pragma once
#include "../AST.h"
#include "BytecodeProgram.h"
#include <vector>
#include <stack>
#include "../Environment.h"

class BytecodeCompiler : public Visitor {
public:
    BytecodeCompiler();
    BytecodeProgram compile(ProgramNode& program);
    
    void visitNumber(NumberNode& n) override;
    void visitString(StringNode& n) override;
    void visitBool(BoolNode& n) override;
    void visitNull(NullNode& n) override;
    void visitBinOp(BinOpNode& n) override;
    void visitUnaryOp(UnaryOpNode& n) override;
    void visitIdent(IdentNode& n) override;
    void visitCall(CallNode& n) override;
    void visitIndexAccess(IndexAccessNode& n) override;
    void visitSubpart(SubpartNode& n) override;
    void visitProgram(ProgramNode& n) override;
    void visitVarDecl(VarDeclNode& n) override;
    void visitAssign(AssignNode& n) override;
    void visitFunDecl(FunDeclNode& n) override;
    void visitIf(IfNode& n) override;
    void visitLoop(LoopNode& n) override;
    void visitWhile(WhileNode& n) override;
    void visitPrint(PrintNode& n) override;
    void visitReturn(ReturnNode& n) override;
    void visitTryCatch(TryCatchNode& n) override;
    void visitThrow(ThrowNode& n) override;
    void visitBreak(BreakNode& n) override;
    void visitContinue(ContinueNode& n) override;

private:
    BytecodeProgram program;
    void compileExpression(Node& node);
    void compileStatement(Node& node);
    void compileBlock(const NodeList& block);
};