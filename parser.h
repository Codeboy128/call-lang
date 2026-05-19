#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <stdexcept>
#include "tokenizer.h"

// === AST Node Definitions ===

enum class ASTNodeType {
    PROGRAM, ASSIGNMENT, EVENT_DECL, FUNC_DECL,
    VARIABLE, LITERAL, FUNC_CALL, BLOCK, ARRAY
};

struct ASTNode {
    ASTNodeType type;
    std::string value; // Stores names, literal values, or operator string
    std::vector<std::unique_ptr<ASTNode>> children;

    ASTNode(ASTNodeType t, std::string v = "") : type(t), value(std::move(v)) {}
};

// === Parser Class ===

class Parser {
public:
    explicit Parser(std::vector<Token> tokens) : tokens_(std::move(tokens)), index_(0) {}

    // Main entry point
    std::unique_ptr<ASTNode> parse() {
        auto program = std::make_unique<ASTNode>(ASTNodeType::PROGRAM, "Program");
        while (!isAtEnd()) {
            program->children.push_back(parseStatement());
        }
        return program;
    }

private:
    std::vector<Token> tokens_;
    size_t index_;

    // Helper: Peek at current token
    const Token& peek() const {
        if (isAtEnd()) throw std::runtime_error("Unexpected end of input");
        return tokens_[index_];
    }

    // Helper: Consume and return current token
    Token advance() {
        if (!isAtEnd()) index_++;
        return tokens_[index_ - 1];
    }

    // Helper: Check if file ended
    bool isAtEnd() const {
        return index_ >= tokens_.size();
    }

    // Helper: Match and consume specific token type
    bool match(TokenType type) {
        if (!isAtEnd() && peek().type == type) {
            advance();
            return true;
        }
        return false;
    }

    // Helper: Enforce a token type or throw error
    Token consume(TokenType type, const std::string& message) {
        if (!isAtEnd() && peek().type == type) return advance();
        throw std::runtime_error(message + " Got: " + (isAtEnd() ? "EOF" : peek().value));
    }

    // Identifiers can be lower, snake, or upper case names
    bool isIdentifier(TokenType type) const {
        return type == TokenType::LOWER_NAME ||
        type == TokenType::SNAKE_NAME ||
        type == TokenType::UPPER_NAME;
    }

    // Statements: on, fn, or Assignment
    std::unique_ptr<ASTNode> parseStatement() {
        if (peek().type == TokenType::KEYWORD) {
            if (peek().value == "on") return parseEventDeclaration();
            if (peek().value == "fn") return parseFunctionDeclaration();
        }
        return parseAssignmentOrExpression();
    }

    // Parses: on start(a, b) { ... }
    std::unique_ptr<ASTNode> parseEventDeclaration() {
        consume(TokenType::KEYWORD, "Expected 'on'");

        Token name = advance();
        if (!isIdentifier(name.type)) throw std::runtime_error("Expected event name");

        auto node = std::make_unique<ASTNode>(ASTNodeType::EVENT_DECL, name.value);

        // Parameter list
        consume(TokenType::BLOCK_OPEN, "Expected '('");
        if (peek().value != ")") {
            do {
                Token param = advance();
                if (!isIdentifier(param.type)) throw std::runtime_error("Expected parameter name");
                node->children.push_back(std::make_unique<ASTNode>(ASTNodeType::VARIABLE, param.value));
            } while (match(TokenType::COMMA));
        }
        consume(TokenType::BLOCK_CLOSE, "Expected ')'");

        // Event Body
        node->children.push_back(parseBlock());
        return node;
    }

    // Parses: fn f(a, b) = expression
    std::unique_ptr<ASTNode> parseFunctionDeclaration() {
        consume(TokenType::KEYWORD, "Expected 'fn'");

        Token name = advance();
        if (!isIdentifier(name.type)) throw std::runtime_error("Expected function name");

        auto node = std::make_unique<ASTNode>(ASTNodeType::FUNC_DECL, name.value);

        // Parameter list
        consume(TokenType::BLOCK_OPEN, "Expected '('");
        if (peek().value != ")") {
            do {
                Token param = advance();
                if (!isIdentifier(param.type)) throw std::runtime_error("Expected parameter name");
                node->children.push_back(std::make_unique<ASTNode>(ASTNodeType::VARIABLE, param.value));
            } while (match(TokenType::COMMA));
        }
        consume(TokenType::BLOCK_CLOSE, "Expected ')'");

        // Assignment operator '='
        Token op = consume(TokenType::OPER, "Expected '='");
        if (op.value != "=") throw std::runtime_error("Expected '=' after function signature");

        // Function Body (Can be a single expression or a block)
        if (peek().value == "{") {
            node->children.push_back(parseBlock());
        } else {
            node->children.push_back(parseExpression());
        }
        return node;
    }

