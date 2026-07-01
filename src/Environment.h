#pragma once
#include "./AST.h"
#include <unordered_map>
#include <memory>
#include <stdexcept>

std::string getAvonType(const AvonValue& val);

struct Environment {
    std::unordered_map<std::string, AvonValue> vars;
    std::unordered_map<std::string, std::string> varTypes;
    std::shared_ptr<Environment> parent;
    
    explicit Environment(std::shared_ptr<Environment> p = nullptr) 
        : parent(std::move(p)) {}
    
    AvonValue get(const std::string& name) const {
        auto it = vars.find(name);
        if (it != vars.end()) return it->second;
        if (parent) return parent->get(name);
        throw std::runtime_error("Undefined variable: " + name);
    }
    
    void set(const std::string& name, const AvonValue& val) {
        auto it = vars.find(name);
        if (it != vars.end()) {
            std::string expectedType = getType(name);
            std::string actualType = getAvonType(val); 
            if (!expectedType.empty() && expectedType != actualType) {
                throw std::runtime_error("Type mismatch: expected " + expectedType + ", got " + actualType);
            }
            it->second = val;
            return;
        }
        if (parent) { 
            parent->set(name, val); 
            return; 
        }
        throw std::runtime_error("Undefined variable: " + name);
    }
    
    std::string getType(const std::string& name) const {
        auto it = varTypes.find(name);
        if (it != varTypes.end()) return it->second;
        if (parent) return parent->getType(name);
        return "";
    }
    
    void define(const std::string& name, const AvonValue& val) {
        vars[name] = val;
    }
    
    void define(const std::string& name, const AvonValue& value, const std::string& type) {
        vars[name] = value;
        varTypes[name] = type;
    }
};