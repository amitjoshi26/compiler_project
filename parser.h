// ============================================================
// FILE: parser.h
// Header for Syntax Analyzer + TAC Generator
// ============================================================

#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "symbol_table.h"
#include "tac.h"

class Parser {
public:
    Parser(const std::vector<Token>& tokens, SymbolTable& symTable);
    void parse();
    TACGenerator& getTAC();

private:
    std::vector<Token> tokens;
    size_t             pos;
    SymbolTable&       symTable;
    TACGenerator       tac;

    // Navigation
    Token peek();
    Token advance();
    Token expect(TokenType type, const std::string& val);
    bool  match(TokenType type, const std::string& val = "");
    bool  isType(const std::string& val);

    // Grammar rules
    void        parseStatement();
    void        parseDeclStatement();
    void        parseAssignStatement();
    std::string parseExpr();
    std::string parseTerm();
    std::string parseFactor();
};

#endif // PARSER_H
