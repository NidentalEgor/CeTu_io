#include "../src/CeTuHashMap.h"

#include <iostream>
#include <sys/resource.h>
#include <gtest/gtest.h>

TEST(CeTuHashMap, Print_IntIntMapTest) {
    // Test with int as both key and value
    CeTuHashMap<int, int> intMap;
    intMap.insert(1, 2);
    auto data = intMap.lookup(1);
    ASSERT_TRUE(data.has_value());
    if (data) {
        std::cout << "data: " << *data << std::endl;
    } else {
        std::cout << "Key not found." << std::endl;
    }

    // Attempt to lookup a key that doesn't exist
    auto missingData = intMap.lookup(3);
    ASSERT_FALSE(missingData.has_value());
    if (missingData) {
        std::cout << "Missing data: " << *missingData << std::endl;
    } else {
        std::cout << "Key 3 not found." << std::endl;
    }

    // Erase a key and then attempt to look it up
    intMap.erase(1);
    auto erasedData = intMap.lookup(1);
    ASSERT_FALSE(erasedData.has_value());
    if (erasedData) {
        std::cout << "Erased data: " << *erasedData << std::endl;
    } else {
        std::cout << "Key 1 not found after erase." << std::endl;
    }
}

TEST(CeTuHashMap, Print_StringDoubleMapTest) {
    // Test with std::string as key and double as value
    static constexpr double Pi = 3.14159;
    static constexpr double E = 2.71828;

    CeTuHashMap<std::string, double> stringMap;
    stringMap.insert("pi", Pi);
    auto piValue = stringMap.lookup("pi");
    ASSERT_TRUE(piValue.has_value());
    ASSERT_DOUBLE_EQ(piValue.value(), Pi);
    if (piValue) {
        std::cout << "pi: " << *piValue << std::endl;
    } else {
        std::cout << "Key 'pi' not found." << std::endl;
    }

    // Insert additional values and demonstrate lookup
    stringMap.insert("e", E);
    auto eValue = stringMap.lookup("e");
    ASSERT_TRUE(eValue.has_value());
    ASSERT_DOUBLE_EQ(eValue.value(), E);
    if (eValue) {
        std::cout << "e: " << *eValue << std::endl;
    } else {
        std::cout << "Key 'e' not found." << std::endl;
    }

    // Erase a key and then attempt to look it up
    stringMap.erase("pi");
    auto erasedPiValue = stringMap.lookup("pi");
    ASSERT_FALSE(erasedPiValue.has_value());
    if (erasedPiValue) {
        std::cout << "Erased pi value: " << *erasedPiValue << std::endl;
    } else {
        std::cout << "Key 'pi' not found after erase." << std::endl;
    }
}

TEST(CeTuHashMap, CopyConstructorTest) {
    CeTuHashMap<int, int> original;
    original.insert(1, 100);
    original.insert(2, 200);
    
    CeTuHashMap<int, int> copy = original;
    ASSERT_EQ(copy.size(), original.size());
    ASSERT_EQ(original.lookup(1).value(), 100);
    ASSERT_EQ(original.lookup(2).value(), 200);
    ASSERT_EQ(copy.lookup(1).value(), 100);
    ASSERT_EQ(copy.lookup(2).value(), 200);
}

TEST(CeTuHashMap, MoveConstructorTest) {
    CeTuHashMap<int, int> original;
    original.insert(1, 100);
    original.insert(2, 200);

    CeTuHashMap<int, int> moved = std::move(original);
    ASSERT_EQ(original.size(), 0);
    ASSERT_EQ(moved.size(), 2);
    ASSERT_EQ(moved.lookup(1).value(), 100);
    ASSERT_EQ(moved.lookup(2).value(), 200);
}

TEST(CeTuHashMap, CapacityAndSizeTest) {
    CeTuHashMap<int, int> map;
    ASSERT_EQ(map.size(), 0);
    
    // Test size increases with insertions
    map.insert(1, 100);
    ASSERT_EQ(map.size(), 1);
    map.insert(2, 200);
    ASSERT_EQ(map.size(), 2);
    
    // Test size decreases with erasure
    map.erase(1);
    ASSERT_EQ(map.size(), 1);
    
    // Test size doesn't change when inserting duplicate keys
    map.insert(2, 300); // Update existing value
    ASSERT_EQ(map.size(), 1);
}

