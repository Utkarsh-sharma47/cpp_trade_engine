#pragma once
#include "LFQueue.h"
#include <thread>
#include <fstream>
#include <string>

class Logger {
private:
    LFQueue<std::string> queue;
    std::thread background_thread;
    std::atomic<bool> running;
    std::ofstream file;

    //
    void processEntries() {
        std::string msg;

        while (running.load()) {
            while (queue.pop(msg)) {
                file << msg << "\n"; //write to file
            }
        }
        
        //drain left over
        while (queue.pop(msg)) {
            file << msg << "\n";
        }
    }

public:
    
    Logger(const std::string& filename) : queue(1024), running(true) {
        file.open(filename);
        background_thread = std::thread(&Logger::processEntries, this);
    }
    
    ~Logger() {
        running.store(false);
        if (background_thread.joinable()) {
            background_thread.join(); // 
        }
        file.close();
    }
    
    void log(const std::string& msg) {
        
        while(!queue.push(msg)); 
    }
};