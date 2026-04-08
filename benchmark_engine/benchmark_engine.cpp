#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <random>
#include <algorithm>
#include <memory>
#include <string>

#include "../include/LFQueue.h"
#include "../include/Logger.h"
#include "../include/MatchingEngine.h"

// Generate a realistic order flow simulation
void generate_order_flow(std::vector<Order>& orders, int num_orders) {
    std::mt19937 gen(1337);
    std::normal_distribution<double> price_dist(50000.0, 500.0);
    std::uniform_int_distribution<uint32_t> qty_dist(10, 100);
    std::uniform_int_distribution<int> side_dist(0, 1);

    for (int i = 0; i < num_orders; i++) {
        uint32_t px = std::max(1, static_cast<int>(price_dist(gen)));
        orders.emplace_back(i + 1, px, qty_dist(gen), side_dist(gen) == 0 ? 'B' : 'S');
    }
}

int main() {
    constexpr int NUM_ORDERS = 1000000; // 1 Million Orders
    std::vector<Order> pre_generated_orders;
    pre_generated_orders.reserve(NUM_ORDERS);
    generate_order_flow(pre_generated_orders, NUM_ORDERS);

    Logger sysLogger;
    LFQueue<Order> orderQueue(1024 * 1024); // Huge queue to hold all benchmark orders

    auto engine = std::make_unique<MatchingEngine>(orderQueue, sysLogger);

    // Warmup the cache
    std::cout << "Pre-loading queues...\n";
    for (const auto& o : pre_generated_orders) {
        orderQueue.push(o);
    }

    std::cout << "Starting Matching Engine Benchmark...\n";
    auto start = std::chrono::high_resolution_clock::now();

    Order incoming;
    while (orderQueue.pop(incoming)) {
        // FIX: The engine actually processes the math now!
        engine->processOrder(incoming);
    }

    auto end = std::chrono::high_resolution_clock::now();
    
    // Clean up
    engine->stop();

    auto duration_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

    double total_seconds = static_cast<double>(duration_ns) / 1e9;
    double ops_per_sec = static_cast<double>(NUM_ORDERS) / total_seconds;
    double latency_per_op_ns = static_cast<double>(duration_ns) / static_cast<double>(NUM_ORDERS);

    std::cout << "=========================================\n";
    std::cout << "       SUB-MICROSECOND BENCHMARK         \n";
    std::cout << "=========================================\n";
    std::cout << "Orders Processed : " << NUM_ORDERS << "\n";
    std::cout << "Total Time       : " << total_seconds << " seconds\n";
    std::cout << "Throughput       : " << ops_per_sec << " ops/second\n";
    std::cout << "Avg Latency      : " << latency_per_op_ns << " nanoseconds/order\n";
    std::cout << "=========================================\n";

    return 0;
}