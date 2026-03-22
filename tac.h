// ============================================================
// MODULE 5: INTERMEDIATE CODE GENERATOR (Three Address Code)
// File: tac.h
// Purpose: Generate TAC instructions for arithmetic expressions
//
// Example:  a = b + c * d
//   t1 = c * d
//   t2 = b + t1
//   a  = t2
// ============================================================

#ifndef TAC_H
#define TAC_H

#include <string>
#include <vector>
#include <iostream>

// ---- A single TAC instruction ----
struct TACInstruction {
    std::string result;  // destination temp or variable
    std::string arg1;    // left operand
    std::string op;      // operator (+, -, *, /)
    std::string arg2;    // right operand (empty for copy)
};

// ---- TAC Generator ----
class TACGenerator {
public:
    TACGenerator() : tempCount(0) {}

    // Create a new temporary variable name: t1, t2, ...
    std::string newTemp() {
        return "t" + std::to_string(++tempCount);
    }

    // Emit a binary operation instruction and return the result temp
    std::string emit(const std::string& arg1, const std::string& op,
                     const std::string& arg2) {
        std::string temp = newTemp();
        instructions.push_back({ temp, arg1, op, arg2 });
        return temp;
    }

    // Emit a copy assignment: result = arg1
    void emitCopy(const std::string& result, const std::string& arg1) {
        instructions.push_back({ result, arg1, "=", "" });
    }

    // Print all generated TAC instructions
    void print() const {
        std::cout << "\n===== THREE ADDRESS CODE =====\n";
        if (instructions.empty()) {
            std::cout << "  (no instructions generated)\n";
        }
        for (const auto& instr : instructions) {
            if (instr.op == "=") {
                // Copy instruction
                printf("  %s = %s\n",
                    instr.result.c_str(), instr.arg1.c_str());
            } else {
                printf("  %s = %s %s %s\n",
                    instr.result.c_str(),
                    instr.arg1.c_str(),
                    instr.op.c_str(),
                    instr.arg2.c_str());
            }
        }
        std::cout << "==============================\n\n";

        // TODO: Add quadruple / triple representation
        // TODO: Add basic block detection
        // TODO: Add control flow for if/while statements
    }

    const std::vector<TACInstruction>& getInstructions() const {
        return instructions;
    }

private:
    int tempCount;
    std::vector<TACInstruction> instructions;
};

#endif // TAC_H
