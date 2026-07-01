#include "BytecodeCompiler.h"

BytecodeCompiler::BytecodeCompiler() {}

BytecodeProgram BytecodeCompiler::compile(ProgramNode& program) {
    this->program.clear();  // ← Clear member variable
    //std::cout << "[COMPILER] Program cleared\n";
    
    // Clear all stacks 
    while (!break_stack.empty()) break_stack.pop();
    while (!continue_stack.empty()) continue_stack.pop();
    while (!loop_start_stack.empty()) loop_start_stack.pop();
    break_positions.clear();
    continue_positions.clear();
    
    program.accept(*this);
    this->program.emit(OpCode::HALT);
    return std::move(this->program);
}
void BytecodeCompiler::compileExpression(Node& node) {
    // ── If it's a StringNode, just push it ──────────────────────────────────
    if (auto* strNode = dynamic_cast<StringNode*>(&node)) {
        //std::cout << "[COMPILER] compileExpression: pushing string directly\n";
        program.emit(OpCode::PUSH, strNode->value);
        return;
    }
    
    //std::cout << "[COMPILER] compileExpression called for type: " << typeid(node).name() << "\n";
    node.accept(*this);
}

void BytecodeCompiler::compileStatement(Node& node) {
    //std::cout << "[COMPILER] compileStatement called for type: " << typeid(node).name() << "\n";
    node.accept(*this);
}

void BytecodeCompiler::compileBlock(const NodeList& block) {
    //std::cout << "[BLOCK] Compiling block with " << block.size() << " statements\n";
    for (size_t i = 0; i < block.size(); ++i) {
        //std::cout << "[BLOCK] Statement " << i << "\n";
        compileStatement(*block[i]);
    }
    //std::cout << "[BLOCK] Block compiled\n";
}
void BytecodeCompiler::visitNumber(NumberNode& node) {
    //std::cout << "[NUM] visitNumber: " << node.value << "\n";
    program.emit(OpCode::PUSH, node.value);
}

void BytecodeCompiler::visitIdent(IdentNode& node) {
    //std::cout << "[IDENT] visitIdent: " << node.name << "\n";
    program.emit(OpCode::LOAD, node.name);
}

void BytecodeCompiler::visitString(StringNode& node) {
    ////std::cout << "[STRING] visitString: " << node.value << "\n";
    program.emit(OpCode::PUSH, node.value);
}

void BytecodeCompiler::visitBool(BoolNode& node) {
    program.emit(OpCode::PUSH, node.value);
}

void BytecodeCompiler::visitNull(NullNode& node) {
    program.emit(OpCode::PUSH, 0.0);
}

void BytecodeCompiler::visitBinOp(BinOpNode& node) {
    //std::cout << "[BINOP] visitBinOp: op=" << node.op << "\n";
    
    // ── Check if the right side contains a function call ──────────────────
    bool hasCall = false;
    if (dynamic_cast<CallNode*>(node.right.get())) {
        hasCall = true;
    }
    if (auto* binOp = dynamic_cast<BinOpNode*>(node.right.get())) {
        if (dynamic_cast<CallNode*>(binOp->left.get()) ||
            dynamic_cast<CallNode*>(binOp->right.get())) {
            hasCall = true;
        }
    }
    
    // ── Only save left value for arithmetic operations ──────────────────
    bool isArithmetic = (node.op == "+" || node.op == "-" || node.op == "*" || 
                         node.op == "/" || node.op == "%" || node.op == "**");
    
    if (hasCall && isArithmetic) {
        //std::cout << "[BINOP] Saving left value for arithmetic\n";
        compileExpression(*node.left);
        std::string temp_name = "__temp_" + std::to_string(temp_counter++);
        program.emit(OpCode::DEFINE, temp_name);
        compileExpression(*node.right);
        program.emit(OpCode::LOAD, temp_name);
    } else {
        // ── Normal case: compile left then right ──────────────────────────
        compileExpression(*node.left);
        compileExpression(*node.right);
    }
    
    // ── Emit the operation ──────────────────────────────────────────────────
    if (node.op == "+") {
        program.emit(OpCode::ADD);
    } else if (node.op == "-") {
        program.emit(OpCode::SUB);
    } else if (node.op == "*") {
        program.emit(OpCode::MUL);
    } else if (node.op == "/") {
        program.emit(OpCode::DIV);
    } else if (node.op == "%") {
        program.emit(OpCode::MOD);
    } else if (node.op == "**") {
        program.emit(OpCode::POW);
    } else if (node.op == "==") {
        program.emit(OpCode::EQ);
    } else if (node.op == "!=") {
        program.emit(OpCode::NEQ);
    } else if (node.op == "<") {
        program.emit(OpCode::LT);
    } else if (node.op == ">") {
        program.emit(OpCode::GT);
    } else if (node.op == "<=") {
        program.emit(OpCode::LTE);
    } else if (node.op == ">=") {
        program.emit(OpCode::GTE);
    } else if (node.op == "and") {
        program.emit(OpCode::AND);
    } else if (node.op == "or") {
        program.emit(OpCode::OR);
    } else {
        throw std::runtime_error("Unknown binary operator: " + node.op);
    }
}

