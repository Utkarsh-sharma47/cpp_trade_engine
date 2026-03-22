#include <iostream>
#include <string>

// Notice how clean the includes are now! CMake knows to look in core/include/
#include "Logger.h"
#include "DMemPool.h"

// A dummy struct to test your memory pool
struct Order {
    int order_id;
    double price;
    int quantity;
    
    // Default constructor
    Order() : order_id(0), price(0.0), quantity(0) {}
    
    // Parameterized constructor
    Order(int id, double p, int q) : order_id(id), price(p), quantity(q) {}
};

int main() {
    std::cout << "Starting Trading Engine Ecosystem..." << std::endl;

    // 1. Initialize the Async Logger (Logs will go into the 'log' folder)
    Logger engineLogger("log/trade_events.log");
    engineLogger.log("SYSTEM: Logger initialized.");

    // 2. Initialize the Memory Pool (Pre-allocate 1000 Order objects)
    std::cout << "Allocating Memory Pool..." << std::endl;
    common::MemPool<Order> orderPool(1000);
    engineLogger.log("SYSTEM: Memory pool allocated with 1000 slots.");

    // 3. Simulate the Hot Path (Fast Trading)
    std::cout << "Executing trades..." << std::endl;
    
    for (int i = 0; i < 5; i++) {
        // FAST: Get memory from the pool instead of using 'new'
        Order* newOrder = orderPool.allocate(i, 150.50 + i, 100);
        
        // FAST: Drop a message in the lock-free queue and move on instantly
        engineLogger.log("Executed Trade ID: " + std::to_string(newOrder->order_id));
        
        // FAST: Return the memory to the pool when done
        orderPool.deallocate(newOrder);
    }

    std::cout << "System shutting down safely. Check log/trade_events.log." << std::endl;
    return 0;
}