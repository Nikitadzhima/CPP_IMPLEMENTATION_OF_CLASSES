#include<iostream>
#include<vector>

template<size_t chunkSize>
class FixedAllocator {
  private:
    int8_t* ptr = nullptr;
    size_t sz = 0;
    const int MAX_SZ = 1024;
    std::vector<int8_t*> used;

    void getNewMemory() {
        sz = MAX_SZ;
        ptr = new int8_t[sz];
    }

    FixedAllocator() = default;

    static FixedAllocator* fixedAlloc;

  public:
    FixedAllocator(FixedAllocator& other) = delete;
    void operator=(const FixedAllocator& other) = delete;

    static FixedAllocator* getInstance() {
        if (fixedAlloc == nullptr) {
            fixedAlloc = new FixedAllocator();
        }
        return fixedAlloc;
    }

    void* allocate() {
        int8_t* newMemory;
        if (used.size()) {
            newMemory = used.back();
            used.pop_back();
        } else {
            if (sz < chunkSize) {
                getNewMemory();
            }
            sz -= chunkSize;
            newMemory = ptr;
            ptr += chunkSize;
        }
        return static_cast<void*>(newMemory);
    }

    void deallocate(void* ptr, size_t) {
        used.push_back(static_cast<int8_t*>(ptr));
    }
};

template<size_t chunkSize>
FixedAllocator<chunkSize>* FixedAllocator<chunkSize>::fixedAlloc = nullptr;

template<typename T>
class FastAllocator {
  public:
    FastAllocator() = default;


    template<typename U>
    FastAllocator(FastAllocator<U>) {}

    using value_type = T;

    ~FastAllocator() {}

    T* allocate(size_t sz) {
        if (sz * sizeof(T) == 8) {
            return static_cast<T*>(FixedAllocator<8>::getInstance()->allocate());
        } else if (sz * sizeof(T) == 16) {
            return static_cast<T*>(FixedAllocator<16>::getInstance()->allocate());
        } else if (sz * sizeof(T) == 24) {
            return static_cast<T*>(FixedAllocator<24>::getInstance()->allocate());
        } else if (sz * sizeof(T) == 32) {
            return static_cast<T*>(FixedAllocator<32>::getInstance()->allocate());
        } else {
            return static_cast<T*>(::operator new(sz * sizeof(T)));
        }
    }

    void deallocate(T* ptr, size_t sz) {
        if (sz * sizeof(T) == 8) {
            FixedAllocator<8>::getInstance()->deallocate(static_cast<void*>(ptr), sz);
        } else if (sz * sizeof(T) == 16) {
            FixedAllocator<16>::getInstance()->deallocate(static_cast<void*>(ptr), sz);
        } else if (sz * sizeof(T) == 24) {
            FixedAllocator<24>::getInstance()->deallocate(static_cast<void*>(ptr), sz);
        } else if (sz * sizeof(T) == 32) {
            FixedAllocator<32>::getInstance()->deallocate(static_cast<void*>(ptr), sz);
        } else {
            ::operator delete(ptr);
        }
    }
};

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

        NodeAllocTraits::deallocate(nodeAlloc, cur, 1);
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
        other.root = nullptr;

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
