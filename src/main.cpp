#include "Lexer.h"
#include "Parser.h"
#include "bytecode/BytecodeCompiler.h"
#include "bytecode/NovaVM.h"
#include <iostream>
#include <fstream>
#include <sstream>
bool runSource(const std::string& source, NovaVM& vm, bool debug = false) {
    try {
        Lexer lexer(source);
        auto tokens = lexer.tokenize();
        
        Parser parser(std::move(tokens));
        auto program = parser.parse();
        
        BytecodeCompiler compiler;
        auto bytecode = compiler.compile(*program);
        
        // ── BYTECODE DUMP ──────────────────────────────────────────────────────
        // std::cout << "\n========== BYTECODE DUMP ==========\n";
        // std::cout << "Size: " << bytecode.code.size() << " bytes\n";
        
        // Opcode names for debugging
        auto opName = [](uint8_t op) -> std::string {
        switch(op) {
            case 0: return "PUSH";
            case 1: return "POP";
            case 2: return "ADD";
            case 3: return "SUB";
            case 4: return "MUL";
            case 5: return "DIV";
            case 6: return "MOD";
            case 7: return "POW";
            case 8: return "NEGATE";
            case 9: return "EQ";
            case 10: return "NEQ";
            case 11: return "LT";
            case 12: return "GT";
            case 13: return "LTE";
            case 14: return "GTE";
            case 15: return "AND";
            case 16: return "OR";
            case 17: return "NOT";
            case 18: return "LOAD";
            case 19: return "STORE";
            case 20: return "DEFINE";
            case 21: return "JUMP";
            case 22: return "JUMP_IF";
            case 23: return "JUMP_IF_NOT";
            case 24: return "CALL";
            case 25: return "RETURN";
            case 26: return "FUN_DECL";
            case 27: return "BUNCH_INIT";
            case 28: return "BUNCH_PUSH";
            case 29: return "BUNCH_POP";
            case 30: return "BUNCH_GET";
            case 31: return "BUNCH_SIZE";
            case 32: return "BUNCH_EMPTY";
            case 33: return "STRING_INIT";
            case 34: return "STRING_SET";
            case 35: return "STRING_GET";
            case 36: return "STRING_SIZE";
            case 37: return "SLICE";
            case 38: return "MAP_INIT";
            case 39: return "MAP_SET";
            case 40: return "MAP_GET";
            case 41: return "MAP_HAS";
            case 42: return "MAP_SIZE";
            case 43: return "MAP_KEYS";
            case 44: return "STACK_INIT";
            case 45: return "STACK_PUSH";
            case 46: return "STACK_POP";
            case 47: return "STACK_TOP";
            case 48: return "STACK_SIZE";
            case 49: return "QUEUE_INIT";
            case 50: return "QUEUE_ENQUEUE";
            case 51: return "QUEUE_DEQUEUE";
            case 52: return "QUEUE_FRONT";
            case 53: return "QUEUE_SIZE";
            case 54: return "HEAP_INIT";
            case 55: return "HEAP_PUSH";
            case 56: return "HEAP_POP";
            case 57: return "HEAP_TOP";
            case 58: return "HEAP_SIZE";
            case 59: return "LIST_INIT";
            case 60: return "LIST_PUSH_FRONT";
            case 61: return "LIST_PUSH_BACK";
            case 62: return "LIST_POP_FRONT";
            case 63: return "LIST_POP_BACK";
            case 64: return "LIST_GET";
            case 65: return "LIST_SIZE";
            case 66: return "PRINT";     
            case 67: return "THROW";
            case 68: return "TRY_START";
            case 69: return "TRY_END";
            case 70: return "CATCH_START";
            case 71: return "HALT";      
            default: return "UNKNOWN(" + std::to_string(op) + ")";
        }
};
        
        // for (size_t i = 0; i < bytecode.code.size(); ++i) {
        //     uint8_t op = bytecode.code[i];
        //     std::cout << "  " << i << ": " << (int)op << " (" << opName(op) << ")";
            
        //     // Show jump targets
        //     if (op == 21 || op == 22 || op == 23) {
        //         if (i + 3 < bytecode.code.size()) {
        //             size_t target = bytecode.code[i+1] | (bytecode.code[i+2] << 8) | (bytecode.code[i+3] << 16);
        //             std::cout << " -> " << target;
        //         }
        //     }
        //     std::cout << "\n";
        // }
        
        // std::cout << "\n[CONSTANTS]\n";
        // for (size_t i = 0; i < bytecode.constants.size(); ++i) {
        //     std::cout << "  " << i << ": ";
        //     if (bytecode.constants[i].type == BytecodeValue::NUMBER)
        //         std::cout << bytecode.constants[i].numVal;
        //     else if (bytecode.constants[i].type == BytecodeValue::STRING)
        //         std::cout << "\"" << bytecode.constants[i].strVal << "\"";
        //     else if (bytecode.constants[i].type == BytecodeValue::BOOL)
        //         std::cout << (bytecode.constants[i].boolVal ? "true" : "false");
        //     else
        //         std::cout << "?";
        //     std::cout << "\n";
        // }
        //std::cout << "====================================\n\n";
        // ──────────────────────────────────────────────────────────────────────
        // std::cout << "\n[DEBUG] Bytecode size: " << bytecode.code.size() << "\n";
        // for (size_t i = 0; i < bytecode.code.size(); ++i) {
        //     std::cout << "  " << i << ": " << (int)bytecode.code[i] << "\n";
        // }
        vm.setDebug(debug);
        vm.run(bytecode);
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "[ERROR] " << e.what() << "\n";
        return false;
    }
}

