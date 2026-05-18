#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <unordered_set>
#include "tokenizer.h"

struct Ast {
    std::vector<Ast*> children;
    Ast() {}
    Ast(std::vector<Ast*> children) : children(std::move(children)) {}
};