void BytecodeCompiler::visitUnaryOp(UnaryOpNode& node) {
    compileExpression(*node.operand);
    if (node.op == "-") program.emit(OpCode::NEGATE);
    else if (node.op == "not") program.emit(OpCode::NOT);
}


void BytecodeCompiler::visitCall(CallNode& node) {
    
    // ===================================================================
    // 1. MAP (Dictionary)
    // ===================================================================
    if (node.name == "map") {
        program.emit(OpCode::MAP_INIT);
        return;
    }
    
    if (node.name == "map_set") {
        for (auto& arg : node.args) {
            compileExpression(*arg);
        }
        program.emit(OpCode::MAP_SET);
        return;
    }
    
    if (node.name == "map_get") {
        for (auto& arg : node.args) {
            compileExpression(*arg);
        }
        program.emit(OpCode::MAP_GET);
        return;
    }
    
    if (node.name == "map_has") {
        for (auto& arg : node.args) {
            compileExpression(*arg);
        }
        program.emit(OpCode::MAP_HAS);
        return;
    }
    
    if (node.name == "map_size") {
        for (auto& arg : node.args) {
            compileExpression(*arg);
        }
        program.emit(OpCode::MAP_SIZE);
        return;
    }
    if (node.name == "map_keys") {
    for (auto& arg : node.args) {
        compileExpression(*arg);
    }
    program.emit(OpCode::MAP_KEYS);
    return;
}
    // ===================================================================
    // 2. BUNCH (Vector)
    // ===================================================================
    if (node.name == "bunch") {
        program.emit(OpCode::BUNCH_INIT);
        for (auto& arg : node.args) {
            compileExpression(*arg);
            program.emit(OpCode::BUNCH_PUSH);
        }
        return;
    }
    
    if (node.name == "bunch_push") {
        for (auto& arg : node.args) {
            compileExpression(*arg);
        }
        program.emit(OpCode::BUNCH_PUSH);
        return;
    }
    
    if (node.name == "bunch_pop") {
        for (auto& arg : node.args) {
            compileExpression(*arg);
        }
        program.emit(OpCode::BUNCH_POP);
        return;
    }
    
    if (node.name == "bunch_size") {
        for (auto& arg : node.args) {
            compileExpression(*arg);
        }
        program.emit(OpCode::BUNCH_SIZE);
        return;
    }
    
    if (node.name == "bunch_get") {
        for (auto& arg : node.args) {
            compileExpression(*arg);
        }
        program.emit(OpCode::BUNCH_GET);
        return;
    }
    
    if (node.name == "bunch_empty") {
        for (auto& arg : node.args) {
            compileExpression(*arg);
        }
        program.emit(OpCode::BUNCH_SIZE);
        program.emit(OpCode::PUSH, 0.0);
        program.emit(OpCode::EQ);
        return;
    }
    
    // ===================================================================
    // 3. STACK (LIFO)
    // ===================================================================
    if (node.name == "stack") {
        program.emit(OpCode::STACK_INIT);
        for (auto& arg : node.args) {
            compileExpression(*arg);
            program.emit(OpCode::STACK_PUSH);
        }
        return;
    }
    
    if (node.name == "stack_push") {
        for (auto& arg : node.args) {
            compileExpression(*arg);
        }
        program.emit(OpCode::STACK_PUSH);
        return;
    }
    
    if (node.name == "stack_pop") {
        for (auto& arg : node.args) {
            compileExpression(*arg);
        }
        program.emit(OpCode::STACK_POP);
        return;
    }
    
    if (node.name == "stack_top") {
        for (auto& arg : node.args) {
            compileExpression(*arg);
        }
        program.emit(OpCode::STACK_TOP);
        return;
    }
    
    if (node.name == "stack_size") {
        for (auto& arg : node.args) {
            compileExpression(*arg);
        }
        program.emit(OpCode::STACK_SIZE);
        return;
    }
    
    if (node.name == "stack_empty") {
        for (auto& arg : node.args) {
            compileExpression(*arg);
        }
        program.emit(OpCode::STACK_SIZE);
        program.emit(OpCode::PUSH, 0.0);
        program.emit(OpCode::EQ);
        return;
    }
    
    // ===================================================================
    // 4. QUEUE (FIFO)
    // ===================================================================
    if (node.name == "queue") {
        program.emit(OpCode::QUEUE_INIT);
        for (auto& arg : node.args) {
            compileExpression(*arg);
            program.emit(OpCode::QUEUE_ENQUEUE);
        }
        return;
    }
    
    if (node.name == "queue_enqueue") {
        for (auto& arg : node.args) {
            compileExpression(*arg);
        }
        program.emit(OpCode::QUEUE_ENQUEUE);
        return;
    }
    
    if (node.name == "queue_dequeue") {
        for (auto& arg : node.args) {
            compileExpression(*arg);
        }
        program.emit(OpCode::QUEUE_DEQUEUE);
        return;
    }
    
    if (node.name == "queue_front") {
        for (auto& arg : node.args) {
            compileExpression(*arg);
        }
        program.emit(OpCode::QUEUE_FRONT);
        return;
    }
    
    if (node.name == "queue_size") {
        for (auto& arg : node.args) {
            compileExpression(*arg);
        }
        program.emit(OpCode::QUEUE_SIZE);
        return;
    }
    
    if (node.name == "queue_empty") {
        for (auto& arg : node.args) {
            compileExpression(*arg);
        }
        program.emit(OpCode::QUEUE_SIZE);
        program.emit(OpCode::PUSH, 0.0);
        program.emit(OpCode::EQ);
        return;
    }
    
    // ===================================================================
    // 5. HEAP (Priority Queue)
    // ===================================================================
    if (node.name == "heap") {
        program.emit(OpCode::HEAP_INIT);
        for (auto& arg : node.args) {
            compileExpression(*arg);
            program.emit(OpCode::HEAP_PUSH);
        }
        return;
    }
    
    if (node.name == "heap_push") {
        for (auto& arg : node.args) {
            compileExpression(*arg);
        }
        program.emit(OpCode::HEAP_PUSH);
        return;
    }
    
    if (node.name == "heap_pop") {
        for (auto& arg : node.args) {
            compileExpression(*arg);
        }
        program.emit(OpCode::HEAP_POP);
        return;
    }
    
    if (node.name == "heap_top") {
        for (auto& arg : node.args) {
            compileExpression(*arg);
        }
        program.emit(OpCode::HEAP_TOP);
        return;
    }
    
    if (node.name == "heap_size") {
        for (auto& arg : node.args) {
            compileExpression(*arg);
        }
        program.emit(OpCode::HEAP_SIZE);
        return;
    }
    
    if (node.name == "heap_empty") {
        for (auto& arg : node.args) {
            compileExpression(*arg);
        }
        program.emit(OpCode::HEAP_SIZE);
        program.emit(OpCode::PUSH, 0.0);
        program.emit(OpCode::EQ);
        return;
    }
    
    // ===================================================================
    // 6. LINKEDLIST (Doubly Linked List)
    // ===================================================================
    if (node.name == "linkedlist") {
        program.emit(OpCode::LIST_INIT);
        for (auto& arg : node.args) {
            compileExpression(*arg);
            program.emit(OpCode::LIST_PUSH_BACK);
        }
        return;
    }
    
    if (node.name == "ll_push_front") {
        for (auto& arg : node.args) {
            compileExpression(*arg);
        }
        program.emit(OpCode::LIST_PUSH_FRONT);
        return;
    }
    
    if (node.name == "ll_push_back") {
        for (auto& arg : node.args) {
            compileExpression(*arg);
        }
        program.emit(OpCode::LIST_PUSH_BACK);
        return;
    }
    
    if (node.name == "ll_pop_front") {
        for (auto& arg : node.args) {
            compileExpression(*arg);
        }
        program.emit(OpCode::LIST_POP_FRONT);
        return;
    }
    
    if (node.name == "ll_pop_back") {
        for (auto& arg : node.args) {
            compileExpression(*arg);
        }
        program.emit(OpCode::LIST_POP_BACK);
        return;
    }
    
    if (node.name == "ll_get") {
        for (auto& arg : node.args) {
            compileExpression(*arg);
        }
        program.emit(OpCode::LIST_GET);
        return;
    }
    
    if (node.name == "ll_size") {
        for (auto& arg : node.args) {
            compileExpression(*arg);
        }
        program.emit(OpCode::LIST_SIZE);
        return;
    }
    
    if (node.name == "ll_empty") {
        for (auto& arg : node.args) {
            compileExpression(*arg);
        }
        program.emit(OpCode::LIST_SIZE);
        program.emit(OpCode::PUSH, 0.0);
        program.emit(OpCode::EQ);
        return;
    }
    
    // ===================================================================
    // 7. STRING
    // ===================================================================
    if (node.name == "string") {
        program.emit(OpCode::STRING_INIT);
        return;
    }
    
    if (node.name == "string_size") {
        for (auto& arg : node.args) {
            compileExpression(*arg);
        }
        program.emit(OpCode::STRING_SIZE);
        return;
    }
    
    if (node.name == "string_get") {
        for (auto& arg : node.args) {
            compileExpression(*arg);
        }
        program.emit(OpCode::STRING_GET);
        return;
    }
    
    if (node.name == "string_set") {
        for (auto& arg : node.args) {
            compileExpression(*arg);
        }
        program.emit(OpCode::STRING_SET);
        return;
    }
    if (functionAddresses.find(node.name) != functionAddresses.end()) {
        //std::cout << "[CALL] Calling user function: " << node.name << "\n";
        
        for (auto& arg : node.args) {
            compileExpression(*arg);
        }
        
        program.emit(OpCode::PUSH, static_cast<double>(node.args.size()));
        program.emit(OpCode::CALL, node.name);
        return;
    }
    
    // ── Fallback: try to load as variable and call ──────────────────────────
    std::cout << "[CALL] Unknown function: " << node.name << ", treating as variable\n";
    program.emit(OpCode::LOAD, node.name);
    program.emit(OpCode::CALL);
}

