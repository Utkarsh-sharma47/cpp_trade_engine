#pragma once
#include "LFQueue.h"
#include "DMPool.h"
#include "Logger.h"
#include "Order.h"
#include <array>
#include <algorithm>

// Assume price range 0 to 1,000,000 ticks (e.g., $0.00 to $10,000.00)
constexpr size_t MAX_PRICE_TICKS = 1000000;

class MatchingEngine {
private:
    LFQueue<Order>& order_queue;
    Logger& logger;
    MemPool<Order> order_pool;
    std::atomic<bool> running;

    // Flat Array Look-Up Tables (LUT)
    // Points to the HEAD of the order linked-list at that specific price
    std::array<Order*, MAX_PRICE_TICKS> bid_book = {nullptr};
    std::array<Order*, MAX_PRICE_TICKS> ask_book = {nullptr};

    // Tracking the best bid and best ask for fast matching
    uint32_t best_bid = 0;
    uint32_t best_ask = MAX_PRICE_TICKS - 1;

public:
    MatchingEngine(LFQueue<Order>& in_queue, Logger& log) 
        : order_queue(in_queue), logger(log), order_pool(100000), running(true) {}

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

private:
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

    void matchAggressiveBuy(Order& aggressive) {
        // Walk up the ask book starting from the best (lowest) ask
        while (aggressive.quantity > 0 && best_ask <= aggressive.price_tick) {
            Order* passive = ask_book[best_ask];
            
            while (passive != nullptr && aggressive.quantity > 0) {
                uint32_t trade_qty = std::min(aggressive.quantity, passive->quantity);
                aggressive.quantity -= trade_qty;
                passive->quantity -= trade_qty;

                if (passive->quantity == 0) {
                    Order* to_delete = passive;
                    passive = passive->next;
                    
                    // Remove from linked list
                    ask_book[best_ask] = passive;
                    if (passive) passive->prev = nullptr;
                    
                    order_pool.deallocate(to_delete);
                }
            }

            // If the level is empty, move the best_ask up
            if (ask_book[best_ask] == nullptr) {
                best_ask++; // Very fast integer increment
            }
        }
    }

    void matchAggressiveSell(Order& aggressive) {
        // Walk down the bid book starting from the best (highest) bid
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

            // If the level is empty, move the best_bid down
            if (bid_book[best_bid] == nullptr && best_bid > 0) {
                best_bid--; 
            }
        }
    }

    void addOrderToBook(const Order& order, std::array<Order*, MAX_PRICE_TICKS>& book, uint32_t& best_price, bool is_bid) {
        Order* new_order = order_pool.allocate(order.system_id, order.price_tick, order.quantity, order.side);
        
        // O(1) Insertion at the tail of the linked list for time-priority (FIFO)
        Order* head = book[order.price_tick];
        if (head == nullptr) {
            book[order.price_tick] = new_order;
        } else {
            Order* tail = head;
            while (tail->next != nullptr) tail = tail->next; // Can be optimized by storing tail pointers too
            tail->next = new_order;
            new_order->prev = tail;
        }

        // Update best prices
        if (is_bid && order.price_tick > best_bid) best_bid = order.price_tick;
        if (!is_bid && order.price_tick < best_ask) best_ask = order.price_tick;
    }
};