#ifndef CETU_HASHMAP_H
#define CETU_HASHMAP_H

#include <optional>

template<typename K, typename V>
class CeTuHashMap {
public:
    CeTuHashMap();
    ~CeTuHashMap();

    // Insert a new pair into the hashmap
    void insert(K key, V value);
 
    // Lookup the given key in the map, if the key is not found return nullptr
    std::optional<V> lookup(K key);
 
    // Delete a pair with the key in the hashmap
    void erase(K key);

private:
    struct Node {
        K key;
        V value;
        Node* next;

        Node(K key, V value) : key(key), value(value), next(nullptr) {}
    };

    Node** table;       // Array of pointers to linked lists
    size_t capacity;    // Number of buckets
    size_t size;        // Number of elements in the hash map

    static constexpr size_t DEFAULT_CAPACITY = 16;
    static constexpr double loadFactor = 0.75;

    size_t hash(K key) const;
    void rehash();
};

template<typename K, typename V>
CeTuHashMap<K, V>::CeTuHashMap() {
    table = new Node*[capacity];
    for (size_t i = 0; i < capacity; ++i) {
        table[i] = nullptr;
    }
}

template<typename K, typename V>
CeTuHashMap<K, V>::~CeTuHashMap() {
    for (size_t i = 0; i < capacity; ++i) {
        Node* current = table[i];
        while (current) {
            Node* next = current->next;
            delete current;
            current = next;
        }
    }
    delete[] table;
}

// Insert a new pair into the hashmap
template<typename K, typename V>
void CeTuHashMap<K, V>::insert(K key, V value) {
    size_t index = hash(key);

    Node* current = table[index];
    while (current) {
        if (current->key == key) {
            current->value = value;
            return;
        }
        current = current->next;
    }

    Node* newNode = new Node(key, value);
    newNode->next = table[index];
    table[index] = newNode;
    ++size;

    if (size > capacity * loadFactor) {
        rehash();
    }
}

// Lookup the given key in the map, if the key is not found return nullptr
template<typename K, typename V>
std::optional<V> CeTuHashMap<K, V>::lookup(K key) {
    size_t index = hash(key);
    Node* current = table[index];

    while (current) {
        if (current->key == key) {
            return current->value;
        }
        current = current->next;
    }
    return std::nullopt;
}

// Delete a pair with the key in the hashmap
template<typename K, typename V>
void CeTuHashMap<K, V>::erase(K key) {
    size_t index = hash(key);
    Node* current = table[index];
    Node* prev = nullptr;

    while (current) {
        if (current->key == key) {
            if (prev) {
                prev->next = current->next;
            } else {
                table[index] = current->next;
            }
            delete current;
            --size;
            return;
        }
        prev = current;
        current = current->next;
    }
}

template<typename K, typename V>
size_t CeTuHashMap<K, V>::hash(K key) const {
    return std::hash<K>()(key) % capacity;
}

template<typename K, typename V>
void CeTuHashMap<K, V>::rehash() {
    size_t oldCapacity = capacity;
    capacity *= 2;
    Node** oldTable = table;

    table = new Node*[capacity];
    for (size_t i = 0; i < capacity; ++i) {
        table[i] = nullptr;
    }

    for (size_t i = 0; i < oldCapacity; ++i) {
        Node* current = oldTable[i];
        while (current) {
            Node* next = current->next;
            insert(current->key, current->value);
            delete current;
            current = next;
        }
    }

    delete[] oldTable;
}

#endif // CETU_HASHMAP_H