void BytecodeCompiler::visitIndexAccess(IndexAccessNode& node) {
    compileExpression(*node.object);
    compileExpression(*node.index);
    program.emit(OpCode::BUNCH_GET);
}

void BytecodeCompiler::visitSubpart(SubpartNode& node) {
    //std::cout << "[SLICE] Using SLICE opcode\n";
    
    // ── Compile object ──────────────────────────────────────────────────
    compileExpression(*node.object);
    
    // ── Compile start (default 0) ──────────────────────────────────────
    if (node.start) {
        compileExpression(*node.start);
    } else {
        program.emit(OpCode::PUSH, 0.0);
    }
    
    // ── Compile end (default -1 sentinel, VM will use size) ──────────
    if (node.end) {
        compileExpression(*node.end);
    } else {
        program.emit(OpCode::PUSH, -1.0);  // ← Sentinel: VM uses size
    }
    
    // ── Compile step (default 1) ──────────────────────────────────────
    if (node.step) {
        compileExpression(*node.step);
    } else {
        program.emit(OpCode::PUSH, 1.0);
    }
    
    // ── Emit SLICE ──────────────────────────────────────────────────────
    program.emit(OpCode::SLICE);
}

void BytecodeCompiler::visitProgram(ProgramNode& node) {
    //std::cout << "[PROGRAM] Compiling program\n";
    
    // ── First pass: Register all function addresses ──────────────────────────
    // Just register, don't compile yet
    for (auto& stmt : node.statements) {
        if (auto* func = dynamic_cast<FunDeclNode*>(stmt.get())) {
            // Reserve address (will be updated later)
            functionAddresses[func->name] = 0;
            functionParams[func->name] = func->params;
            program.function_addresses[func->name] = 0;
            program.function_params[func->name] = func->params;
            //std::cout << "[PROGRAM] Registered function: " << func->name << " with " << func->params.size() << " params\n";
        }
    }
    
    // ── Second pass: Compile the MAIN PROGRAM FIRST! ──────────────────────────
    //std::cout << "[PROGRAM] Compiling main program\n";
    for (auto& stmt : node.statements) {
        if (!dynamic_cast<FunDeclNode*>(stmt.get())) {
            compileStatement(*stmt);
        }
    }
    
    // ── Add HALT after main program ──────────────────────────────────────────
    program.emit(OpCode::HALT);
    
    // ── Third pass: Compile ALL function bodies AFTER the main program ──────
    for (auto& stmt : node.statements) {
        if (auto* func = dynamic_cast<FunDeclNode*>(stmt.get())) {
            // Update the address to the current position
            functionAddresses[func->name] = program.size();
            program.function_addresses[func->name] = program.size();
            //std::cout << "[PROGRAM] Compiling function: " << func->name << " at address " << functionAddresses[func->name] << "\n";
            
            for (auto& body_stmt : func->body) {
                compileStatement(*body_stmt);
            }
            
            // Implicit return
            bool hasReturn = false;
            if (!func->body.empty()) {
                if (auto* returnNode = dynamic_cast<ReturnNode*>(func->body.back().get())) {
                    hasReturn = true;
                }
            }
            if (!hasReturn) {
                program.emit(OpCode::PUSH, 0.0);
                program.emit(OpCode::RETURN);
            }
        }
    }
    
    //std::cout << "[PROGRAM] Program compiled\n";
}

