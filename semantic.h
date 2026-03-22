// ============================================================
// MODULE 4: SEMANTIC ANALYZER (Basic)
// File: semantic.h
// Purpose: Type checking and use-before-declare validation
// Note: Core checks are woven into the parser; this module
//       provides a standalone pass for additional checks.
// ============================================================

#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "lexer.h"
#include "symbol_table.h"
#include <iostream>

class SemanticAnalyzer {
public:
    SemanticAnalyzer(SymbolTable& symTable)
        : symTable(symTable), errorCount(0) {}

    // ---- Check if a variable is declared before use ----
    bool checkDeclared(const std::string& name, int line) {
        if (!symTable.exists(name)) {
            std::cerr << "[Semantic] ERROR line " << line
                      << ": Variable '" << name << "' used before declaration.\n";
            errorCount++;
            return false;
        }
        return true;
    }

    // ---- Basic type compatibility check ----
    // Returns the result type of combining two types with an operator
    std::string checkTypes(const std::string& t1, const std::string& t2,
                           const std::string& op) {
        // TODO: Expand with full type compatibility matrix
        if (t1 == t2) return t1;

        if ((t1 == "int" && t2 == "float") ||
            (t1 == "float" && t2 == "int")) {
            std::cout << "[Semantic] Note: Implicit int->float conversion "
                      << "for operator '" << op << "'\n";
            return "float"; // Promote to float
        }

        std::cerr << "[Semantic] ERROR: Type mismatch: '"
                  << t1 << "' and '" << t2 << "' with operator '"
                  << op << "'\n";
        errorCount++;
        return "error";
    }

    // ---- Check assignment compatibility ----
    bool checkAssignTypes(const std::string& lhsType,
                          const std::string& rhsType, int line) {
        if (lhsType == rhsType) return true;
        if (lhsType == "float" && rhsType == "int") return true; // widening

        std::cerr << "[Semantic] ERROR line " << line
                  << ": Cannot assign '" << rhsType
                  << "' to '" << lhsType << "'\n";
        errorCount++;
        return false;
    }

    // Report summary
    void report() {
        std::cout << "\n===== SEMANTIC ANALYSIS =====\n";
        if (errorCount == 0)
            std::cout << "[Semantic] No errors found.\n";
        else
            std::cout << "[Semantic] " << errorCount << " error(s) found.\n";

        // TODO: Add warning for unused variables
        // TODO: Add return type checking for functions
        std::cout << "=============================\n\n";
    }

private:
    SymbolTable& symTable;
    int errorCount;
};

#endif // SEMANTIC_H
