#include "CeTuHashMap.h"

#include <iostream>

int main() {
    try{
        // Test with int as both key and value
        CeTuHashMap<int, int> intMap;
        intMap.insert(1, 2);
        auto data = intMap.lookup(1);
        if (data) {
            std::cout << "data: " << *data << std::endl;
        } else {
            std::cout << "Key not found." << std::endl;
        }
    
        // Attempt to lookup a key that doesn't exist
        auto missingData = intMap.lookup(3);
        if (missingData) {
            std::cout << "Missing data: " << *missingData << std::endl;
        } else {
            std::cout << "Key 3 not found." << std::endl;
        }
    
        // Erase a key and then attempt to look it up
        intMap.erase(1);
        auto erasedData = intMap.lookup(1);
        if (erasedData) {
            std::cout << "Erased data: " << *erasedData << std::endl;
        } else {
            std::cout << "Key 1 not found after erase." << std::endl;
        }
    
        // Test with std::string as key and double as value
        CeTuHashMap<std::string, double> stringMap;
        stringMap.insert("pi", 3.14159);
        auto piValue = stringMap.lookup("pi");
        if (piValue) {
            std::cout << "pi: " << *piValue << std::endl;
        } else {
            std::cout << "Key 'pi' not found." << std::endl;
        }
    
        // Insert additional values and demonstrate lookup
        stringMap.insert("e", 2.71828);
        auto eValue = stringMap.lookup("e");
        if (eValue) {
            std::cout << "e: " << *eValue << std::endl;
        } else {
            std::cout << "Key 'e' not found." << std::endl;
        }
    
        // Erase a key and then attempt to look it up
        stringMap.erase("pi");
        auto erasedPiValue = stringMap.lookup("pi");
        if (erasedPiValue) {
            std::cout << "Erased pi value: " << *erasedPiValue << std::endl;
        } else {
            std::cout << "Key 'pi' not found after erase." << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}