void BytecodeCompiler::visitVarDecl(VarDeclNode& node) {
    ////std::cout << "[DEBUG] visitVarDecl: name=" << node.name << ", type=" << node.type << "\n";
    
    if (node.value) {
        compileExpression(*node.value);
        program.emit(OpCode::DEFINE, node.name);
    } else {
        // Default initialization based on type
        if (node.type == "number") {
            program.emit(OpCode::PUSH, 0.0);
        } else if (node.type == "string") {
            program.emit(OpCode::PUSH, "");
        } else if (node.type == "bool") {
            program.emit(OpCode::PUSH, false);
        } else if (node.type == "null") {
            program.emit(OpCode::PUSH, 0.0);
        } else if (node.type == "bunch") {
            program.emit(OpCode::BUNCH_INIT);
        } else if (node.type == "map") {
            program.emit(OpCode::MAP_INIT);
        } else if (node.type == "stack") {
            program.emit(OpCode::STACK_INIT);
        } else if (node.type == "queue") {
            program.emit(OpCode::QUEUE_INIT);
        } else if (node.type == "heap") {
            program.emit(OpCode::HEAP_INIT);
        } else if (node.type == "linkedlist") {
            program.emit(OpCode::LIST_INIT);
        } else {
            program.emit(OpCode::PUSH, 0.0);
        }
        program.emit(OpCode::DEFINE, node.name);
    }
}
void BytecodeCompiler::visitAssign(AssignNode& node) {
    compileExpression(*node.value);
    program.emit(OpCode::STORE, node.name);
}

