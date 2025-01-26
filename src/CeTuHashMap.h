#ifndef CETU_HASHMAP_H
#define CETU_HASHMAP_H

#include <optional>
#include <concepts>

// защита от исключений
// многопоточность
// копирование с обменом
// CeTuHashMap& operator=(CeTuHashMap other) {
//     std::swap(buckets, other.buckets);
//     std::swap(currentSize, other.currentSize);
//     std::swap(capacity, other.capacity);
//     return *this;
// }

template <typename K>
concept Hashable = requires {
    std::hash<K>{};
};

template <typename K>
concept EqualityComparable = requires(K a, K b) {
    { a == b } -> std::convertible_to<bool>;
};


template<typename K, typename V>
requires Hashable<K> && EqualityComparable<K> &&
std::is_copy_constructible_v<K> && std::is_copy_assignable_v<K> &&
std::is_copy_constructible_v<V> && std::is_copy_assignable_v<V>
class CeTuHashMap final {
public:
    // Constructor
    CeTuHashMap() : currentSize(0), capacity(defaultSize) {
        buckets = new Node*[capacity]();  // Initialize all buckets to nullptr
    }

    // Destructor
    ~CeTuHashMap() {
        for (size_t i = 0; i < capacity; i++) {
            Node* current = buckets[i];
            while (current != nullptr) {
                Node* next = current->next;
                delete current;
                current = next;
            }
        }
        delete[] buckets;
    }

    // Copy constructor
    CeTuHashMap(const CeTuHashMap& other) : currentSize(other.currentSize), capacity(other.capacity) {
        buckets = new Node*[capacity]();
        for (size_t i = 0; i < capacity; i++) {
            if (other.buckets[i] != nullptr) {
                // Copy the linked list at this bucket
                Node* otherCurrent = other.buckets[i];
                Node* prev = nullptr;
                while (otherCurrent != nullptr) {
                    Node* newNode = new Node(otherCurrent->key, otherCurrent->value);
                    if (prev == nullptr) {
                        buckets[i] = newNode;
                    } else {
                        prev->next = newNode;
                    }
                    prev = newNode;
                    otherCurrent = otherCurrent->next;
                }
            }
        }
    }

    // Move constructor
    CeTuHashMap(CeTuHashMap&& other) noexcept : buckets(other.buckets),
        currentSize(other.currentSize), capacity(other.capacity) {
        other.buckets = nullptr;
        other.currentSize = 0;
        other.capacity = 0;
    }

    // Copy assignment operator
    CeTuHashMap& operator=(const CeTuHashMap& other) {
        if (this != &other) {
            // Clear existing contents
            for (size_t i = 0; i < capacity; i++) {
                Node* current = buckets[i];
                while (current != nullptr) {
                    Node* next = current->next;
                    delete current;
                    current = next;
                }
            }
            delete[] buckets;

            // Copy from other
            capacity = other.capacity;
            currentSize = other.currentSize;
            buckets = new Node*[capacity]();

            for (size_t i = 0; i < capacity; i++) {
                if (other.buckets[i] != nullptr) {
                    Node* otherCurrent = other.buckets[i];
                    Node* prev = nullptr;
                    while (otherCurrent != nullptr) {
                        Node* newNode = new Node(otherCurrent->key, otherCurrent->value);
                        if (prev == nullptr) {
                            buckets[i] = newNode;
                        } else {
                            prev->next = newNode;
                        }
                        prev = newNode;
                        otherCurrent = otherCurrent->next;
                    }
                }
            }
        }
        return *this;
    }

    // Move assignment operator
    CeTuHashMap& operator=(CeTuHashMap&& other) noexcept {
        if (this != &other) {
            // Delete current contents
            for (size_t i = 0; i < capacity; i++) {
                Node* current = buckets[i];
                while (current != nullptr) {
                    Node* next = current->next;
                    delete current;
                    current = next;
                }
            }
            delete[] buckets;

            // Move from other
            buckets = other.buckets;
            currentSize = other.currentSize;
            capacity = other.capacity;

            // Reset other
            other.buckets = nullptr;
            other.currentSize = 0;
            other.capacity = 0;
        }
        return *this;
    }

    void insert(K key, V value) {
        size_t index = hash(key);
        
        // Check if key already exists
        Node* current = buckets[index];
        while (current != nullptr) {
            if (current->key == key) {
                current->value = value;  // Update existing value
                return;
            }
            current = current->next;
        }

        // Create new node and insert at the beginning of the list
        Node* newNode = new Node(key, value);
        newNode->next = buckets[index];
        buckets[index] = newNode;
        currentSize++;

        if (currentSize > capacity * loadFactor) {
            rehash();
        }
    }

    std::optional<V> lookup(K key) {
        if(currentSize == 0) {
            return std::nullopt;
        }

        size_t index = hash(key);
        Node* current = buckets[index];
        
        while (current != nullptr) {
            if (current->key == key) {
                return std::optional<V>(current->value);
            }
            current = current->next;
        }
        
        return std::nullopt;
    }

    void erase(K key) {
        if(currentSize == 0) {
            return;
        }

        size_t index = hash(key);
        Node* current = buckets[index];
        Node* prev = nullptr;

        while (current != nullptr) {
            if (current->key == key) {
                if (prev == nullptr) {
                    buckets[index] = current->next;
                } else {
                    prev->next = current->next;
                }
                delete current;
                currentSize--;
                return;
            }
            prev = current;
            current = current->next;
        }
    }

    size_t size() const {
        return currentSize;
    }

private:
    // Node structure for the linked list
    struct Node {
        K key;
        V value;
        Node* next;
        Node(const K& k, const V& v) : key(k), value(v), next(nullptr) {}
    };

    Node** buckets;
    size_t currentSize;
    size_t capacity;

    static const size_t defaultSize = 16;
    static constexpr double loadFactor = 0.75;

    // Simple hash function
    size_t hash(const K& key) const {
        return std::hash<K>{}(key) % capacity;
    }


    void rehash() {
        size_t oldCapacity = capacity;
        capacity *= 2;
        Node** oldBuckets = buckets;

        buckets = new Node*[capacity];
        for (size_t i = 0; i < capacity; ++i) {
            buckets[i] = nullptr;
        }

        currentSize = 0;
        for (size_t i = 0; i < oldCapacity; ++i) {
            Node* current = oldBuckets[i];
            while (current) {
                Node* next = current->next;
                insert(current->key, current->value);
                delete current;
                current = next;
            }
        }

        delete[] oldBuckets;
    }
};

#endif // CETU_HASHMAP_H