#include<iostream>
#include<vector>


template<typename T, typename Allocator = std::allocator<T>>
class List {
  private:
    class Node {
      public:
        T value;
        Node* next = this;
        Node* prev = this;

        Node() = default;
        Node(const T& newValue): value(newValue), next(this), prev(this) {}
        ~Node() {}
    };


    Node* root;
    size_t sz;

    Allocator alloc;
    using AllocTraits = std::allocator_traits<Allocator>;
    typename AllocTraits::template rebind_alloc<Node> nodeAlloc;
    using NodeAllocTraits = std::allocator_traits<typename AllocTraits::template rebind_alloc<Node>>;

    template<bool isConst>
    class iter {
      private:
        Node* ptr;
      public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = std::conditional_t<isConst, const T*, T*>;
        using reference = std::conditional_t<isConst, const T&, T&>;


        iter(Node* v): ptr(v) {}

        Node* getNodePtr() const {
            return ptr;
        }

        iter& operator++() {
            ptr = ptr->next;
            return *this;
        }

        iter operator++(int) {
            iter ans = *this;
            ++*this;
            return ans;
        }

        iter& operator--() {
            ptr = ptr->prev;
            return *this;
        }

        iter operator--(int) {
            iter ans = *this;
            --*this;
            return ans;
        }

        bool operator==(const iter& other) const {
            return ptr == other.ptr;
        }

        bool operator!=(const iter& other) const {
            return !(*this == other);
        }

        reference operator*() const {
            return ptr->value;
        }

        pointer operator->() const {
            return &ptr->value;
        }

        operator iter<true>() {
            return iter<true>(ptr);
        }
    };

  public:
    explicit List(const Allocator& alloc = Allocator()) : alloc(alloc) {
        sz = 0;
        typename std::allocator_traits<Allocator>::template rebind_alloc<Node> newNodeAlloc;
        nodeAlloc = newNodeAlloc;
        root = NodeAllocTraits::allocate(nodeAlloc, 1);
        root->next = root->prev = root;
    }
    List(size_t newSz, const T& newValue, const Allocator& alloc = Allocator()) : List(alloc) {
        sz = newSz;
        for (size_t i = 0; i < sz; ++i) {
            push_front(newValue);
        }
    }
    List(size_t newSz, const Allocator& alloc = Allocator()) : List(alloc) {
        sz = newSz;
        Node* cur = root;
        for (size_t i = 0; i < sz; ++i) {
            Node* v = NodeAllocTraits::allocate(nodeAlloc, 1);
            NodeAllocTraits::construct(nodeAlloc, v);

            v->next = root;
            v->prev = cur;
            cur->next = root->prev = v;

            cur = v;
        }
    }

    List(const List& other):
        List(std::allocator_traits<Allocator>::select_on_container_copy_construction(other.get_allocator())){
        for (auto it = other.begin(); it != other.end(); ++it) {
            push_back(*it);
        }
    }

    List(List&& other): root(other.root), sz(other.sz) {
        other.root = nullptr;
        other.sz = 0;

        alloc = std::move(other.alloc);
        nodeAlloc = std::move(other.nodeAlloc);
    }

    ~List() {
        Node* cur = root->next;
        for (size_t i = 0; i < sz; ++i) {
            Node* nxt = cur->next;
            NodeAllocTraits::destroy(nodeAlloc, cur);
            NodeAllocTraits::deallocate(nodeAlloc, cur, 1);
            cur = nxt;
        }

        NodeAllocTraits::deallocate(nodeAlloc, root, 1);
        root = nullptr;
        sz = 0;
    }

    List& operator=(const List& other) {
        if (this == &other) {
            return *this;
        }
        if (AllocTraits::propagate_on_container_copy_assignment::value) {
            nodeAlloc = other.nodeAlloc;
            alloc = other.alloc;
        }
        List newList = other;
        std::swap(newList.sz, sz);
        std::swap(newList.root, root);
        return *this;
    }

    List& operator=(List&& other) {
        if (this == &other) {
            return *this;
        }
        if (AllocTraits::propagate_on_container_copy_assignment::value) {
            nodeAlloc = std::move(other.nodeAlloc);
            alloc = std::move(other.alloc);
        }
        root = other.root;
        other.root = NodeAllocTraits::allocate(nodeAlloc, 1);;

        sz = other.sz;
        other.sz = 0;
        return *this;
    }

    Allocator get_allocator() const {
        return alloc;
    }

    size_t size() const {
        return sz;
    }

    void insert_between(const T& newValue, Node* prv, Node* nxt) {
        ++sz;
        Node* v = NodeAllocTraits::allocate(nodeAlloc, 1);
        NodeAllocTraits::construct(nodeAlloc, v, newValue);
        v->next = nxt;
        v->prev = prv;
        prv->next = nxt->prev = v;
    }

    void insert_between(Node* v, Node* prv, Node* nxt) {
        ++sz;
        v->next = nxt;
        v->prev = prv;
        prv->next = nxt->prev = v;
    }

