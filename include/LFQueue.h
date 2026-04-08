#pragma once
#include <vector>
#include <atomic>

//template
template <typename T>
class LFQueue {
private:
    std::vector<T> buffer;
    std::atomic<size_t> head; 
    std::atomic<size_t> tail; 
    size_t capacity;

public:
    //Initialize size of Queue
    LFQueue(size_t size) : buffer(size), head(0), tail(0), capacity(size) {}

    // Push
    bool push(const T& item) {
        size_t current_head = head.load(std::memory_order_relaxed);
        size_t next_head = (current_head + 1) % capacity; \

        // queue full
        if (next_head == tail.load(std::memory_order_acquire)) {
            return false; 
        }

        buffer[current_head] = item; 
        head.store(next_head, std::memory_order_release);
        return true;
    }

    // Pop
    bool pop(T& item) {
        size_t current_tail = tail.load(std::memory_order_relaxed);
        
        //nothing to read
        if (current_tail == head.load(std::memory_order_acquire)) {
            return false; 
        }

        item = buffer[current_tail];
        tail.store((current_tail + 1) % capacity, std::memory_order_release); // Move tail forward safely
        return true;
    }
};