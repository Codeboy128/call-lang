#pragma once
#include <iostream>
#include <string>
#include <vector>

std::vector<std::string> splitAndKeep(const std::string& str, const std::string& keep_delims, const std::string& drop_delims) {
    std::vector<std::string> result;
    std::string all_delims = keep_delims + drop_delims;
    size_t start = 0;
    size_t end = str.find_first_of(all_delims, start);

    while (end != std::string::npos) {
        // Add the substring before the delimiter
        if (end > start) {
            result.push_back(str.substr(start, end - start));
        }

        // If the delimiter is one we want to keep, add it as a separate token
        char found_char = str[end];
        if (keep_delims.find(found_char) != std::string::npos) {
            result.push_back(std::string(1, found_char));
        }

        start = end + 1;
        end = str.find_first_of(all_delims, start);
    }

    // Add the remainder of the string
    if (start < str.length()) {
        result.push_back(str.substr(start));
    }

    return result;
}

std::vector<std::string> joinLiterals(std::vector<std::string> v) {
    std::vector<std::string> result;
    bool inside_quote = false;
    char quote_char = '\0';
    std::string current_literal = "";

    for (const auto& token : v) {
        if (!inside_quote) {
            // Check if token starts a string literal
            if (token == "\"" || token == "'") {
                inside_quote = true;
                quote_char = token[0];
                current_literal = token;
            } else {
                result.push_back(token);
            }
        } else {
            // Append token to the ongoing literal
            current_literal += token;

            // Check if token closes the string literal
            if (token.length() == 1 && token[0] == quote_char) {
                result.push_back(current_literal);
                inside_quote = false;
                quote_char = '\0';
            }
        }
    }

    // Handle mismatched/unclosed quotes
    if (inside_quote) {
        result.push_back(current_literal);
    }

    return result;
}

std::vector<std::string> joinFloats(const std::vector<std::string>& tokens) {
    std::vector<std::string> merged;
    if (tokens.empty()) return merged;

    for (size_t i = 0; i < tokens.size(); ++i) {
        // Check if current token is a dot and has neighbors
        if (tokens[i] == "." && !merged.empty() && (i + 1 < tokens.size())) {
            const std::string& prev = merged.back();
            const std::string& next = tokens[i + 1];

            // Verify both neighbors are numbers
            if (!prev.empty() && std::isdigit(prev.back()) &&
                !next.empty() && std::isdigit(next.front())) {

                // Merge into the previous token
                merged.back() += "." + next;
            ++i; // Skip the next token since it was consumed
            continue;
                }
        }
        merged.push_back(tokens[i]);
    }
    return merged;
}

const std::unordered_set<std::string> operators = {
    "+", "-", "*", "/", "%", "**", "//", "=", "+=", "-=", "*=",
    "/=", "%=", "//=", "**=", "==", "!=", "<", ">", "<=", ">=",
    "&", "|", "^", "<<", ">>", "~", "."
};

std::vector<std::string> joinOperators(const std::vector<std::string>& tokens) {
    std::vector<std::string> merged;
    size_t i = 0;

    while (i < tokens.size()) {
        std::string current = tokens[i];

        // If current token isn't an operator, keep it and move on
        if (operators.find(current) == operators.end()) {
            merged.push_back(current);
            i++;
            continue;
        }

        // Look ahead to build the longest possible valid operator
        while (i + 1 < tokens.size()) {
            std::string next = tokens[i + 1];
            std::string combined = current + next;

            // If combined string is a valid multi-char operator, merge them
            if (operators.find(combined) != operators.end()) {
                current = combined;
                i++;
            } else {
                break;
            }
        }

        merged.push_back(current);
        i++;
    }

    return merged;
}

