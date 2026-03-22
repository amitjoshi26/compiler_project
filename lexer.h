// ============================================================
// FILE: lexer.h
// Header for Lexical Analyzer
// ============================================================

#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <vector>

// ---- Token Types ----
enum TokenType {
    TK_KEYWORD,
    TK_IDENTIFIER,
    TK_NUMBER,
    TK_OPERATOR,
    TK_SYMBOL,
    TK_STRING,
    TK_EOF,
    TK_UNKNOWN
};

// ---- Token Structure ----
// Each token holds its type, string value, and source line number
struct Token {
    TokenType   type;
    std::string value;
    int         line;
};

// ---- Lexer Class ----
class Lexer {
public:
    Lexer(const std::string& source);
    std::vector<Token> tokenize();  // Tokenize entire source

private:
    std::string src;
    size_t      pos;
    int         line;

    char peek();
    char advance();
    void skipWhitespace();
    Token nextToken();
};

// ---- Utility Functions ----
bool        isKeyword(const std::string& word);
std::string tokenTypeName(TokenType t);
void        printTokens(const std::vector<Token>& tokens);

#endif // LEXER_H
