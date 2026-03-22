// ============================================================
// MODULE 3: SYMBOL TABLE
// File: symbol_table.h
// Purpose: Store and look up variable names and their types
// ============================================================

#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <string>
#include <unordered_map>
#include <iostream>

// ---- Symbol Entry ----
// Represents one variable in the symbol table
struct Symbol {
    std::string name;   // Variable name
    std::string type;   // Data type: "int", "float", etc.
    int         line;   // Line where it was declared
    // TODO: Add scope level, memory offset, value tracking
};

// ---- Symbol Table Class ----
class SymbolTable {
public:
    // Insert a new symbol; returns false if already declared
    bool insert(const std::string& name, const std::string& type, int line) {
        if (table.count(name)) {
            std::cerr << "[SymbolTable] Error: '" << name
                      << "' already declared (line " << line << ")\n";
            return false;
        }
        table[name] = { name, type, line };
        return true;
    }

    // Lookup a symbol by name; returns nullptr if not found
    Symbol* lookup(const std::string& name) {
        auto it = table.find(name);
        if (it != table.end()) return &it->second;
        return nullptr;
    }

    // Print the full symbol table contents
    void print() const {
        std::cout << "\n===== SYMBOL TABLE =====\n";
        printf("%-15s %-10s %-6s\n", "NAME", "TYPE", "LINE");
        printf("%-15s %-10s %-6s\n", "----", "----", "----");
        for (const auto& [key, sym] : table) {
            printf("%-15s %-10s %-6d\n",
                sym.name.c_str(), sym.type.c_str(), sym.line);
        }
        std::cout << "========================\n\n";
    }

    // Check if symbol exists
    bool exists(const std::string& name) const {
        return table.count(name) > 0;
    }

    // TODO: Add support for scoped symbol tables (stack of tables)
    // TODO: Add function signatures (return type, parameter list)

private:
    std::unordered_map<std::string, Symbol> table;
};

#endif // SYMBOL_TABLE_H