TEST(CeTuHashMap, UpdateExistingKeyTest) {
    CeTuHashMap<std::string, int> map;
    
    // Insert initial value
    map.insert("test", 100);
    auto value = map.lookup("test");
    ASSERT_TRUE(value.has_value());
    ASSERT_EQ(*value, 100);
    
    // Update value for existing key
    map.insert("test", 200);
    value = map.lookup("test");
    ASSERT_TRUE(value.has_value());
    ASSERT_EQ(*value, 200);
}

TEST(CeTuHashMap, RehashTest) {
    static constexpr int elementsCount = 1000;

    CeTuHashMap<int, int> map;
    for(size_t i = 0; i < elementsCount; ++i) {
        map.insert(i, i);
        ASSERT_EQ(map.size(), i + 1);
    }

    for(size_t i = 0; i < elementsCount; ++i) {
        auto value = map.lookup(i);
        ASSERT_TRUE(value.has_value());
        ASSERT_EQ(*value, i);
    }
}

TEST(CeTuHashMap, StressTest) {
    static constexpr int elementsCount = 1000;
    
    CeTuHashMap<int, int> map;
    // Insert many elements
    for (int i = 0; i < elementsCount; ++i) {
        map.insert(i, i * 2);
    }
    
    // Verify all elements
    for (int i = 0; i < elementsCount; ++i) {
        auto value = map.lookup(i);
        ASSERT_TRUE(value.has_value());
        ASSERT_EQ(*value, i * 2);
    }
    
    // Erase every other element
    for (int i = 0; i < elementsCount; i += 2) {
        map.erase(i);
    }
    
    // Verify remaining elements
    for (int i = 0; i < elementsCount; ++i) {
        auto value = map.lookup(i);
        if (i % 2 == 0) {
            ASSERT_FALSE(value.has_value());
        } else {
            ASSERT_TRUE(value.has_value());
            ASSERT_EQ(*value, i * 2);
        }
    }
}

TEST(CeTuHashMap, EdgeCasesTest) {
    CeTuHashMap<std::string, int> map;
    
    // Test empty string key
    map.insert("", 42);
    ASSERT_TRUE(map.lookup("").has_value());
    ASSERT_EQ(map.lookup("").value(), 42);
    
    // Test erasing non-existent key
    map.erase("nonexistent");
    ASSERT_FALSE(map.lookup("nonexistent").has_value());
    
    // Test multiple erasures of same key
    map.insert("test", 100);
    map.erase("test");
    map.erase("test"); // Should not cause any issues
    ASSERT_FALSE(map.lookup("test").has_value());
}

class MyTestClass{
public:
    MyTestClass() : myId(id++) { std::cout << "MyTestClass()" << std::endl; }
    ~MyTestClass() { std::cout << "~MyTestClass()" << std::endl; }

    MyTestClass(const MyTestClass&) { std::cout << "MyTestClass(const MyTestClass&)" << std::endl; }
    MyTestClass& operator=(const MyTestClass&) { std::cout << "MyTestClass& operator=(const MyTestClass&)" << std::endl; return *this; }

    MyTestClass(MyTestClass&&) { std::cout << "MyTestClass(MyTestClass&&)" << std::endl; }
    MyTestClass& operator=(MyTestClass&&) { std::cout << "MyTestClass& operator=(MyTestClass&&)" << std::endl; return *this; }

    bool operator==(const MyTestClass& other) const { std::cout << "bool operator==(const MyTestClass& other) const" << std::endl; return myId == other.myId; }

private:
    static unsigned long long id;

    unsigned long long myId;
};

unsigned long long MyTestClass::id = 0;

namespace std {
    template <>
    struct hash<MyTestClass> {
        std::size_t operator()(const MyTestClass& t) const {
            return std::hash<int>()(1);
        }
    };
}

TEST(CeTuHashMap, TestConceptsAndMoveSemantic) {
    CeTuHashMap<int, MyTestClass> testMap1;
    MyTestClass value;
    testMap1.insert(0, std::move(value));

    CeTuHashMap<MyTestClass, int> testMap2;
    MyTestClass key;
    testMap2.insert(key, 1);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);

    ::testing::GTEST_FLAG(break_on_failure) = true;
    try{
        return RUN_ALL_TESTS();
    } catch(const std::exception& ex){
        std::cout << ex.what() << std::endl;
        return 1;
    }
}