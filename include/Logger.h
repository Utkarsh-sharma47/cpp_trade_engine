#pragma once
#include "LFQueue.h"
#include <thread>
#include <iostream>
#include <string>

class Logger {
private:
    LFQueue<std::string> queue;
    std::thread background_thread;
    std::atomic<bool> running;

    void processEntries() {
        std::string msg;
        while (running.load(std::memory_order_acquire)) {
            while (queue.pop(msg)) {
                std::cout << "[LOG] " << msg << "\n"; // Output to CMD
            }
        }
        // Drain leftover
        while (queue.pop(msg)) {
            std::cout << "[LOG] " << msg << "\n";
        }
    }

public:
    Logger() : queue(4096), running(true) {
        background_thread = std::thread(&Logger::processEntries, this);
    }
    
    ~Logger() {
        running.store(false, std::memory_order_release);
        if (background_thread.joinable()) {
            background_thread.join();
        }
    }
    
    void log(const std::string& msg) {
        while(!queue.push(msg)) {
            std::this_thread::yield(); // Backoff if queue is full
        }
    }
};