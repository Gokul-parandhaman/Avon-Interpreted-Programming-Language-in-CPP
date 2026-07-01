#pragma once
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <variant>
#include <iostream>

struct Visitor;
struct Node;
struct Environment;
struct AvonFunction;
struct AvonBunch;
struct AvonString;
struct AvonMap;
struct AvonStack;
struct AvonQueue;
struct AvonHeap;
struct AvonLinkedList;

using NodePtr = std::unique_ptr<Node>;
using NodeList = std::vector<NodePtr>;

using AvonValue = std::variant<
    double,
    std::string,
    bool,
    std::nullptr_t,
    std::shared_ptr<AvonBunch>,
    std::shared_ptr<AvonString>,
    std::shared_ptr<AvonMap>,
    std::shared_ptr<AvonStack>,
    std::shared_ptr<AvonQueue>,
    std::shared_ptr<AvonHeap>,
    std::shared_ptr<AvonLinkedList>,
    std::shared_ptr<AvonFunction>
>;

// ── Data Structures ──────────────────────────────────────────────────────────

struct AvonBunch {
    std::vector<AvonValue> data;
    AvonBunch() = default;
    explicit AvonBunch(const std::vector<AvonValue>& d) : data(d) {}
    
    void push(const AvonValue& v) { data.push_back(v); }
    AvonValue pop() {
        if (data.empty()) throw std::runtime_error("Bunch is empty");
        AvonValue v = data.back();
        data.pop_back();
        return v;
    }
    AvonValue& at(size_t idx) {
        if (idx >= data.size()) throw std::runtime_error("Index out of bounds");
        return data[idx];
    }
    AvonValue& operator[](size_t idx) { return data[idx]; }
    size_t size() const { return data.size(); }
    bool empty() const { return data.empty(); }
    void clear() { data.clear(); }
    void insert(size_t idx, const AvonValue& v) {
        if (idx > data.size()) throw std::runtime_error("Index out of bounds");
        data.insert(data.begin() + idx, v);
    }
    void remove(size_t idx) {
        if (idx >= data.size()) throw std::runtime_error("Index out of bounds");
        data.erase(data.begin() + idx);
    }
};

struct AvonString {
    std::string data;
    AvonString() = default;
    AvonString(const std::string& s) : data(s) {}
    size_t size() const { return data.size(); }
    bool empty() const { return data.empty(); }
    char& operator[](size_t idx) { return data[idx]; }
    const char& operator[](size_t idx) const { return data[idx]; }
};

struct AvonMap {
    std::unordered_map<std::string, AvonValue> data;
    AvonMap() = default;
    
    void set(const std::string& key, const AvonValue& value) { data[key] = value; }
    AvonValue get(const std::string& key) const {
        auto it = data.find(key);
        if (it == data.end()) throw std::runtime_error("Key not found: " + key);
        return it->second;
    }
    bool has(const std::string& key) const { return data.find(key) != data.end(); }
    void remove(const std::string& key) { data.erase(key); }
    size_t size() const { return data.size(); }
    bool empty() const { return data.empty(); }
    void clear() { data.clear(); }
    std::vector<std::string> keys() const {
        std::vector<std::string> result;
        for (const auto& pair : data) result.push_back(pair.first);
        return result;
    }
};

struct AvonStack {
    std::vector<AvonValue> data;
    AvonStack() = default;
    
    void push(const AvonValue& v) { data.push_back(v); }
    AvonValue pop() {
        if (data.empty()) throw std::runtime_error("Stack is empty");
        AvonValue v = data.back();
        data.pop_back();
        return v;
    }
    AvonValue top() const {
        if (data.empty()) throw std::runtime_error("Stack is empty");
        return data.back();
    }
    size_t size() const { return data.size(); }
    bool empty() const { return data.empty(); }
    void clear() { data.clear(); }
};

struct AvonQueue {
    std::vector<AvonValue> data;
    size_t front_idx = 0;
    AvonQueue() = default;
    
    void enqueue(const AvonValue& v) { data.push_back(v); }
    AvonValue dequeue() {
        if (front_idx >= data.size()) throw std::runtime_error("Queue is empty");
        AvonValue v = data[front_idx++];
        if (front_idx > 1000 && front_idx > data.size() / 2) {
            data.erase(data.begin(), data.begin() + front_idx);
            front_idx = 0;
        }
        return v;
    }
    AvonValue front() const {
        if (front_idx >= data.size()) throw std::runtime_error("Queue is empty");
        return data[front_idx];
    }
    size_t size() const { return data.size() - front_idx; }
    bool empty() const { return front_idx >= data.size(); }
    void clear() { data.clear(); front_idx = 0; }
};

struct AvonHeap {
    std::vector<double> data;
    AvonHeap() = default;
    