    // Parses code inside curly braces { ... }
    std::unique_ptr<ASTNode> parseBlock() {
        consume(TokenType::BLOCK_OPEN, "Expected '{'");
        auto block = std::make_unique<ASTNode>(ASTNodeType::BLOCK, "Block");

        while (peek().value != "}") {
            block->children.push_back(parseStatement());
        }

        consume(TokenType::BLOCK_CLOSE, "Expected '}'");
        return block;
    }

    // Parses: variable = expression OR fallback to isolated expressions
    std::unique_ptr<ASTNode> parseAssignmentOrExpression() {
        // Lookahead checking for variable assignment: ID '='
        if (isIdentifier(peek().type) && index_ + 1 < tokens_.size() && tokens_[index_ + 1].value == "=") {
            Token target = advance(); // Consume variable name
            advance();                // Consume '='

            auto assignNode = std::make_unique<ASTNode>(ASTNodeType::ASSIGNMENT, "=");
            assignNode->children.push_back(std::make_unique<ASTNode>(ASTNodeType::VARIABLE, target.value));
            assignNode->children.push_back(parseExpression());
            return assignNode;
        }
        return parseExpression();
    }

    // Expressions hierarchy (Function calls, Literals, Arrays)
    std::unique_ptr<ASTNode> parseExpression() {
        return parsePrimary();
    }

    // Primary parsing: Literals, Names, Function Calls, Arrays
    std::unique_ptr<ASTNode> parsePrimary() {
        Token token = peek();

        // Handle Literals
        if (token.type == TokenType::INT || token.type == TokenType::FLOAT ||
            token.type == TokenType::BOOL || token.type == TokenType::CHAR ||
            token.type == TokenType::STRING) {
            advance();
        return std::make_unique<ASTNode>(ASTNodeType::LITERAL, token.value);
            }

            // Handle Array Literals: [1, -1.1]
            if (token.type == TokenType::BLOCK_OPEN && token.value == "[") {
                advance();
                auto arrayNode = std::make_unique<ASTNode>(ASTNodeType::ARRAY, "Array");
                if (peek().value != "]") {
                    do {
                        arrayNode->children.push_back(parseExpression());
                    } while (match(TokenType::COMMA));
                }
                consume(TokenType::BLOCK_CLOSE, "Expected ']'");
                return arrayNode;
            }

            // Handle Identifiers / Function Calls
            if (isIdentifier(token.type)) {
                advance();
                // If followed by '(', it's a function call: SUM(...)
                if (!isAtEnd() && peek().type == TokenType::BLOCK_OPEN && peek().value == "(") {
                    advance(); // consume '('
                    auto callNode = std::make_unique<ASTNode>(ASTNodeType::FUNC_CALL, token.value);
                    if (peek().value != ")") {
                        do {
                            callNode->children.push_back(parseExpression());
                        } while (match(TokenType::COMMA));
                    }
                    consume(TokenType::BLOCK_CLOSE, "Expected ')'");
                    return callNode;
                }
                // Otherwise, it's just a variable reference
                return std::make_unique<ASTNode>(ASTNodeType::VARIABLE, token.value);
            }

            throw std::runtime_error("Unexpected token in expression: " + token.value);
    }
};

void printAST(const ASTNode* node, int depth = 0) {
    if (!node) return;

    // Map enum types to readable strings
    static const std::string type_strings[] = {
        "PROGRAM", "ASSIGNMENT", "EVENT_DECL", "FUNC_DECL",
        "VARIABLE", "LITERAL", "FUNC_CALL", "BLOCK", "ARRAY"
    };

    // Print indentation
    for (int i = 0; i < depth; ++i) std::cout << "  ";

    // Print "TYPE: value"
    std::cout << type_strings[static_cast<int>(node->type)]
    << ": " << node->value << "\n";

    // Recursively print children
    for (const auto& child : node->children) {
        printAST(child.get(), depth + 1);
    }
}
