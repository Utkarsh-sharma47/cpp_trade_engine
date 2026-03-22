#pragma once

#include <iostream>
#include <cstring>

#define LIKELY(x) __builtin_expect(!!(x), 1)
#define UNLIKELY(x) __builtin_expect(!!(x), 0)
using namespace std;

inline auto ASSERT(bool cond , const string &message) {
    if (UNLIKELY(!cond)) {
        cout << "Assertion failed: " << message << endl;
        exit(EXIT_FAILURE);
    }
}