void BytecodeCompiler::visitFunDecl(FunDeclNode& node) {
    //std::cout << "[FUNCTION] visitFunDecl: " << node.name << "\n";
    
    // ── Store function info ──────────────────────────────────────────────────
    functionAddresses[node.name] = program.size();
    functionParams[node.name] = node.params;
    program.function_addresses[node.name] = program.size();
    program.function_params[node.name] = node.params;
    
    // ── Compile function body ────────────────────────────────────────────────
    for (auto& stmt : node.body) {
        compileStatement(*stmt);
    }
    
    // ── Implicit return if no explicit return ──────────────────────────────
    bool hasReturn = false;
    if (!node.body.empty()) {
        if (auto* returnNode = dynamic_cast<ReturnNode*>(node.body.back().get())) {
            hasReturn = true;
        }
    }
    if (!hasReturn) {
        program.emit(OpCode::PUSH, 0.0);
        program.emit(OpCode::RETURN);
    }
    
    // std::cout << "[FUNCTION] " << node.name << " compiled at address " 
    //           << functionAddresses[node.name] << " with " << node.params.size() << " params\n";
}

void BytecodeCompiler::visitPrint(PrintNode& node) {
   // std::cout << "[COMPILER] visitPrint called! " << node.args.size() << " arguments\n";
    
    // ── Compile all arguments ──────────────────────────────────────────────
    for (auto& arg : node.args) {
        compileExpression(*arg);
       // std::cout << "[COMPILER] Compiled argument\n";
    }
    
    // ── Compile sep (separator) ────────────────────────────────────────────
    if (node.sep) {
        compileExpression(*node.sep);
        //std::cout << "[COMPILER] Using custom sep\n";
    } else {
        program.emit(OpCode::PUSH, std::string(" "));
        //std::cout << "[COMPILER] Using default sep: ' '\n";
    }
    
    // ── Compile end (ending) ────────────────────────────────────────────────
    if (node.end) {
        compileExpression(*node.end);
        //std::cout << "[COMPILER] Using custom end\n";
    } else {
        program.emit(OpCode::PUSH, std::string("\n"));
        //std::cout << "[COMPILER] Using default end: '\\n'\n";
    }
    
    // ── Push argument count ──────────────────────────────────────────────────
    program.emit(OpCode::PUSH, static_cast<double>(node.args.size()));
    //std::cout << "[COMPILER] Argument count: " << node.args.size() << "\n";
    
    // ── Emit PRINT instruction ──────────────────────────────────────────────
    program.emit(OpCode::PRINT);
    //std::cout << "[COMPILER] Emitted PRINT\n";
}

