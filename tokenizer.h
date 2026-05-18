#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <unordered_set>
#include "split.h"

enum class TokenType {
    INT, FLOAT, CHAR, STRING,
    LOWER_NAME, SNAKE_NAME, UPPER_NAME,
    BLOCK_OPEN, BLOCK_CLOSE,
    KEYWORD, OPER, COMMA, NEWLINE
};

struct Token {
    TokenType type;
    std::string value;
};

void printToken(const Token& token) {
    std::string type_str;

    switch (token.type) {
        case TokenType::INT:         type_str = "INT"; break;
        case TokenType::FLOAT:       type_str = "FLOAT"; break;
        case TokenType::CHAR:        type_str = "CHAR"; break;
        case TokenType::STRING:      type_str = "STRING"; break;
        case TokenType::LOWER_NAME:  type_str = "LOWER_NAME"; break;
        case TokenType::SNAKE_NAME:  type_str = "SNAKE_NAME"; break;
        case TokenType::UPPER_NAME:  type_str = "UPPER_NAME"; break;
        case TokenType::BLOCK_OPEN:  type_str = "BLOCK_OPEN"; break;
        case TokenType::BLOCK_CLOSE: type_str = "BLOCK_CLOSE"; break;
        case TokenType::KEYWORD:     type_str = "KEYWORD"; break;
        case TokenType::OPER:        type_str = "OPER"; break;
        case TokenType::COMMA:       type_str = "COMMA"; break;
        case TokenType::NEWLINE:     type_str = "NEWLINE"; break;
    }

    // Escape newline values visually for clear print output
    std::string value_str = token.value;
    if (token.type == TokenType::NEWLINE) {
        value_str = "\\n";
    }

    std::cout << "Token { type: " << type_str << ", value: \"" << value_str << "\" }\n";
}

std::vector<Token> tokenize(std::vector<std::string> v) {
    std::vector<Token> tokens;

    // Define keywords set for quick lookup
    static const std::unordered_set<std::string> keywords = {
        "on", "True", "False"
    };

    for (const auto& token_str : v) {
        Token token;
        token.value = token_str;

        // Skip empty tokens
        if (token_str.empty()) continue;

        // Check for string literals
        if (token_str.length() >= 2 &&
            ((token_str.front() == '"' && token_str.back() == '"') ||
            (token_str.front() == '\'' && token_str.back() == '\''))) {
            // Distinguish between char and string
            if (token_str.front() == '\'') {
                token.type = TokenType::CHAR;
            } else {
                token.type = TokenType::STRING;
            }
            tokens.push_back(token);
        continue;
            }

            // Check for block open/close
            if (token_str == "{" || token_str == "(" || token_str == "[") {
                token.type = TokenType::BLOCK_OPEN;
                tokens.push_back(token);
                continue;
            }
            if (token_str == "}" || token_str == ")" || token_str == "]") {
                token.type = TokenType::BLOCK_CLOSE;
                tokens.push_back(token);
                continue;
            }

            // Check for comma
            if (token_str == ",") {
                token.type = TokenType::COMMA;
                tokens.push_back(token);
                continue;
            }

            // Check for newline
            if (token_str == "\n") {
                token.type = TokenType::NEWLINE;
                tokens.push_back(token);
                continue;
            }

            // Check for operators
            if (operators.count(token_str)) {
                token.type = TokenType::OPER;
                tokens.push_back(token);
                continue;
            }

            // Check for numeric literals
            bool is_numeric = true;
            bool has_dot = false;
            bool has_digit = false;

            // Handle negative numbers
            size_t start_idx = (token_str[0] == '-' && token_str.length() > 1) ? 1 : 0;

            if (start_idx >= token_str.length()) {
                is_numeric = false;
            } else {
                for (size_t i = start_idx; i < token_str.length(); i++) {
                    char c = token_str[i];
                    if (c == '.') {
                        if (has_dot) {
                            is_numeric = false;
                            break;
                        }
                        has_dot = true;
                    } else if (c >= '0' && c <= '9') {
                        has_digit = true;
                    } else {
                        is_numeric = false;
                        break;
                    }
                }
            }

            if (is_numeric && has_digit) {
                if (has_dot) {
                    token.type = TokenType::FLOAT;
                } else {
                    token.type = TokenType::INT;
                }
                tokens.push_back(token);
                continue;
            }

            // Check for identifiers
            if (!token_str.empty() && (std::isalpha(token_str[0]) || token_str[0] == '_')) {
                // Check if it's a keyword
                if (keywords.count(token_str)) {
                    token.type = TokenType::KEYWORD;
                    tokens.push_back(token);
                    continue;
                }

                // Determine identifier type based on naming convention
                bool has_underscore = token_str.find('_') != std::string::npos;
                bool is_all_upper = true;
                bool has_lower = false;

                for (char c : token_str) {
                    if (std::islower(c)) has_lower = true;
                    if (!std::isupper(c) && c != '_') is_all_upper = false;
                }

                if (is_all_upper && token_str.length() > 0) {
                    token.type = TokenType::UPPER_NAME;
                } else if (has_underscore && has_lower) {
                    token.type = TokenType::SNAKE_NAME;
                } else if (std::islower(token_str[0])) {
                    token.type = TokenType::LOWER_NAME;
                } else {
                    // Default to LOWER_NAME for identifiers that don't fit patterns
                    token.type = TokenType::LOWER_NAME;
                }

                tokens.push_back(token);
                continue;
            }

            // If we get here, treat as operator (catch-all for unrecognized tokens)
            token.type = TokenType::OPER;
            tokens.push_back(token);
    }

    return tokens;
}
