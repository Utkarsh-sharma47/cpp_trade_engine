#pragma once
#include "LFQueue.h"
#include "DMpool.h"
#include "Logger.h"
#include "Order.h"
#include <vector> // MODIFICATION: Changed from array to vector to use Heap Memory
#include <algorithm>

// Assume price range 0 to 1,000,000 ticks (e.g., $0.00 to $10,000.00)
constexpr size_t MAX_PRICE_TICKS = 100000;

class MatchingEngine {
private:
    LFQueue<Order>& order_queue;
    Logger& logger;
    MemPool<Order> order_pool;
    std::atomic<bool> running;

    // FIX 1: Flat Array Look-Up Tables moved to std::vector
    // This dynamically allocates the 16MB of pointers on the Heap, avoiding Stack Overflow
    std::vector<Order*> bid_book;
    std::vector<Order*> ask_book;

    // Tracking the best bid and best ask for fast matching
    uint32_t best_bid = 0;
    uint32_t best_ask = MAX_PRICE_TICKS - 1;

public:
    // FIX 2: Initialize the vectors with MAX_PRICE_TICKS size filled with nullptrs
    MatchingEngine(LFQueue<Order>& in_queue, Logger& log) 
        : order_queue(in_queue), logger(log), order_pool(1000000), running(true),
          bid_book(MAX_PRICE_TICKS, nullptr), ask_book(MAX_PRICE_TICKS, nullptr) {}

    void stop() {
        running.store(false, std::memory_order_release);
    }

    void run() {
        Order incoming_order;
        while (running.load(std::memory_order_acquire)) {
            if (order_queue.pop(incoming_order)) {
                processOrder(incoming_order);
            }
        }
    }

    // FIX 3: Moved processOrder to PUBLIC so the benchmark loop can call it
    void processOrder(Order& order) {
        if (order.side == 'B') {
            matchAggressiveBuy(order);
            if (order.quantity > 0) {
                addOrderToBook(order, bid_book, best_bid, true);
            }
        } else {
            matchAggressiveSell(order);
            if (order.quantity > 0) {
                addOrderToBook(order, ask_book, best_ask, false);
            }
        }
    }

private:
    void matchAggressiveBuy(Order& aggressive) {
        while (aggressive.quantity > 0 && best_ask <= aggressive.price_tick) {
            Order* passive = ask_book[best_ask];
            
            while (passive != nullptr && aggressive.quantity > 0) {
                uint32_t trade_qty = std::min(aggressive.quantity, passive->quantity);
                aggressive.quantity -= trade_qty;
                passive->quantity -= trade_qty;

                if (passive->quantity == 0) {
                    Order* to_delete = passive;
                    passive = passive->next;
                    
                    ask_book[best_ask] = passive;
                    if (passive) passive->prev = nullptr;
                    
                    order_pool.deallocate(to_delete);
                }
            }

            if (ask_book[best_ask] == nullptr) {
                best_ask++; 
            }
        }
    }

    void matchAggressiveSell(Order& aggressive) {
        while (aggressive.quantity > 0 && best_bid >= aggressive.price_tick) {
            Order* passive = bid_book[best_bid];
            
            while (passive != nullptr && aggressive.quantity > 0) {
                uint32_t trade_qty = std::min(aggressive.quantity, passive->quantity);
                aggressive.quantity -= trade_qty;
                passive->quantity -= trade_qty;

                if (passive->quantity == 0) {
                    Order* to_delete = passive;
                    passive = passive->next;
                    
                    bid_book[best_bid] = passive;
                    if (passive) passive->prev = nullptr;
                    
                    order_pool.deallocate(to_delete);
                }
            }

            if (bid_book[best_bid] == nullptr && best_bid > 0) {
                best_bid--; 
            }
        }
    }

    // FIX 4: Changed parameter from std::array to std::vector
    void addOrderToBook(const Order& order, std::vector<Order*>& book, uint32_t& best_price, bool is_bid) {
        Order* new_order = order_pool.allocate(order.system_id, order.price_tick, order.quantity, order.side);
        
        Order* head = book[order.price_tick];
        if (head == nullptr) {
            book[order.price_tick] = new_order;
        } else {
            Order* tail = head;
            while (tail->next != nullptr) tail = tail->next; 
            tail->next = new_order;
            new_order->prev = tail;
        }

        if (is_bid && order.price_tick > best_bid) best_bid = order.price_tick;
        if (!is_bid && order.price_tick < best_ask) best_ask = order.price_tick;
    }
};