    void erase_between(Node* prv, Node* nxt) {
        --sz;
        Node* v = prv->next;
        prv->next = nxt;
        nxt->prev = prv;
        NodeAllocTraits::destroy(nodeAlloc, v);
        NodeAllocTraits::deallocate(nodeAlloc, v, 1);
    }


    void push_back(const T& newValue) {
        insert_between(newValue, root->prev, root);
    }
    void push_front(const T& newValue) {
        insert_between(newValue, root, root->next);
    }
    void pop_back() {
        erase_between(root->prev->prev, root);
    }
    void pop_front() {
        erase_between(root, root->next->next);
    }
    template<typename... Args>
    void emplace_back(Args&&... args) {
        Node* cur = NodeAllocTraits::allocate(nodeAlloc, 1);
        AllocTraits::construct(alloc, &cur->value, std::forward<Args>(args)...);
        insert_between(cur, root->prev, root);
    }

    using iterator = iter<false>;
    using const_iterator = iter<true>;
    using reverse_iterator = std::reverse_iterator<iter<false>>;
    using const_reverse_iterator = std::reverse_iterator<iter<true>>;

    iterator begin() {
        return iterator(root->next);
    }
    const_iterator begin() const {
        return const_iterator(root->next);
    }
    iterator end() {
        return iterator(root);
    }
    const_iterator end() const {
        return const_iterator(root);
    }
    const_iterator cbegin() const {
        return const_iterator(root->next);
    }
    const_iterator cend() const {
        return const_iterator(root);
    }
    reverse_iterator rbegin() {
        auto ans = reverse_iterator(root);
        return ans;
    }
    const_reverse_iterator rbegin() const {
        return const_reverse_iterator(root);
    }
    reverse_iterator rend() {
        return reverse_iterator(root->next);
    }
    const_reverse_iterator rend() const {
        return const_reverse_iterator(root->next);
    }
    const_reverse_iterator crbegin() const {
        return const_reverse_iterator(root);
    }
    const_reverse_iterator crend() const {
        return const_reverse_iterator(root->next);
    }

    void insert(const_iterator id, const T& newValue) {
        Node* v = id.getNodePtr();
        insert_between(newValue, v->prev, v);
    }

    void erase(const_iterator id) {
        Node* v = id.getNodePtr();
        erase_between(v->prev, v->next);
    }
};




template<typename Key, typename Value, typename Hash = std::hash<Key>,
    typename Equal = std::equal_to<Key>, typename Alloc = std::allocator<std::pair<const Key, Value>>>
class UnorderedMap {
  private:
    using AllocTraits = std::allocator_traits<Alloc>;
    using ListIterator = typename List<std::pair<const Key, Value>, Alloc>::iterator;
    using VectorOfIteratorsAlloc = typename AllocTraits::template rebind_alloc<ListIterator>;
    using HashTableAlloc = typename AllocTraits::template rebind_alloc<std::vector<ListIterator, VectorOfIteratorsAlloc>>;

    std::vector<std::vector<ListIterator, VectorOfIteratorsAlloc>, HashTableAlloc> hashTable;
    List<std::pair<const Key, Value>, Alloc> elements;

    size_t hashTableSize = 0;
    float maxLoadFactor = 1.0;

    Hash hashFun;
    Equal equalFun;
    Alloc alloc;

    void rehash() {
        if (hashTableSize == 0) {
            hashTableSize = 1;
        }
        if (hashTableSize == hashTable.size()) {
            return;
        }
        hashTable.clear();
        hashTable.resize(hashTableSize);
        for (ListIterator iter = elements.begin(); iter != elements.end(); ++iter) {
            hashTable[hashFun(iter->first) % hashTableSize].push_back(iter);
        }
    }

  public:
    using NodeType = std::pair<const Key, Value>;

    UnorderedMap() {}

    UnorderedMap(const UnorderedMap& other): elements(other.elements),
                hashTableSize(other.hashTableSize), maxLoadFactor(other.maxLoadFactor),
                hashFun(other.hashFun), equalFun(other.equalFun), alloc(other.alloc) {
        rehash();
    }

    UnorderedMap(UnorderedMap&& other): hashTable(std::move(other.hashTable)),
                elements(std::move(other.elements)), hashTableSize(other.hashTableSize),
                maxLoadFactor(other.maxLoadFactor), hashFun(std::move(other.hashFun)),
                equalFun(std::move(other.equalFun)), alloc(std::move(other.alloc)) {
        other.hashTableSize = 0;
        other.maxLoadFactor = 1.0;
    }

    UnorderedMap& operator=(const UnorderedMap& other) {
        if (this == &other) {
            return *this;
        }

        UnorderedMap newMap = other;
        std::swap(newMap.hashTableSize, hashTableSize);
        std::swap(newMap.maxLoadFactor, maxLoadFactor);
        std::swap(newMap.hashTable, hashTable);
        std::swap(newMap.elements, elements);
        return *this;
    }

