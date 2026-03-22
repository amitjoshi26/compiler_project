// ============================================================
// MODULE 1: LEXICAL ANALYZER
// File: lexer.cpp
// Purpose: Tokenize the input source code into meaningful units
// ============================================================

#include "lexer.h"
#include <cctype>
#include <iostream>

// ---- Keyword list ----
// These are reserved words in our simple language
static const std::vector<std::string> KEYWORDS = {
    "int", "float", "if", "else", "while", "return", "void"
};

// Check if a given string is a keyword
bool isKeyword(const std::string& word) {
    for (const auto& kw : KEYWORDS)
        if (kw == word) return true;
    return false;
}

// Convert TokenType enum to a printable string
std::string tokenTypeName(TokenType t) {
    switch (t) {
        case TK_KEYWORD:    return "KEYWORD";
        case TK_IDENTIFIER: return "IDENTIFIER";
        case TK_NUMBER:     return "NUMBER";
        case TK_OPERATOR:   return "OPERATOR";
        case TK_SYMBOL:     return "SYMBOL";
        case TK_STRING:     return "STRING";
        case TK_EOF:        return "EOF";
        case TK_UNKNOWN:    return "UNKNOWN";
        default:            return "?";
    }
}

// ============================================================
// Lexer constructor — takes the full source code as a string
// ============================================================
Lexer::Lexer(const std::string& source)
    : src(source), pos(0), line(1) {}

// Peek at current character without consuming it
char Lexer::peek() {
    if (pos < src.size()) return src[pos];
    return '\0';
}

// Consume and return the current character
char Lexer::advance() {
    char c = src[pos++];
    if (c == '\n') line++;
    return c;
}

// Skip whitespace and comments
void Lexer::skipWhitespace() {
    while (pos < src.size()) {
        char c = peek();
        if (isspace(c)) { advance(); }
        // Handle single-line comments: // ...
        else if (c == '/' && pos + 1 < src.size() && src[pos + 1] == '/') {
            while (pos < src.size() && peek() != '\n') advance();
        }
        else break;
    }
}

// ============================================================
// Core tokenizer: reads one token at a time
// ============================================================
Token Lexer::nextToken() {
    skipWhitespace();

    if (pos >= src.size())
        return { TK_EOF, "EOF", line };

    char c = peek();

    // ---- Numbers (integer only for now) ----
    if (isdigit(c)) {
        std::string num;
        while (pos < src.size() && isdigit(peek()))
            num += advance();
        // TODO: Add support for floating-point numbers (e.g., 3.14)
        return { TK_NUMBER, num, line };
    }

    // ---- Identifiers and Keywords ----
    if (isalpha(c) || c == '_') {
        std::string word;
        while (pos < src.size() && (isalnum(peek()) || peek() == '_'))
            word += advance();
        TokenType type = isKeyword(word) ? TK_KEYWORD : TK_IDENTIFIER;
        return { type, word, line };
    }

    // ---- String literals ----
    if (c == '"') {
        advance(); // consume opening "
        std::string str;
        while (pos < src.size() && peek() != '"')
            str += advance();
        advance(); // consume closing "
        return { TK_STRING, str, line };
    }

    // ---- Operators (single and double character) ----
    advance(); // consume the character
    std::string op(1, c);
    char next = peek();

    // Two-character operators: ==, !=, <=, >=
    if ((c == '=' && next == '=') || (c == '!' && next == '=') ||
        (c == '<' && next == '=') || (c == '>' && next == '=')) {
        op += advance();
        return { TK_OPERATOR, op, line };
    }

    // Single-character operators
    if (c == '+' || c == '-' || c == '*' || c == '/' ||
        c == '=' || c == '<' || c == '>') {
        return { TK_OPERATOR, op, line };
    }

    // ---- Symbols / Punctuation ----
    if (c == '(' || c == ')' || c == '{' || c == '}' ||
        c == ';' || c == ',' || c == '[' || c == ']') {
        return { TK_SYMBOL, op, line };
    }

    // Unknown character
    return { TK_UNKNOWN, op, line };
}

// ============================================================
// Tokenize the entire source and return the token list
// ============================================================
std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    Token tok;
    do {
        tok = nextToken();
        tokens.push_back(tok);
    } while (tok.type != TK_EOF);
    return tokens;
}

// ============================================================
// Print all tokens in a formatted table
// ============================================================
void printTokens(const std::vector<Token>& tokens) {
    std::cout << "\n===== TOKEN STREAM =====\n";
    std::cout << std::left;
    printf("%-6s %-15s %-12s\n", "LINE", "TYPE", "VALUE");
    printf("%-6s %-15s %-12s\n", "----", "----", "-----");
    for (const auto& tok : tokens) {
        printf("%-6d %-15s %-12s\n",
            tok.line,
            tokenTypeName(tok.type).c_str(),
            tok.value.c_str());
    }
    std::cout << "========================\n\n";
}
