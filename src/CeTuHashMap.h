#ifndef CETU_HASHMAP_H
#define CETU_HASHMAP_H

#include <optional>
#include <concepts>
#include <iostream>

template <typename K>
concept Hashable = requires {
    std::hash<K>{};
};

template <typename K>
concept EqualityComparable = requires(K a, K b) {
    { a == b } -> std::convertible_to<bool>;
};

template <typename K, typename V>
concept CopyAssignableAndConstructible =
    std::is_copy_constructible_v<K> && std::is_copy_assignable_v<K> &&
    std::is_copy_constructible_v<V> && std::is_copy_assignable_v<V>;

template<typename K, typename V>
concept HashMapRequirements = Hashable<K> && EqualityComparable<K> && CopyAssignableAndConstructible<K, V>;

// Attention: CeTuHashMap is not thread-safe.
template<typename K, typename V>
requires HashMapRequirements<K, V>
class CeTuHashMap final {
public:
    CeTuHashMap();
    ~CeTuHashMap() noexcept;

    CeTuHashMap(const CeTuHashMap& other);
    CeTuHashMap& operator=(const CeTuHashMap& other);

    CeTuHashMap(CeTuHashMap&& other) noexcept;
    CeTuHashMap& operator=(CeTuHashMap&& other) noexcept;

    void insert(K key, V value);
    std::optional<V> lookup(K key);
    void erase(K key);
    size_t size() const { return currentSize; }

private:
    // Node structure for the linked list
    struct Node {
        K key;
        V value;
        Node* next;

        Node(const K& k, const V& v) : key(k), value(v), next(nullptr) {}

        // This method is enabled only if KeyType and ValueType have move constructors.
        template<typename KeyType, typename ValueType, typename = std::enable_if_t<std::is_move_constructible_v<K> && std::is_move_constructible_v<V>>>
        Node(KeyType&& k, ValueType&& v) : key(std::forward<KeyType>(k)), value(std::forward<ValueType>(v)), next(nullptr) {}
    };

    // RAII wrapper for a single node
    class NodeHolder {
    public:
        template<typename... Args>
        explicit NodeHolder(Args&&... args);
        ~NodeHolder() { delete node; }

        Node* release();
        Node* get() { return node; }

    private:
        Node* node;
    };

    // RAII wrapper for the bucket array
    class BucketsHolder {
    public:
        BucketsHolder() : capacity(0), buckets(nullptr) {}
        explicit BucketsHolder(size_t _capacity) : capacity(_capacity), buckets(new Node*[capacity]()) {}
        ~BucketsHolder() { clear(); }
        
        // Disable copying
        BucketsHolder(const BucketsHolder&) = delete;
        BucketsHolder& operator=(const BucketsHolder&) = delete;
        
        // Enable moving
        BucketsHolder(BucketsHolder&& other) noexcept;
        BucketsHolder& operator=(BucketsHolder&& other) noexcept;

        Node** get() { return buckets; }
        Node*& operator[](size_t index) { return buckets[index]; }
        const Node* operator[](size_t index) const { return buckets[index]; }

        void rehash(size_t newCapacity);

    private:
        size_t capacity;
        Node** buckets;

        void clear();
    };

    BucketsHolder buckets;
    size_t currentSize;
    size_t capacity;

    static const size_t defaultSize = 16;
    static constexpr double loadFactor = 0.75;

    size_t hash(const K& key) const { return std::hash<K>{}(key) % capacity; }
    void rehash();
    void copy(const CeTuHashMap& other);
};

// Constructor
template<typename K, typename V>
requires HashMapRequirements<K, V>
CeTuHashMap<K, V>::CeTuHashMap() : buckets(defaultSize), currentSize(0), capacity(defaultSize) {}

// Destructor
template<typename K, typename V>
requires HashMapRequirements<K, V>
CeTuHashMap<K, V>::~CeTuHashMap() noexcept {
}

// Copy constructor
template<typename K, typename V>
requires HashMapRequirements<K, V>
CeTuHashMap<K, V>::CeTuHashMap(const CeTuHashMap& other) : currentSize(other.currentSize), capacity(other.capacity) {
    copy(other);
}

// Copy assignment operator
template<typename K, typename V>
requires HashMapRequirements<K, V>
CeTuHashMap<K, V>& CeTuHashMap<K, V>::operator=(const CeTuHashMap& other) {
    if(this == &other) {
        return *this;
    }

    // Copy from other
    capacity = other.capacity;
    currentSize = other.currentSize;
    copy(other);

    return *this;
}

// Move constructor
template<typename K, typename V>
requires HashMapRequirements<K, V>
CeTuHashMap<K, V>::CeTuHashMap(CeTuHashMap&& other) noexcept : buckets(std::move(other.buckets)),
    currentSize(other.currentSize), capacity(other.capacity) {
    other.currentSize = 0;
    other.capacity = 0;
}

// Move assignment operator
template<typename K, typename V>
requires HashMapRequirements<K, V>
CeTuHashMap<K, V>& CeTuHashMap<K, V>::operator=(CeTuHashMap&& other) noexcept {
    if(this == &other) {
        return *this;
    }

    std::swap(buckets, other.buckets);
    std::swap(currentSize, other.currentSize);
    std::swap(capacity, other.capacity);

    return *this;
}