    void push(double v) {
        data.push_back(v);
        size_t idx = data.size() - 1;
        while (idx > 0) {
            size_t parent = (idx - 1) / 2;
            if (data[parent] <= data[idx]) break;
            std::swap(data[parent], data[idx]);
            idx = parent;
        }
    }
    double pop() {
        if (data.empty()) throw std::runtime_error("Heap is empty");
        double result = data[0];
        data[0] = data.back();
        data.pop_back();
        heapify(0);
        return result;
    }
    double top() const {
        if (data.empty()) throw std::runtime_error("Heap is empty");
        return data[0];
    }
    size_t size() const { return data.size(); }
    bool empty() const { return data.empty(); }
    void clear() { data.clear(); }
    
private:
    void heapify(size_t idx) {
        size_t smallest = idx;
        size_t left = 2 * idx + 1;
        size_t right = 2 * idx + 2;
        if (left < data.size() && data[left] < data[smallest]) smallest = left;
        if (right < data.size() && data[right] < data[smallest]) smallest = right;
        if (smallest != idx) {
            std::swap(data[idx], data[smallest]);
            heapify(smallest);
        }
    }
};

struct AvonLinkedList {
    struct Node {
        AvonValue value;
        std::shared_ptr<Node> next;
        std::shared_ptr<Node> prev;
        Node(const AvonValue& v) : value(v), next(nullptr), prev(nullptr) {}
    };
    std::shared_ptr<Node> head;
    std::shared_ptr<Node> tail;
    size_t length = 0;
    
    AvonLinkedList() = default;
    AvonLinkedList(const std::vector<AvonValue>& init) {
        for (const auto& v : init) push_back(v);
    }
    
    void push_front(const AvonValue& v) {
        auto node = std::make_shared<Node>(v);
        if (!head) {
            head = tail = node;
        } else {
            node->next = head;
            head->prev = node;
            head = node;
        }
        length++;
    }
    void push_back(const AvonValue& v) {
        auto node = std::make_shared<Node>(v);
        if (!tail) {
            head = tail = node;
        } else {
            node->prev = tail;
            tail->next = node;
            tail = node;
        }
        length++;
    }
    AvonValue pop_front() {
        if (!head) throw std::runtime_error("List is empty");
        AvonValue v = head->value;
        head = head->next;
        if (head) head->prev = nullptr;
        else tail = nullptr;
        length--;
        return v;
    }
    AvonValue pop_back() {
        if (!tail) throw std::runtime_error("List is empty");
        AvonValue v = tail->value;
        tail = tail->prev;
        if (tail) tail->next = nullptr;
        else head = nullptr;
        length--;
        return v;
    }
    AvonValue& at(size_t idx) {
        if (idx >= length) throw std::runtime_error("Index out of bounds");
        auto current = head;
        for (size_t i = 0; i < idx; i++) current = current->next;
        return current->value;
    }
    AvonValue& operator[](size_t idx) { return at(idx); }
    size_t size() const { return length; }
    bool empty() const { return length == 0; }
    void clear() { head = tail = nullptr; length = 0; }
};

struct AvonFunction {
    std::string name;
    std::vector<std::string> params;
    std::vector<Node*> body;
    std::shared_ptr<Environment> closure_env;
    
    AvonFunction(std::string n, std::vector<std::string> p,
                 std::vector<Node*> b, std::shared_ptr<Environment> e)
        : name(std::move(n)), params(std::move(p)),
          body(std::move(b)), closure_env(std::move(e)) {}
};

// ── Visitor ──────────────────────────────────────────────────────────────────

struct Visitor {
    virtual ~Visitor() = default;
    
    virtual void visitNumber(class NumberNode&) = 0;
    virtual void visitString(class StringNode&) = 0;
    virtual void visitBool(class BoolNode&) = 0;
    virtual void visitNull(class NullNode&) = 0;
    
    virtual void visitBinOp(class BinOpNode&) = 0;
    virtual void visitUnaryOp(class UnaryOpNode&) = 0;
    virtual void visitIdent(class IdentNode&) = 0;
    virtual void visitCall(class CallNode&) = 0;
    virtual void visitIndexAccess(class IndexAccessNode&) = 0;
    virtual void visitSubpart(class SubpartNode&) = 0;
    
    virtual void visitProgram(class ProgramNode&) = 0;
    virtual void visitVarDecl(class VarDeclNode&) = 0;
    virtual void visitAssign(class AssignNode&) = 0;
    virtual void visitFunDecl(class FunDeclNode&) = 0;
    virtual void visitIf(class IfNode&) = 0;
    virtual void visitLoop(class LoopNode&) = 0;
    virtual void visitWhile(class WhileNode&) = 0;
    virtual void visitPrint(class PrintNode&) = 0;
    virtual void visitReturn(class ReturnNode&) = 0;
    virtual void visitTryCatch(class TryCatchNode&) = 0;
    virtual void visitThrow(class ThrowNode&) = 0;
    virtual void visitBreak(class BreakNode&) = 0;
    virtual void visitContinue(class ContinueNode&) = 0;
};

// ── AST Nodes ─────────────────────────────────────────────────────────────────

struct Node {
    virtual ~Node() = default;
    virtual void accept(Visitor& v) = 0;
};

#define VISIT(X) void accept(Visitor& v) override { v.visit##X(*this); }