void repl() {
    std::cout << "============================================================\n";
    std::cout << "              Nova 2.1 - Bytecode VM Edition\n";
    std::cout << "        Python-inspired syntax | C++ Powered VM\n";
    std::cout << "============================================================\n";
    std::cout << "Type 'exit' to quit, 'debug' to toggle debug mode\n\n";

    NovaVM vm;  // ← CREATE ONCE, REUSE
    bool debug = false;

    while (true) {
        std::cout << "=> ";
        std::string line;
        if (!std::getline(std::cin, line)) break;

        if (line == "exit" || line == "quit") {
            std::cout << "Goodbye!\n";
            break;
        }
        if (line == "debug") {
            debug = !debug;
            std::cout << "Debug mode: " << (debug ? "ON" : "OFF") << "\n";
            continue;
        }
        if (line.empty()) continue;

        std::string source = line;
        int depth = 0;
        for (char c : line) {
            if (c == '{') ++depth;
            if (c == '}') --depth;
        }

        while (depth > 0) {
            std::cout << "... ";
            std::string cont;
            if (!std::getline(std::cin, cont)) break;
            source += "\n" + cont;
            for (char c : cont) {
                if (c == '{') ++depth;
                if (c == '}') --depth;
            }
        }

        runSource(source, vm, debug);
    }
}

int runFile(const std::string& path, bool debug = false) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "[ERROR] Cannot open file: " << path << "\n";
        return 1;
    }
    std::ostringstream ss;
    ss << file.rdbuf();

    NovaVM vm;
    return runSource(ss.str(), vm, debug) ? 0 : 1;
}

int main(int argc, char* argv[]) {
    bool debug = false;
    
    if (argc == 1) {
        repl();
        return 0;
    }
    
    if (argc >= 2) {
        std::string arg1 = argv[1];
        if (arg1 == "--debug") {
            debug = true;
            if (argc == 2) {
                repl();
                return 0;
            }
            return runFile(argv[2], debug);
        }
        return runFile(argv[1], debug);
    }
    
    std::cerr << "Usage: nova [--debug] [file.nv]\n";
    std::cerr << "       nova                 (REPL mode)\n";
    std::cerr << "       nova --debug         (REPL with debug)\n";
    std::cerr << "       nova file.nv         (Run file)\n";
    std::cerr << "       nova --debug file.nv (Run file with debug)\n";
    return 1;
}