void BytecodeCompiler::visitReturn(ReturnNode& node) {
    if (node.value) {
        compileExpression(*node.value);
    } else {
        program.emit(OpCode::PUSH, 0.0);
    }
    program.emit(OpCode::RETURN);
}
void BytecodeCompiler::visitIf(IfNode& node) {
  //  std::cout << "[IF] visitIf\n";
    
    compileExpression(*node.condition);
  //  std::cout << "[IF] Condition compiled\n";
    
    size_t else_jump = program.code.size();
    program.emitJump(OpCode::JUMP_IF_NOT, 0);
  //  std::cout << "[IF] JUMP_IF_NOT emitted at " << else_jump << "\n";
    
    compileBlock(node.body);
    //std::cout << "[IF] Body compiled\n";
    
    size_t end_jump = program.code.size();
    program.emitJump(OpCode::JUMP, 0);
    //std::cout << "[IF] JUMP emitted at " << end_jump << "\n";
    
    size_t else_start = program.code.size();
    if (!node.elseBody.empty()) {
        compileBlock(node.elseBody);
        //std::cout << "[IF] Else body compiled\n";
    }
    
    size_t end = program.code.size();
    
    // Patch jumps
    program.code[else_jump + 1] = (else_start >> 0) & 0xFF;
    program.code[else_jump + 2] = (else_start >> 8) & 0xFF;
    program.code[else_jump + 3] = (else_start >> 16) & 0xFF;
    
    program.code[end_jump + 1] = (end >> 0) & 0xFF;
    program.code[end_jump + 2] = (end >> 8) & 0xFF;
    program.code[end_jump + 3] = (end >> 16) & 0xFF;
    
    //std::cout << "[IF] Jumps patched: else_start=" << else_start << ", end=" << end << "\n";
}


