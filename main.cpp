// main.cpp
#include "parser.h"

int main() {
    std::string code = R"(
    a = True
    on start(a, b) {
        SUM(MUL(b, 3), a)
    }
    fn f(a, b) = SUM(MUL(b, 3), a)
    fn point(x, y) = {
        X = x
        Y = y
    }
    s = "hi"
    s = 'hi'
    arr = [1, -1.1]
    )";

    // Split into tokens
    auto tokens = tokenize(split(code));

    // Print tokens for debugging
    std::cout << "=== Tokens ===\n";
    for (const auto& token : tokens) {
        printToken(token);
    }

    Parser parser(tokens);
    std::unique_ptr<ASTNode> ast = parser.parse();
    printAST(ast.get());

    return 0;
}
