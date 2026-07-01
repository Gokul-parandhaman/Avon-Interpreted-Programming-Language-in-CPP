# 🚀 Nova Programming Language

<div align="center">

**A complete, Python-inspired programming language with bytecode VM, 6 data structures, and full control flow - built in C++**

[![C++](https://img.shields.io/badge/C++-17-blue.svg)](https://isocpp.org/)
[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![Build](https://img.shields.io/badge/build-passing-brightgreen.svg)](https://github.com/Breddy67/Nova)
[![Lines](https://img.shields.io/badge/lines-4300%2B-blue.svg)](https://github.com/Breddy67/Nova)

</div>

---

## 📌 About Nova

**Nova** is a modern programming language with dynamic and static typing, built from scratch in C++17. It features a custom lexer, parser, AST, bytecode compiler, and stack-based virtual machine.

**Key Features:**
- Python-inspired syntax
- 6 built-in data structures (List, Map, Stack, Queue, Heap, LinkedList)
- Functions with recursion (Factorial, Sum_to working)
- Try/Catch/Throw error handling
- Slicing syntax `[start:end:step]`
- Static & Dynamic typing
- REPL environment
- Bytecode debugging

---

## 🚀 Quick Start

```bash
git clone https://github.com/Breddy67/Nova.git
cd Nova
g++ -std=c++17 -I. -o nova_new.exe src/Lexer.cpp src/Parser.cpp src/bytecode/BytecodeCompiler.cpp src/bytecode/NovaVM.cpp src/main.cpp
./nova_new.exe Tests/EX23.nv
```

### REPL Commands

| Command | Purpose |
|---------|---------|
| `exit` / `quit` | Exit the REPL |
| `debug` | Toggle debug mode |

---

## 📝 Language Syntax

### Variables

```nova
# Dynamic typing
var x = 10
var name = "Nova"

# Static typing
number x = 10
string name = "Nova"
bool flag = true
```

### Print

```nova
print("Hello, World!")
print("Numbers:", 10, 20, 30, sep=", ", end="!\n")
debug("Variable x =", x)
```

### Control Flow

```nova
if (x > 10) {
    print("big")
} else if (x > 5) {
    print("medium")
} else {
    print("small")
}

loop (i = 0; i < 10; i = i + 1) {
    print(i)
}

```

### Functions(Including Rudimentary Recursion)

```nova
fun add(a, b) {
    return a + b
}

fun factorial(n) {
    if (n <= 1) return 1
    return n * factorial(n - 1)
}

print(factorial(5))  # 120
```

### Error Handling

```nova
try {
    throw("Something went wrong!")
} catch (err) {
    print("Error:", err)
}
```

---

## 📚 Data Structures

### Bunch (List)

```nova
var list = {1, 2, 3, 4}
bunch_push(list, 5)
print(list[0])        # 1
print(list[0:3])      # [1, 2, 3]
```

### Map (Dictionary)

```nova
var user = map()
map_set(user, "name", "Alice")
print(map_get(user, "name"))
```

### Stack (LIFO)

```nova
var s = stack()
stack_push(s, 10)
stack_push(s, 20)
print(stack_pop(s))   # 20
```

### Queue (FIFO)

```nova
var q = queue()
queue_enqueue(q, "first")
queue_enqueue(q, "second")
print(queue_dequeue(q))  # first
```

### Heap (Min-Heap)

```nova
var h = heap()
heap_push(h, 5)
heap_push(h, 3)
print(heap_pop(h))    # 3
```

### LinkedList

```nova
var ll = linkedlist()
ll_push_back(ll, 10)
ll_push_front(ll, 20)
print(ll_get(ll, 0))  # 20
```

---

## 🏗️ Architecture

```
Source -> Lexer -> Tokens -> Parser -> AST -> Compiler -> Bytecode -> VM -> Output
```

| Component | File | Description |
|-----------|------|-------------|
| Lexer | `Lexer.cpp/h` | Tokenizes source code |
| Parser | `Parser.cpp/h` | Builds AST |
| AST | `AST.h` | AST nodes and data structures |
| Compiler | `BytecodeCompiler.cpp/h` | AST → Bytecode |
| Program | `BytecodeProgram.h` | Bytecode storage |
| OpCodes | `OpCodes.h` | Opcode definitions |
| VM | `NovaVM.cpp/h` | Executes bytecode |
| Environment | `Environment.h` | Variable management |

---

## 🔧 Keywords

```
var  number  string  bool  null  bunch  map  stack  queue  heap  linkedlist
fun  return  if  else  loop  while  break  continue  print  debug
try  catch  throw  and  or  not
```

---

## 📄 License

MIT License