void BytecodeCompiler::visitLoop(LoopNode& node) {
    //std::cout << "\n[LOOP] ========== STARTING LOOP ==========\n";
    
    // Compile initialization
    for (auto& stmt : node.init) {
        compileStatement(*stmt);
    }
    //std::cout << "[LOOP] Init compiled\n";
    
    size_t loop_start = program.size();
    //std::cout << "[LOOP] loop_start = " << loop_start << "\n";
    
    if (node.condition) {
        compileExpression(*node.condition);
        //std::cout << "[LOOP] Condition compiled\n";
        size_t exit_jump = program.size();
        program.emit(OpCode::JUMP_IF_NOT);
        program.emitByte(0);
        program.emitByte(0);
        program.emitByte(0);
        
        break_stack.push(exit_jump);
        size_t break_start_idx = break_positions.size();
        size_t continue_start_idx = continue_positions.size();
        continue_stack.push(loop_start);
        
        // Compile body
        //std::cout << "[LOOP] Compiling body with " << node.body.size() << " statements\n";
        compileBlock(node.body);
        //std::cout << "[LOOP] Body compiled\n";
        
        // Update position
        size_t update_pos = program.size();
        //std::cout << "[LOOP] update_pos = " << update_pos << "\n";
        
        // Patch continues
        for (size_t i = continue_start_idx; i < continue_positions.size(); ++i) {
            size_t pos = continue_positions[i];
            //std::cout << "[LOOP] Patching continue at " << pos << " to " << update_pos << "\n";
            uint8_t* code = program.code.data();
            code[pos + 1] = (update_pos >> 0) & 0xFF;
            code[pos + 2] = (update_pos >> 8) & 0xFF;
            code[pos + 3] = (update_pos >> 16) & 0xFF;
        }
        continue_positions.erase(continue_positions.begin() + continue_start_idx, continue_positions.end());
        continue_stack.top() = update_pos;
        
        // Compile update
        //std::cout << "[LOOP] Compiling update\n";
        for (auto& stmt : node.update) {
            compileStatement(*stmt);
        }
        //std::cout << "[LOOP] Update compiled\n";
        
        // ── JUMP BACK TO LOOP START ──────────────────────────────────────────
        program.emit(OpCode::JUMP);
        program.emitByte((loop_start >> 0) & 0xFF);
        program.emitByte((loop_start >> 8) & 0xFF);
        program.emitByte((loop_start >> 16) & 0xFF);
        
        // ── NOW exit is AFTER the jump back ──────────────────────────────────
        size_t exit = program.size();
        //std::cout << "[LOOP] exit = " << exit << "\n";
        
        // Patch break jumps
        for (size_t i = break_start_idx; i < break_positions.size(); ++i) {
            size_t pos = break_positions[i];
            //std::cout << "[LOOP] Patching break at " << pos << " to " << exit << "\n";
            uint8_t* code = program.code.data();
            code[pos + 1] = (exit >> 0) & 0xFF;
            code[pos + 2] = (exit >> 8) & 0xFF;
            code[pos + 3] = (exit >> 16) & 0xFF;
        }
        break_positions.erase(break_positions.begin() + break_start_idx, break_positions.end());
        
        // Patch exit jump
        uint8_t* code = program.code.data();
        code[exit_jump + 1] = (exit >> 0) & 0xFF;
        code[exit_jump + 2] = (exit >> 8) & 0xFF;
        code[exit_jump + 3] = (exit >> 16) & 0xFF;
        
        break_stack.pop();
        continue_stack.pop();
    }
    //std::cout << "[LOOP] ========== LOOP DONE ==========\n\n";
}
void BytecodeCompiler::visitWhile(WhileNode& node) {
    size_t loop_start = program.size();
    
    compileExpression(*node.condition);
    size_t exit_jump = program.size();
    program.emit(OpCode::JUMP_IF_NOT);
    program.emitByte(0);
    program.emitByte(0);
    program.emitByte(0);
    
    // Continue jumps to loop_start (condition check)
    break_stack.push(exit_jump);
    continue_stack.push(loop_start);  // ← Continue to condition check
    
    size_t break_start_idx = break_positions.size();
    
    compileBlock(node.body);
    
    size_t exit = program.size();
    
    // Patch breaks
    for (size_t i = break_start_idx; i < break_positions.size(); ++i) {
        size_t pos = break_positions[i];
        uint8_t* code = program.code.data();
        code[pos + 1] = (exit >> 0) & 0xFF;
        code[pos + 2] = (exit >> 8) & 0xFF;
        code[pos + 3] = (exit >> 16) & 0xFF;
    }
    break_positions.erase(break_positions.begin() + break_start_idx, break_positions.end());
    
    program.emit(OpCode::JUMP);
    program.emitByte((loop_start >> 0) & 0xFF);
    program.emitByte((loop_start >> 8) & 0xFF);
    program.emitByte((loop_start >> 16) & 0xFF);
    
    uint8_t* code = program.code.data();
    code[exit_jump + 1] = (exit >> 0) & 0xFF;
    code[exit_jump + 2] = (exit >> 8) & 0xFF;
    code[exit_jump + 3] = (exit >> 16) & 0xFF;
    
    break_stack.pop();
    continue_stack.pop();
}
void BytecodeCompiler::visitBreak(BreakNode& node) {
    //std::cout << "[BREAK] visitBreak called\n";
    
    if (break_stack.empty()) {
        //std::cout << "[BREAK] ERROR: break_stack is empty!\n";
        throw std::runtime_error("Break outside loop");
    }
    
    // Store the current position for patching
    size_t pos = program.size();
    //std::cout << "[BREAK] Storing break at position " << pos << "\n";
    break_positions.push_back(pos);
    
    // Emit placeholder JUMP
    program.emit(OpCode::JUMP);
    program.emitByte(0);
    program.emitByte(0);
    program.emitByte(0);
}
void BytecodeCompiler::visitContinue(ContinueNode& node) {
    //std::cout << "[CONTINUE] visitContinue called\n";
    
    if (continue_stack.empty()) {
        //std::cout << "[CONTINUE] ERROR: continue_stack is empty!\n";
        throw std::runtime_error("Continue outside loop");
    }
    
    // Store the current position for patching
    size_t pos = program.size();
    //std::cout << "[CONTINUE] Storing continue at position " << pos << "\n";
    continue_positions.push_back(pos);
    
    // Emit placeholder JUMP
    program.emit(OpCode::JUMP);
    program.emitByte(0);
    program.emitByte(0);
    program.emitByte(0);
}
void BytecodeCompiler::visitTryCatch(TryCatchNode& node) {
    std::cout << "[TRY] visitTryCatch\n";
    
    // ── Emit TRY_START with catch address ──────────────────────────────
    size_t try_start = program.size();
    program.emit(OpCode::TRY_START);
    program.emitByte(0);
    program.emitByte(0);
    program.emitByte(0);
    
    // ── Compile try block ──────────────────────────────────────────────────
    compileBlock(node.tryBlock);
    
    // ── After try block, jump to end (skip catch) ──────────────────────────
    size_t end_jump = program.size();
    program.emit(OpCode::JUMP);
    program.emitByte(0);
    program.emitByte(0);
    program.emitByte(0);
    
    // ── Catch block ──────────────────────────────────────────────────────────
    size_t catch_start = program.size();
    std::cout << "[TRY] catch_start = " << catch_start << "\n";
    
    // Patch TRY_START with catch address
    uint8_t* code = program.code.data();
    code[try_start + 1] = (catch_start >> 0) & 0xFF;
    code[try_start + 2] = (catch_start >> 8) & 0xFF;
    code[try_start + 3] = (catch_start >> 16) & 0xFF;
    
    // ── Emit CATCH_START ──────────────────────────────────────────────────
    program.emit(OpCode::CATCH_START);
    std::cout << "[TRY] Emitting DEFINE with name: '" << node.catchVar << "'\n";
    program.emit(OpCode::DEFINE, node.catchVar);
    
    // ── Compile catch block ──────────────────────────────────────────────────
    compileBlock(node.catchBlock);
    
    // ── Patch end jump ──────────────────────────────────────────────────────
    size_t end = program.size();
    code[end_jump + 1] = (end >> 0) & 0xFF;
    code[end_jump + 2] = (end >> 8) & 0xFF;
    code[end_jump + 3] = (end >> 16) & 0xFF;
    
    std::cout << "[TRY] end = " << end << "\n";
}
void BytecodeCompiler::visitThrow(ThrowNode& node) {
    compileExpression(*node.value);
    program.emit(OpCode::THROW);
}



