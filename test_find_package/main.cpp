#include "cebu/absurdity.h"
#include <iostream>

int main() {
    // Create a fuzzy interval
    cebu::FuzzyInterval interval(0.3, 0.5, 0.9);
    std::cout << "Fuzzy interval: " << interval.to_string() << std::endl;
    
    // Test basic operations
    cebu::FuzzyInterval interval2(0.4, 0.6, 0.8);
    auto sum = interval + interval2;
    std::cout << "Sum: " << sum.to_string() << std::endl;
    
    std::cout << "Test completed successfully!" << std::endl;
    return 0;
}
