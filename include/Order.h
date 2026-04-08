#pragma once
#include <cstdint>

struct Order {
    uint64_t system_id;
    uint32_t price_tick; // Integer representation of price for O(1) array lookups
    uint32_t quantity;
    char side;           // 'B' for Buy, 'S' for Sell

    // Intrusive linked-list pointers for instantly adding/removing orders
    Order* next = nullptr;
    Order* prev = nullptr;

    Order() : system_id(0), price_tick(0), quantity(0), side('B') {}

    Order(uint64_t id, uint32_t p, uint32_t q, char s)
        : system_id(id), price_tick(p), quantity(q), side(s) {}
};