void BytecodeCompiler::compileInitializerList(const NodeList& elements) {
    program.emit(OpCode::BUNCH_INIT);
    for (auto& elem : elements) {
        compileExpression(*elem);
        program.emit(OpCode::BUNCH_PUSH);
    }
}

void BytecodeCompiler::emitMethodCall(const std::string& method, const NodeList& args, const std::string& objectType) {
    for (auto& arg : args) {
        compileExpression(*arg);
    }
    
    if (objectType == "bunch" || objectType == "Bunch") {
        compileBunchOperation(method, args);
    } else if (objectType == "map" || objectType == "Map") {
        compileMapOperation(method, args);
    } else if (objectType == "stack" || objectType == "Stack") {
        compileStackOperation(method, args);
    } else if (objectType == "queue" || objectType == "Queue") {
        compileQueueOperation(method, args);
    } else if (objectType == "heap" || objectType == "Heap") {
        compileHeapOperation(method, args);
    } else if (objectType == "linkedlist" || objectType == "LinkedList") {
        compileListOperation(method, args);
    }
}

void BytecodeCompiler::compileBunchOperation(const std::string& method, const NodeList& args) {
    if (method == "push") program.emit(OpCode::BUNCH_PUSH);
    else if (method == "pop") program.emit(OpCode::BUNCH_POP);
    else if (method == "size") program.emit(OpCode::BUNCH_SIZE);
    else if (method == "empty") program.emit(OpCode::BUNCH_EMPTY);
}

void BytecodeCompiler::compileMapOperation(const std::string& method, const NodeList& args) {
    if (method == "set") program.emit(OpCode::MAP_SET);
    else if (method == "get") program.emit(OpCode::MAP_GET);
    else if (method == "has") program.emit(OpCode::MAP_HAS);
    else if (method == "size") program.emit(OpCode::MAP_SIZE);
}

void BytecodeCompiler::compileStackOperation(const std::string& method, const NodeList& args) {
    if (method == "push") program.emit(OpCode::STACK_PUSH);
    else if (method == "pop") program.emit(OpCode::STACK_POP);
    else if (method == "top") program.emit(OpCode::STACK_TOP);
    else if (method == "size") program.emit(OpCode::STACK_SIZE);
}

void BytecodeCompiler::compileQueueOperation(const std::string& method, const NodeList& args) {
    if (method == "enqueue") program.emit(OpCode::QUEUE_ENQUEUE);
    else if (method == "dequeue") program.emit(OpCode::QUEUE_DEQUEUE);
    else if (method == "front") program.emit(OpCode::QUEUE_FRONT);
    else if (method == "size") program.emit(OpCode::QUEUE_SIZE);
}

void BytecodeCompiler::compileHeapOperation(const std::string& method, const NodeList& args) {
    if (method == "push") program.emit(OpCode::HEAP_PUSH);
    else if (method == "pop") program.emit(OpCode::HEAP_POP);
    else if (method == "top") program.emit(OpCode::HEAP_TOP);
    else if (method == "size") program.emit(OpCode::HEAP_SIZE);
}

void BytecodeCompiler::compileListOperation(const std::string& method, const NodeList& args) {
    if (method == "push_front") program.emit(OpCode::LIST_PUSH_FRONT);
    else if (method == "push_back") program.emit(OpCode::LIST_PUSH_BACK);
    else if (method == "pop_front") program.emit(OpCode::LIST_POP_FRONT);
    else if (method == "pop_back") program.emit(OpCode::LIST_POP_BACK);
    else if (method == "size") program.emit(OpCode::LIST_SIZE);
}