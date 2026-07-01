#pragma once
#include <stdexcept> 
#include <iostream>
#include "OpCodes.h" 
#include <vector>
#include <string>
#include <unordered_map>
#include <cstdint>

struct BytecodeValue {
    enum Type { NONE, NUMBER, STRING, BOOL, LABEL } type;
    
    union {
        double numVal;
        const char* strVal;
        size_t string_idx;
        bool boolVal;
        size_t labelPos;
    };
    
    BytecodeValue() : type(NONE), numVal(0) {}
    BytecodeValue(double v) : type(NUMBER), numVal(v) {}
    BytecodeValue(const char* v) : type(STRING), strVal(v) {}
    BytecodeValue(bool v) : type(BOOL), boolVal(v) {}
    BytecodeValue(size_t v) : type(LABEL), labelPos(v) {}
    BytecodeValue(size_t idx, bool) : type(STRING), string_idx(idx) {}
};

class BytecodeProgram {
public:
    std::vector<uint8_t> code;
    std::vector<BytecodeValue> constants;
    std::unordered_map<std::string, size_t> labels;
    std::vector<std::string> string_constants;
    std::unordered_map<std::string, size_t> function_addresses; 
    std::unordered_map<std::string, std::vector<std::string>> function_params; 
    void emit(OpCode op) {
        code.push_back(static_cast<uint8_t>(op));
    }
    
   void emit(OpCode op, double val) {
    //std::cout << "[EMIT] PUSH double: " << val << "\n";
    code.push_back(static_cast<uint8_t>(op));
    constants.push_back(BytecodeValue(val));
    code.push_back(static_cast<uint8_t>(constants.size() - 1));
}
    
    void emit(OpCode op, const std::string& val) {
        code.push_back(static_cast<uint8_t>(op));
        size_t idx = string_constants.size();
        string_constants.push_back(val);
        constants.push_back(BytecodeValue(idx, true));  
        code.push_back(static_cast<uint8_t>(constants.size() - 1));
    }
    
    void emit(OpCode op, bool val) {
        code.push_back(static_cast<uint8_t>(op));
        constants.push_back(BytecodeValue(val));
        code.push_back(static_cast<uint8_t>(constants.size() - 1));
    }
    
    void emitLabel(OpCode op, size_t label) {
        code.push_back(static_cast<uint8_t>(op));
        constants.push_back(BytecodeValue(label));
        code.push_back(static_cast<uint8_t>(constants.size() - 1));
    }
    
    void emitByte(uint8_t byte) {
        code.push_back(byte);
    }
    
    void emitJump(OpCode op, size_t target) {
        code.push_back(static_cast<uint8_t>(op));
        code.push_back((target >> 0) & 0xFF);
        code.push_back((target >> 8) & 0xFF);
        code.push_back((target >> 16) & 0xFF);
    }
    
    void addLabel(const std::string& name) {
        labels[name] = code.size();
    }
    
    size_t getLabel(const std::string& name) const {
        auto it = labels.find(name);
        if (it == labels.end()) {
            throw std::runtime_error("Undefined label: " + name);
        }
        return it->second;
    }
    
    void patchLabel(const std::string& name, size_t pos) {
        size_t target = getLabel(name);
        code[pos] = target & 0xFF;
        code[pos + 1] = (target >> 8) & 0xFF;
        code[pos + 2] = (target >> 16) & 0xFF;
    }
    
    size_t size() const { return code.size(); }
    bool empty() const { return code.empty(); }
    void clear() {
        code.clear();
        constants.clear();
        labels.clear();
    }
};