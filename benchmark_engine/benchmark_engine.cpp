#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <random>
#include <algorithm>
#include <memory>
#include <string>
#include <fstream> // NEW: File Stream library for CSV writing

#include "../include/LFQueue.h"
#include "../include/Logger.h"
#include "../include/MatchingEngine.h"

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

int main(int argc, char* argv[]) {
    int NUM_ORDERS = 1000000;
    if (argc > 1) {
        NUM_ORDERS = std::stoi(argv[1]);
    }

    std::vector<Order> pre_generated_orders;
    pre_generated_orders.reserve(NUM_ORDERS);
    generate_order_flow(pre_generated_orders, NUM_ORDERS);

    Logger sysLogger;
    LFQueue<Order> orderQueue(NUM_ORDERS + 1024); 

    // NEW: Automatically make the pool size double the order size to guarantee no crashes
    size_t pool_size = NUM_ORDERS * 2; 
    auto engine = std::make_unique<MatchingEngine>(orderQueue, sysLogger, pool_size);

    std::vector<long long> latencies;
    latencies.reserve(NUM_ORDERS);

    std::cout << "Pre-loading " << NUM_ORDERS << " orders into queue...\n";
    for (const auto& o : pre_generated_orders) {
        orderQueue.push(o);
    }

    std::cout << "Starting Matching Engine Benchmark...\n";
    auto start = std::chrono::high_resolution_clock::now();

    Order incoming;
    while (orderQueue.pop(incoming)) {
        auto t1 = std::chrono::high_resolution_clock::now();
        
        engine->processOrder(incoming);
        
        auto t2 = std::chrono::high_resolution_clock::now();
        latencies.push_back(std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count());
    }

    auto end = std::chrono::high_resolution_clock::now();
    engine->stop();

    std::sort(latencies.begin(), latencies.end());
    long long p50 = latencies[latencies.size() * 0.50];
    long long p95 = latencies[latencies.size() * 0.95];
    long long p99 = latencies[latencies.size() * 0.99];

    auto duration_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    double total_seconds = static_cast<double>(duration_ns) / 1e9;
    double ops_per_sec = static_cast<double>(NUM_ORDERS) / total_seconds;
    double latency_per_op_ns = static_cast<double>(duration_ns) / static_cast<double>(NUM_ORDERS);

    std::cout << "=========================================\n";
    std::cout << "       SUB-MICROSECOND BENCHMARK         \n";
    std::cout << "=========================================\n";
    std::cout << "Orders Processed : " << NUM_ORDERS << "\n";
    std::cout << "Throughput       : " << ops_per_sec << " ops/second\n";
    std::cout << "Avg Latency      : " << latency_per_op_ns << " ns\n";
    std::cout << "Median (P50)     : " << p50 << " ns\n";
    std::cout << "P95 Latency      : " << p95 << " ns\n";
    std::cout << "P99 Latency      : " << p99 << " ns\n";
    std::cout << "=========================================\n";

    // NEW: Write the data to a CSV file for Python to read later
    std::ofstream csv_file("benchmark_results.csv", std::ios::app);
    
    // Write headers if the file is completely empty
    csv_file.seekp(0, std::ios::end); 
    if (csv_file.tellp() == 0) {
        csv_file << "Orders,Avg_ns,P50_ns,P95_ns,P99_ns\n";
    }
    
    // Append the current run's data
    csv_file << NUM_ORDERS << "," << latency_per_op_ns << "," << p50 << "," << p95 << "," << p99 << "\n";
    csv_file.close();

    std::cout << "Data appended to benchmark_results.csv\n";
    return 0;
}