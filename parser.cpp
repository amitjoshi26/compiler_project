// ============================================================
// MODULE 2: SYNTAX ANALYZER (Recursive Descent Parser)
// File: parser.cpp
// Grammar supported:
//   program      → statement*
//   statement    → decl_stmt | assign_stmt
//   decl_stmt    → type IDENTIFIER ';'
//                | type IDENTIFIER '=' expr ';'
//   assign_stmt  → IDENTIFIER '=' expr ';'
//   expr         → term (('+' | '-') term)*
//   term         → factor (('*' | '/') factor)*
//   factor       → NUMBER | IDENTIFIER | '(' expr ')'
// ============================================================

#include "parser.h"
#include <iostream>
#include <stdexcept>

// ============================================================
// Constructor — takes the token list from the lexer
// ============================================================
Parser::Parser(const std::vector<Token>& tokens, SymbolTable& symTable)
    : tokens(tokens), pos(0), symTable(symTable) {}

// ---- Token navigation helpers ----

Token Parser::peek() {
    return tokens[pos];
}

Token Parser::advance() {
    Token t = tokens[pos];
    if (t.type != TK_EOF) pos++;
    return t;
}

// Expect a specific token; throw error if it doesn't match
Token Parser::expect(TokenType type, const std::string& val) {
    Token t = peek();
    if (t.type != type || (!val.empty() && t.value != val)) {
        std::string msg = "Syntax Error at line " + std::to_string(t.line)
            + ": Expected '" + val + "' but got '" + t.value + "'";
        throw std::runtime_error(msg);
    }
    return advance();
}

// Check if the current token matches type (and optional value)
bool Parser::match(TokenType type, const std::string& val) {
    Token t = peek();
    if (t.type == type && (val.empty() || t.value == val)) {
        advance();
        return true;
    }
    return false;
}

bool Parser::isType(const std::string& val) {
    return val == "int" || val == "float" || val == "void";
}

// ============================================================
// Parse entry point — parse all statements
// ============================================================
void Parser::parse() {
    std::cout << "\n===== SYNTAX ANALYSIS =====\n";
    try {
        while (peek().type != TK_EOF) {
            parseStatement();
        }
        std::cout << "[Parser] Parsing completed successfully.\n";
    } catch (const std::exception& e) {
        std::cerr << e.what() << "\n";
    }
    std::cout << "===========================\n\n";
}

// ============================================================
// Parse a single statement
// ============================================================
void Parser::parseStatement() {
    Token t = peek();

    // Declaration: int x; or int x = expr;
    if (t.type == TK_KEYWORD && isType(t.value)) {
        parseDeclStatement();
    }
    // Assignment: x = expr;
    else if (t.type == TK_IDENTIFIER) {
        parseAssignStatement();
    }
    else {
        // Unknown statement — skip and warn
        std::cerr << "[Parser] Warning: Unexpected token '"
                  << t.value << "' at line " << t.line << ". Skipping.\n";
        advance();
    }
}

// ============================================================
// Parse variable declaration: type IDENTIFIER [= expr] ;
// ============================================================
void Parser::parseDeclStatement() {
    Token typeToken = advance();            // consume type keyword
    Token nameToken = expect(TK_IDENTIFIER, ""); // variable name

    std::cout << "[Parser] Declaration: " << typeToken.value
              << " " << nameToken.value << "\n";

    // Register in symbol table
    symTable.insert(nameToken.value, typeToken.value, nameToken.line);

    // Optional initializer: = expr
    if (peek().value == "=") {
        advance(); // consume '='
        std::string initVal = parseExpr();
        std::cout << "[Parser]   Initialized with: " << initVal << "\n";
    }

    expect(TK_SYMBOL, ";"); // require semicolon
}

// ============================================================
// Parse assignment: IDENTIFIER = expr ;
// ============================================================
void Parser::parseAssignStatement() {
    Token nameToken = advance(); // consume identifier

    // Check declared before use (Semantic check embedded here)
    if (!symTable.exists(nameToken.value)) {
        std::cerr << "[Semantic] Error: Variable '" << nameToken.value
                  << "' used before declaration (line "
                  << nameToken.line << ")\n";
    }

    expect(TK_OPERATOR, "=");
    std::cout << "[Parser] Assignment: " << nameToken.value << " = ";
    std::string val = parseExpr();
    std::cout << val << "\n";

    expect(TK_SYMBOL, ";");
}

// ============================================================
// Parse expression: term ((+ | -) term)*
// ============================================================
std::string Parser::parseExpr() {
    std::string left = parseTerm();

    while (peek().value == "+" || peek().value == "-") {
        std::string op = advance().value;
        std::string right = parseTerm();

        // Generate TAC for this operation
        std::string temp = tac.emit(left, op, right);
        left = temp;
    }

    return left;
}

// ============================================================
// Parse term: factor ((* | /) factor)*
// ============================================================
std::string Parser::parseTerm() {
    std::string left = parseFactor();

    while (peek().value == "*" || peek().value == "/") {
        std::string op = advance().value;
        std::string right = parseFactor();

        std::string temp = tac.emit(left, op, right);
        left = temp;
    }

    return left;
}

// ============================================================
// Parse factor: NUMBER | IDENTIFIER | ( expr )
// ============================================================
std::string Parser::parseFactor() {
    Token t = peek();

    if (t.type == TK_NUMBER) {
        advance();
        return t.value;
    }

    if (t.type == TK_IDENTIFIER) {
        advance();
        // Semantic: check declared
        if (!symTable.exists(t.value)) {
            std::cerr << "[Semantic] Error: Variable '" << t.value
                      << "' undeclared at line " << t.line << "\n";
        }
        return t.value;
    }

    if (t.value == "(") {
        advance();                  // consume '('
        std::string val = parseExpr();
        expect(TK_SYMBOL, ")");     // consume ')'
        return val;
    }

    // TODO: Handle unary minus, function calls
    throw std::runtime_error(
        "Syntax Error at line " + std::to_string(t.line)
        + ": Unexpected '" + t.value + "' in expression");
}

// ---- Expose the TAC generator for printing ----
TACGenerator& Parser::getTAC() { return tac; }
