#pragma once
#include <vector>
#include <string>
#include <regex>

std::vector<std::string> split(const std::string& str) {
    std::vector<std::string> tokens;

    // Pattern matches strings, identifiers, numbers, operators, and punctuation
    std::regex token_regex(R"("[^"\\]*(?:\\.[^"\\]*)*"|'[^'\\]*(?:\\.[^'\\]*)*'|-?\d+(?:\.\d+)?|\w+|[={}\[\](),])");

    auto words_begin = std::sregex_iterator(str.begin(), str.end(), token_regex);
    auto words_end = std::sregex_iterator();

    for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
        tokens.push_back(i->str());
    }

    return tokens;
}
