#include "AvonVM.h"
#include "../Bytecode/OpCodes.h"

std::string AvonVM::getAvonType(const AvonValue& val) {
    return std::visit([](auto&& v) -> std::string {
        using T = std::decay_t<decltype(v)>;
        if constexpr (std::is_same_v<T, double>) return "number";
        else if constexpr (std::is_same_v<T, std::string>) return "string";
        else if constexpr (std::is_same_v<T, bool>) return "bool";
        else if constexpr (std::is_same_v<T, std::nullptr_t>) return "null";
        else if constexpr (std::is_same_v<T, std::shared_ptr<AvonBunch>>) return "bunch";
        else return "object";
    }, val);
}

void AvonVM::run(const BytecodeProgram& program) {
    code = program.code;
    constants = program.constants;
    pc = 0;
    running = true;
    executeLoop();
}

void AvonVM::executeLoop() {
    while (running && pc < code.size()) {
        OpCode op = static_cast<OpCode>(code[pc++]);
        switch (op) {
            case OpCode::PUSH: {
                uint8_t idx = code[pc++];
                // Push logic based on constants[idx]
                break;
            }
            case OpCode::ADD: {
                auto b = pop();
                auto a = pop();
                // Add logic
                break;
            }
            case OpCode::HALT:
                running = false;
                break;
            default:
                throw std::runtime_error("Unknown OpCode");
        }
    }
}