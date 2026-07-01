#pragma once
#include "../Bytecode/BytecodeProgram.h"
#include "../Environment.h"
#include "../AST.h"
#include <vector>
#include <stack>
#include <unordered_map>
#include <memory>

class AvonVM {
public:
    AvonVM();
    void run(const BytecodeProgram& program);
    void setDebug(bool enabled) { debug_mode = enabled; }
    static std::string getAvonType(const AvonValue& val);

private:
    struct Frame {
        size_t pc;
        std::shared_ptr<Environment> env;
        size_t return_pc;
    };

    std::vector<uint8_t> code;
    std::vector<BytecodeValue> constants;
    std::vector<AvonValue> stack;
    std::vector<Frame> frames;
    std::shared_ptr<Environment> current_env;
    size_t pc;
    bool running;
    bool debug_mode;

    void push(const AvonValue& val);
    AvonValue pop();
    void executeLoop();
};