#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>
#include <cctype>
#include <stdexcept>
#include "split.h"

enum class TokenType {
    BOOL, INT, FLOAT, CHAR, STRING,
    LOWER_NAME, SNAKE_NAME, UPPER_NAME,
    BLOCK_OPEN, BLOCK_CLOSE,
    KEYWORD, OPER, COMMA
};

struct Token {
    TokenType type;
    std::string value;
};

void printToken(const Token& token) {
    static const std::string type_strings[] = {
        "BOOL", "INT", "FLOAT", "CHAR", "STRING",
        "LOWER_NAME", "SNAKE_NAME", "UPPER_NAME",
        "BLOCK_OPEN", "BLOCK_CLOSE",
        "KEYWORD", "OPER", "COMMA"
    };

    std::string value_str = token.value;
    std::cout << "Token { type: " << type_strings[static_cast<int>(token.type)]
    << ", value: \"" << value_str << "\" }\n";
}

TokenType determineIdentifierType(const std::string& str) {
    if (str == "True" || str == "False") return TokenType::BOOL;
    static const std::unordered_set<std::string> keywords = {"on", "fn"};
    if (keywords.count(str)) return TokenType::KEYWORD;

    bool has_upper = std::any_of(str.begin(), str.end(), ::isupper);
    bool has_lower = std::any_of(str.begin(), str.end(), ::islower);
    bool has_under = str.find('_') != std::string::npos;

    if (has_upper && !has_lower) return TokenType::UPPER_NAME;
    if (has_under && has_lower) return TokenType::SNAKE_NAME;
    return TokenType::LOWER_NAME;
}

TokenType determineLiteralType(const std::string& str) {
    if (str[0] == '\'') return TokenType::CHAR;
    if (str[0] == '"')  return TokenType::STRING;

    size_t start = (str[0] == '-' && str.length() > 1) ? 1 : 0;
    size_t dot_count = std::count(str.begin() + start, str.end(), '.');
    bool all_digits = std::all_of(str.begin() + start, str.end(), [](char c) {
        return std::isdigit(c) || c == '.';
    });

    if (all_digits && dot_count <= 1 && str.length() > start + dot_count) {
        return (dot_count == 1) ? TokenType::FLOAT : TokenType::INT;
    }

    if (str == "{" || str == "(" || str == "[") return TokenType::BLOCK_OPEN;
    if (str == "}" || str == ")" || str == "]") return TokenType::BLOCK_CLOSE;
    if (str == ",") return TokenType::COMMA;

    return TokenType::OPER;
}

std::vector<Token> tokenize(const std::vector<std::string>& v) {
    std::vector<Token> tokens;
    tokens.reserve(v.size());

    for (const auto& token_str : v) {
        if (token_str.empty()) continue;

        TokenType type;
        if (std::isalpha(token_str[0]) || token_str[0] == '_') {
            type = determineIdentifierType(token_str);
        } else {
            type = determineLiteralType(token_str);
        }

        tokens.push_back({type, token_str});
    }
    return tokens;
}

struct SymbolTable {
    std::unordered_map<std::string, size_t> string_to_id;
    std::unordered_map<std::string, size_t> name_to_id;
    std::unordered_map<std::string, size_t> keyword_to_id = {{"on", 0}, {"fn", 1}};

    std::vector<std::string> id_to_string;
    std::vector<std::string> id_to_name;
    std::vector<std::string> id_to_keyword = {"on", "fn"};

    SymbolTable() {}

    SymbolTable(std::vector<Token> tokens) {
        for (const auto& token : tokens) {
            switch (token.type) {
                case TokenType::STRING: {
                    if (string_to_id.find(token.value) == string_to_id.end()) {
                        size_t id = id_to_string.size();
                        string_to_id[token.value] = id;
                        id_to_string.push_back(token.value);
                    }
                    break;
                }
                case TokenType::LOWER_NAME:
                case TokenType::SNAKE_NAME:
                case TokenType::UPPER_NAME: {
                    if (name_to_id.find(token.value) == name_to_id.end()) {
                        size_t id = id_to_name.size();
                        name_to_id[token.value] = id;
                        id_to_name.push_back(token.value);
                    }
                    break;
                }
                default:
                    break;
            }
        }
    }
};
