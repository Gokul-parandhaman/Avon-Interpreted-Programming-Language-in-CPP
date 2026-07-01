#pragma once
#include <cstdint>

enum class OpCode : uint8_t {
    // Stack & Arithmetic
    PUSH, POP,
    ADD, SUB, MUL, DIV, MOD, POW, NEGATE,
    
    // Comparison & Logical
    EQ, NEQ, LT, GT, LTE, GTE,
    AND, OR, NOT,
    
    // Variables & Control Flow
    LOAD, STORE, DEFINE,
    JUMP, JUMP_IF, JUMP_IF_NOT,
    
    // Functions
    FUN_DECL, CALL, RETURN,
    
    // Built-ins: Bunch & String
    BUNCH_INIT, BUNCH_PUSH, BUNCH_POP, BUNCH_GET, BUNCH_SIZE, BUNCH_EMPTY,
    STRING_INIT, STRING_SET, STRING_GET, STRING_SIZE, SLICE,
    
    // Built-ins: Map, Stack, Queue
    MAP_INIT, MAP_SET, MAP_GET, MAP_HAS, MAP_SIZE, MAP_KEYS,
    STACK_INIT, STACK_PUSH, STACK_POP, STACK_TOP, STACK_SIZE,
    QUEUE_INIT, QUEUE_ENQUEUE, QUEUE_DEQUEUE, QUEUE_FRONT, QUEUE_SIZE,
    
    // Built-ins: Heap & List
    HEAP_INIT, HEAP_PUSH, HEAP_POP, HEAP_TOP, HEAP_SIZE,
    LIST_INIT, LIST_PUSH_FRONT, LIST_PUSH_BACK, LIST_POP_FRONT, LIST_POP_BACK, LIST_GET, LIST_SIZE,

    // I/O, Error Handling & Terminate
    PRINT,
    THROW, TRY_START, TRY_END, CATCH_START,
    HALT
};