struct NumberNode : Node { VISIT(Number) double value; explicit NumberNode(double v) : value(v) {} };
struct StringNode : Node { VISIT(String) std::string value; explicit StringNode(std::string v) : value(std::move(v)) {} };
struct BoolNode : Node { VISIT(Bool) bool value; explicit BoolNode(bool v) : value(v) {} };
struct NullNode : Node { VISIT(Null) };

struct BinOpNode : Node {
    VISIT(BinOp)
    NodePtr left;
    std::string op;
    NodePtr right;
    BinOpNode(NodePtr l, std::string o, NodePtr r) : left(std::move(l)), op(std::move(o)), right(std::move(r)) {}
};

struct UnaryOpNode : Node {
    VISIT(UnaryOp)
    std::string op;
    NodePtr operand;
    UnaryOpNode(std::string o, NodePtr n) : op(std::move(o)), operand(std::move(n)) {}
};

struct IdentNode : Node {
    VISIT(Ident)
    std::string name;
    explicit IdentNode(std::string n) : name(std::move(n)) {}
};

struct CallNode : Node {
    VISIT(Call)
    std::string name;
    NodeList args;
    CallNode(std::string n, NodeList a) : name(std::move(n)), args(std::move(a)) {}
};

struct IndexAccessNode : Node {
    VISIT(IndexAccess)
    NodePtr object;
    NodePtr index;
    IndexAccessNode(NodePtr obj, NodePtr idx) : object(std::move(obj)), index(std::move(idx)) {}
};

struct SubpartNode : Node {
    VISIT(Subpart)
    NodePtr object, start, end, step;
    SubpartNode(NodePtr obj, NodePtr s, NodePtr e, NodePtr st = nullptr)
        : object(std::move(obj)), start(std::move(s)), end(std::move(e)), step(std::move(st)) {}
};

struct ProgramNode : Node {
    VISIT(Program)
    NodeList statements;
    explicit ProgramNode(NodeList s) : statements(std::move(s)) {}
};

struct VarDeclNode : Node {
    VISIT(VarDecl)
    std::string name;
    std::string type;
    NodePtr value;
    
    VarDeclNode(std::string n, std::string t, NodePtr v) : name(std::move(n)), type(std::move(t)), value(std::move(v)) {}
    VarDeclNode(std::string n, NodePtr v) : name(std::move(n)), type(""), value(std::move(v)) {}
};

struct AssignNode : Node {
    VISIT(Assign)
    std::string name;
    NodePtr value;
    AssignNode(std::string n, NodePtr v) : name(std::move(n)), value(std::move(v)) {}
};

struct FunDeclNode : Node {
    VISIT(FunDecl)
    std::string name;
    std::vector<std::string> params;
    NodeList body;
    FunDeclNode(std::string n, std::vector<std::string> p, NodeList b) : name(std::move(n)), params(std::move(p)), body(std::move(b)) {}
};

struct IfNode : Node {
    VISIT(If)
    NodePtr condition;
    NodeList body;
    NodeList elseBody;
    IfNode(NodePtr c, NodeList b, NodeList e) : condition(std::move(c)), body(std::move(b)), elseBody(std::move(e)) {}
};

struct LoopNode : Node {
    VISIT(Loop)
    NodeList init;
    NodePtr condition;
    NodeList update;
    NodeList body;
    LoopNode(NodeList i, NodePtr c, NodeList u, NodeList b)
        : init(std::move(i)), condition(std::move(c)), update(std::move(u)), body(std::move(b)) {}
};

struct WhileNode : Node {
    VISIT(While)
    NodePtr condition;
    NodeList body;
    WhileNode(NodePtr c, NodeList b) : condition(std::move(c)), body(std::move(b)) {}
};

struct PrintNode : Node {
    VISIT(Print)
    NodeList args;
    NodePtr end;
    NodePtr sep;
    bool isDebug;
    PrintNode(NodeList a, NodePtr e = nullptr, NodePtr s = nullptr, bool d = false)
        : args(std::move(a)), end(std::move(e)), sep(std::move(s)), isDebug(d) {}
};

struct ReturnNode : Node {
    VISIT(Return)
    NodePtr value;
    explicit ReturnNode(NodePtr v = nullptr) : value(std::move(v)) {}
};

struct TryCatchNode : Node {
    VISIT(TryCatch)
    NodeList tryBlock;
    std::string catchVar;
    NodeList catchBlock;
    NodeList finallyBlock;
    TryCatchNode(NodeList t, std::string c, NodeList cb, NodeList f)
        : tryBlock(std::move(t)), catchVar(std::move(c)), catchBlock(std::move(cb)), finallyBlock(std::move(f)) {}
};

struct ThrowNode : Node {
    VISIT(Throw)
    NodePtr value;
    explicit ThrowNode(NodePtr v) : value(std::move(v)) {}
};

struct BreakNode : Node {
    VISIT(Break)
    std::string label;
    explicit BreakNode(std::string l = "") : label(std::move(l)) {}
};

struct ContinueNode : Node {
    VISIT(Continue)
    std::string label;
    explicit ContinueNode(std::string l = "") : label(std::move(l)) {}
};