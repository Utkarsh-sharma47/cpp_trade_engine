#include <iostream>
#include "Logger.h"

int main() {
    std::cout << "Starting Trading Engine..." << std::endl;

    // 1. Create the logger
    Logger engineLogger("trade_events.log");

    // 2.Hot PAth
    std::cout << "Executing fast trades..." << std::endl;
    
    for (int i = 0; i < 50; i++) {
        
        engineLogger.log("Executed Trade ID: " + std::to_string(i) + " for AAPL.");
    }

    std::cout << "Trades complete. Shutting down system." << std::endl;
   
    return 0;
}