template<typename K, typename V>
requires HashMapRequirements<K, V>
void CeTuHashMap<K, V>::insert(K key, V value) {
    if(currentSize > capacity * loadFactor) {
        rehash();
    }

    size_t index = hash(key);
    
    // Check if key already exists
    Node* current = buckets[index];
    while(current != nullptr) {
        if(current->key == key) {
            current->value = value;  // Update existing value
            return;
        }
        current = current->next;
    }

    // Create new node and insert at the beginning of the list
    NodeHolder newNode(std::move(key), std::move(value));
    newNode.get()->next = buckets[index];
    buckets[index] = newNode.release();
    currentSize++;
}

template<typename K, typename V>
requires HashMapRequirements<K, V>
std::optional<V>  CeTuHashMap<K, V>::lookup(K key) {
    if(currentSize == 0) {
        return std::nullopt;
    }

    size_t index = hash(key);
    Node* current = buckets[index];
    
    while(current != nullptr) {
        if(current->key == key) {
            return std::make_optional(current->value);
        }
        current = current->next;
    }
    
    return std::nullopt;
}

template<typename K, typename V>
requires HashMapRequirements<K, V>
void CeTuHashMap<K, V>::erase(K key) {
    if(currentSize == 0) {
        return;
    }

    size_t index = hash(key);
    Node* current = buckets[index];
    Node* prev = nullptr;

    while(current != nullptr) {
        if(current->key == key) {
            if(prev == nullptr) {
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

template<typename K, typename V>
requires HashMapRequirements<K, V>
void CeTuHashMap<K, V>::rehash() {
    size_t newCapacity = capacity * 2;
    buckets.rehash(newCapacity);
    capacity = newCapacity;
}

template<typename K, typename V>
requires HashMapRequirements<K, V>
void CeTuHashMap<K, V>::copy(const CeTuHashMap& other) {
    BucketsHolder tempBuckets(capacity);
    for(size_t i = 0; i < capacity; i++) {
        if(other.buckets[i] != nullptr) {
            // Copy the linked list at this bucket
            Node* otherCurrent = const_cast<Node*>(other.buckets[i]);
            Node* prev = nullptr;
            while(otherCurrent != nullptr) {
                NodeHolder newNode(otherCurrent->key, otherCurrent->value);
                Node* current = newNode.release();
                if(prev == nullptr) {
                    tempBuckets[i] = current;
                } else {
                    prev->next = current;
                }
                prev = current;
                otherCurrent = otherCurrent->next;
            }
        }
    }
    buckets = std::move(tempBuckets);
}

template<typename K, typename V>
requires HashMapRequirements<K, V>
template<typename... Args>
CeTuHashMap<K, V>::NodeHolder::NodeHolder(Args&&... args) :
    node(new Node(std::forward<Args>(args)...)) {}

template<typename K, typename V>
requires HashMapRequirements<K, V>
CeTuHashMap<K, V>::Node* CeTuHashMap<K, V>::NodeHolder::release() {
    Node* tmp = node;
    node = nullptr;
    return tmp;
}

template<typename K, typename V>
requires HashMapRequirements<K, V>
CeTuHashMap<K, V>::BucketsHolder::BucketsHolder(BucketsHolder&& other) noexcept :
    capacity(other.capacity), buckets(other.buckets)
{
    other.capacity = 0;
    other.buckets = nullptr;
}

template<typename K, typename V>
requires HashMapRequirements<K, V>
CeTuHashMap<K, V>::BucketsHolder& CeTuHashMap<K, V>::BucketsHolder::operator=(BucketsHolder&& other) noexcept {
    if(this == &other) {
        return *this;
    }

    std::swap(capacity, other.capacity);
    std::swap(buckets, other.buckets);

    return *this;
}

template<typename K, typename V>
requires HashMapRequirements<K, V>
void CeTuHashMap<K, V>::BucketsHolder::rehash(size_t newCapacity) {
    // Create new array of buckets
    Node** newBuckets = new Node*[newCapacity]();

    // Move all nodes to new buckets
    for(size_t i = 0; i < capacity; ++i) {
        Node* current = buckets[i];
        while(current) {
            Node* next = current->next;
            // Calculate new index based on new capacity
            size_t newIndex = std::hash<K>{}(current->key) % newCapacity;
            // Insert at beginning of new bucket
            current->next = newBuckets[newIndex];
            newBuckets[newIndex] = current;
            current = next;
        }
    }

    delete[] buckets;

    // Update capacity and buckets pointer
    capacity = newCapacity;
    buckets = newBuckets;
}

template<typename K, typename V>
requires HashMapRequirements<K, V>
void CeTuHashMap<K, V>::BucketsHolder::clear() {
    if(buckets) {
        for (size_t i = 0; i < capacity; ++i) {
            Node* current = buckets[i];
            while(current) {
                Node* next = current->next;
                delete current;
                current = next;
            }
        }
        delete[] buckets;
        buckets = nullptr;
    }
}

#endif // CETU_HASHMAP_H