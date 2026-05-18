#include "tokenizer.h"

int main() {
    // Use raw string literal to avoid escape sequence confusion
    std::string text = R"(on a = (True + b.a) * [1.0, 0.2, 3] == ~1 + 'hg')";

    auto split = splitAndKeep(text, "+-*/=.'\"~(),[]", " ");
    auto joined = joinOperators(joinFloats(joinLiterals(split)));
    std::vector<Token> tokens = tokenize(joined);

    for (const Token& t : tokens) {
        printToken(t);
    }

    return 0;
}
