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

template <typename K, typename V>
concept CopyAssignableAndConstructible =
    std::is_copy_constructible_v<K> && std::is_copy_assignable_v<K> &&
    std::is_copy_constructible_v<V> && std::is_copy_assignable_v<V>;

// Attention: CeTuHashMap is not thread-safe.
template<typename K, typename V>
requires Hashable<K> && EqualityComparable<K> && CopyAssignableAndConstructible<K, V>
class CeTuHashMap final {
public:
    CeTuHashMap();
    ~CeTuHashMap() noexcept;

    CeTuHashMap(const CeTuHashMap& other);
    CeTuHashMap& operator=(const CeTuHashMap& other);

    CeTuHashMap(CeTuHashMap&& other) noexcept;
    CeTuHashMap& operator=(CeTuHashMap&& other) noexcept;

    void insert(K key, V value);
    std::optional<V> lookup(K key) const;
    void erase(K key);
    size_t size() const { return currentSize; }

private:
    // Node structure for the linked list
    struct Node {
        K key;
        V value;
        Node* next;

        Node(const K& k, const V& v) : key(k), value(v), next(nullptr) {}

        template<typename KeyType, typename ValueType, typename = std::enable_if_t<std::is_move_constructible_v<K> && std::is_move_constructible_v<V>>>
        Node(KeyType&& k, ValueType&& v) : key(std::forward<KeyType>(k)),
            value(std::forward<ValueType>(v)), next(nullptr) {}
    };

    Node** buckets;
    size_t currentSize;
    size_t capacity;

    static const size_t defaultSize = 16;
    static constexpr double loadFactor = 0.75;

    size_t hash(const K& key) const { return std::hash<K>{}(key) % capacity; }
    void rehash();
    void copy(const CeTuHashMap& other);
    void remove();
};

// Constructor
template<typename K, typename V>
requires Hashable<K> && EqualityComparable<K> && CopyAssignableAndConstructible<K, V>
CeTuHashMap<K, V>::CeTuHashMap() : currentSize(0), capacity(defaultSize) {
    buckets = new Node*[capacity]();  // Initialize all buckets to nullptr
}

// Destructor
template<typename K, typename V>
requires Hashable<K> && EqualityComparable<K> && CopyAssignableAndConstructible<K, V>
CeTuHashMap<K, V>::~CeTuHashMap() noexcept {
    remove();
}

// Copy constructor
template<typename K, typename V>
requires Hashable<K> && EqualityComparable<K> && CopyAssignableAndConstructible<K, V>
CeTuHashMap<K, V>::CeTuHashMap(const CeTuHashMap& other) : currentSize(other.currentSize), capacity(other.capacity) {
    copy(other);
}

// Copy assignment operator
template<typename K, typename V>
requires Hashable<K> && EqualityComparable<K> && CopyAssignableAndConstructible<K, V>
CeTuHashMap<K, V>& CeTuHashMap<K, V>::operator=(const CeTuHashMap& other) {
    if (this == &other) {
        return *this;
    }

    // Clear existing contents
    remove();

    // Copy from other
    capacity = other.capacity;
    currentSize = other.currentSize;
    copy(other);

    return *this;
}

// Move constructor
template<typename K, typename V>
requires Hashable<K> && EqualityComparable<K> && CopyAssignableAndConstructible<K, V>
CeTuHashMap<K, V>::CeTuHashMap(CeTuHashMap&& other) noexcept : buckets(other.buckets),
    currentSize(other.currentSize), capacity(other.capacity) {
    other.buckets = nullptr;
    other.currentSize = 0;
    other.capacity = 0;
}

// Move assignment operator
template<typename K, typename V>
requires Hashable<K> && EqualityComparable<K> && CopyAssignableAndConstructible<K, V>
CeTuHashMap<K, V>& CeTuHashMap<K, V>::operator=(CeTuHashMap&& other) noexcept {
    if (this == &other) {
        return *this;
    }

    std::swap(buckets, other.buckets);
    std::swap(currentSize, other.currentSize);
    std::swap(capacity, other.capacity);

    return *this;
}

template<typename K, typename V>
requires Hashable<K> && EqualityComparable<K> && CopyAssignableAndConstructible<K, V>
void CeTuHashMap<K, V>::insert(K key, V value) {
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
    Node* newNode = new Node(std::move(key), std::move(value));
    newNode->next = buckets[index];
    buckets[index] = newNode;
    currentSize++;

    if (currentSize > capacity * loadFactor) {
        rehash();
    }
}

template<typename K, typename V>
requires Hashable<K> && EqualityComparable<K> && CopyAssignableAndConstructible<K, V>
std::optional<V>  CeTuHashMap<K, V>::lookup(K key) const {
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

template<typename K, typename V>
requires Hashable<K> && EqualityComparable<K> && CopyAssignableAndConstructible<K, V>
void CeTuHashMap<K, V>::erase(K key) {
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

template<typename K, typename V>
requires Hashable<K> && EqualityComparable<K> && CopyAssignableAndConstructible<K, V>
void CeTuHashMap<K, V>::rehash() {
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
            insert(std::move(current->key), std::move(current->value));
            delete current;
            current = next;
        }
    }

    delete[] oldBuckets;
}

template<typename K, typename V>
requires Hashable<K> && EqualityComparable<K> && CopyAssignableAndConstructible<K, V>
void CeTuHashMap<K, V>::copy(const CeTuHashMap& other) {
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

template<typename K, typename V>
requires Hashable<K> && EqualityComparable<K> && CopyAssignableAndConstructible<K, V>
void CeTuHashMap<K, V>::remove() {
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

#endif // CETU_HASHMAP_H