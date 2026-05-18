#pragma once
#include <iostream>
#include <stdint>

using Fun = uint64_t(*)(uint64_t*& p);
Fun funs[256] = {

};
uint64_t sum(uint64_t*& p) {
    uint64_t a = funs[*p](++p);
    uint64_t b = funs[*p](++p);
    return a + b;
}
uint64_t sub(uint64_t*& p) {
    uint64_t a = funs[*p](++p);
    uint64_t b = funs[*p](++p);
    return a - b;
}
