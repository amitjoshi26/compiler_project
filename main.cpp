// ============================================================
// COMPILER FRONTEND — MAIN DRIVER
// File: main.cpp
// Project: Design and Implementation of a Compiler Frontend
// Modules: Lexer → Parser → SymbolTable → Semantic → TAC
// ============================================================

#include <iostream>
#include <fstream>
#include <sstream>

#include "lexer.h"
#include "parser.h"
#include "symbol_table.h"
#include "semantic.h"
#include "tac.h"

// ---- Read source from file ----
std::string readFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open file '" << filename << "'\n";
        return "";
    }
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

int main(int argc, char* argv[]) {
    // ---- Choose source: file argument or inline sample ----
    std::string source;

    if (argc > 1) {
        source = readFile(argv[1]);
        if (source.empty()) return 1;
    } else {
        // Default sample program (used when no file is provided)
        source = R"(
// Sample input program
int a;
int b;
float c;
a = 5;
b = 3;
a = a + b * 2;
c = a + 1;
        )";
        std::cout << "=== Using built-in sample source ===\n";
        std::cout << source << "\n";
    }

    std::cout << "======================================\n";
    std::cout << "   COMPILER FRONTEND — PHASE OUTPUT   \n";
    std::cout << "======================================\n";

    // ============================================================
    // PHASE 1: LEXICAL ANALYSIS
    // ============================================================
    std::cout << "\n[Phase 1] Running Lexical Analyzer...\n";
    Lexer lexer(source);
    std::vector<Token> tokens = lexer.tokenize();
    printTokens(tokens);

    // ============================================================
    // PHASE 2 & 3: PARSING + SYMBOL TABLE BUILD
    // ============================================================
    std::cout << "[Phase 2] Running Syntax Analyzer...\n";
    SymbolTable symTable;
    Parser parser(tokens, symTable);
    parser.parse();

    // ============================================================
    // PHASE 3: PRINT SYMBOL TABLE
    // ============================================================
    std::cout << "[Phase 3] Symbol Table:\n";
    symTable.print();

    // ============================================================
    // PHASE 4: SEMANTIC ANALYSIS REPORT
    // ============================================================
    std::cout << "[Phase 4] Semantic Analysis:\n";
    SemanticAnalyzer sem(symTable);
    sem.report();

    // ============================================================
    // PHASE 5: THREE ADDRESS CODE
    // ============================================================
    std::cout << "[Phase 5] Intermediate Code (TAC):\n";
    parser.getTAC().print();

    std::cout << "=== Compilation Phases Complete ===\n";
    return 0;
}