    UnorderedMap& operator=(UnorderedMap&& other) {
        if (this == &other) {
            return *this;
        }
        hashTableSize = other.hashTableSize;
        other.hashTableSize = 0;
        maxLoadFactor = other.maxLoadFactor;
        other.maxLoadFactor = 1.0;
        hashTable = std::move(other.hashTable);
        elements = std::move(other.elements);
        hashFun = std::move(other.hashFun);
        equalFun = std::move(other.equalFun);
        alloc = std::move(other.alloc);
        return *this;
    }

    ~UnorderedMap() {}

    size_t size() const {
        return elements.size();
    }

    using Iterator = ListIterator;
    using ConstIterator = typename List<NodeType, Alloc>::const_iterator;

    Iterator begin() {
        return elements.begin();
    }
    ConstIterator begin() const {
        return elements.begin();
    }
    Iterator end() {
        return elements.end();
    }
    ConstIterator end() const {
        return elements.end();
    }
    ConstIterator cbegin() const {
        return elements.cbegin();
    }
    ConstIterator cend() const {
        return elements.cend();
    }




    Value& operator[](const Key& key) {
        Iterator iter = find(key);
        if (iter != end()) {
            return iter->second;
        } else {
            return emplace(NodeType(key, Value())).first->second;
        }
    }

    Value& at(const Key& key) {
        Iterator iter = find(key);
        if (iter != end()) {
            return iter->second;
        } else {
            throw std::out_of_range("This key was not found");
        }
    }

    const Value& at(const Key& key) const {
        Iterator iter = find(key);
        if (iter != end()) {
            return iter->second;
        } else {
            throw std::out_of_range("This key was not found");
        }
    }

    Iterator find(const Key& key) {
        if (hashTableSize == 0) {
            hashTableSize = 1;
            hashTable.resize(1);
        }
        size_t hash = hashFun(key) % hashTableSize;
        for (size_t i = 0; i < hashTable[hash].size(); ++i) {
            Iterator iter = hashTable[hash][i];
            if (equalFun(key, iter->first)) {
                return iter;
            }
        }
        return end();
    }

    ConstIterator find(const Key& key) const {
        if (hashTableSize == 0) {
            hashTableSize = 1;
            hashTable.resize(1);
        }
        size_t hash = hashFun(key) % hashTableSize;
        for (auto iter : hashTable[hash]) {
            if (equalFun(key, iter->first)) {
                return const_cast<ConstIterator>(iter);
            }
        }
        return end();
    }

    float load_factor() const {
        if (hashTableSize == 0) {
            hashTableSize = 1;
            hashTable.resize(1);
        }
        return static_cast<float>(size()) / hashTableSize;
    }

    float max_load_factor() const {
        return maxLoadFactor;
    }

    void max_load_factor(float newMaxLoadFactor) {
        maxLoadFactor = newMaxLoadFactor;
        if (maxLoadFactor * hashTableSize < size()) {
            hashTableSize = static_cast<int>(size() / maxLoadFactor) + 2;
            rehash();
        }
    }

    void reserve(size_t n) {
        if (maxLoadFactor * n >= size()) {
            hashTableSize = n;
            rehash();
        }
    }

    template<typename... Args>
    std::pair<Iterator, bool> emplace(Args&&... args) {
        elements.emplace_back(std::forward<Args>(args)...);
        Iterator new_iter = elements.end();
        --new_iter;
        Iterator old_iter = find(new_iter->first);
        if (old_iter != elements.end()) {
            elements.pop_back();
            return {old_iter, false};
        } else {
            if (hashTableSize == 0) {
                hashTableSize = 1;
                hashTable.resize(1);
            }
            size_t hash = hashFun(new_iter->first) % hashTableSize;
            hashTable[hash].push_back(new_iter);
            if (maxLoadFactor * hashTableSize < size()) {
                hashTableSize = hashTableSize * 2 + 2;
                rehash();
            }
            return {new_iter, true};
        }
    }

    std::pair<Iterator, bool> insert(const NodeType& node) {
        return emplace(node);
    }

    template<typename U>
    std::pair<Iterator, bool> insert(U&& node) {
        return emplace(std::forward<U>(node));
    }

    template<typename InputIterator>
    void insert(InputIterator begin, InputIterator end) {
        while (begin != end) {
            emplace(*begin);
            ++begin;
        }
    }

    void erase(ConstIterator iter) {
        if (hashTableSize == 0) {
            return;
        }
        size_t hash = hashFun(iter->first) % hashTableSize;
        size_t block_size = hashTable[hash].size();
        for (size_t i = 0; i < block_size; ++i) {
            if (equalFun(hashTable[hash][i]->first, iter->first)) {
                std::swap(hashTable[hash][i], hashTable[hash][block_size - 1]);
                break;
            }
        }
        elements.erase(iter);
        hashTable[hash].pop_back();
    }

    void erase(ConstIterator begin, ConstIterator end) {
        while (begin != end) {
            erase(begin);
            ++begin;
        }
    }
};
