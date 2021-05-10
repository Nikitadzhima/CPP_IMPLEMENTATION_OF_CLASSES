#include<iostream>

class DeleterBase {
  public:
    virtual void operator()(void*) = 0;
    virtual ~DeleterBase() = default;
};

template<typename T, typename Deleter>
class DeleterDerived : public DeleterBase {
  public:
    Deleter deleter;
    DeleterDerived(Deleter deleter): deleter(deleter) {}
    void operator()(void* ptr) override {
        deleter(reinterpret_cast<T*>(ptr));
    }
};

template<typename T>
class DeleterT : public DeleterBase {
    void operator()(void* ptr) override {
        std::allocator<T>().destroy(reinterpret_cast<T*>(ptr));
        std::allocator<T>().deallocate(reinterpret_cast<T*>(ptr), 1);
    }
};

template<typename T, typename Allocator>
class DeleterWithAllocator : public DeleterBase {
  public:
    Allocator allocator;
    DeleterWithAllocator(Allocator allocator): allocator(allocator) {}
    void operator()(void* ptr) override {
        using rebindAlloc = typename std::allocator_traits<Allocator>::template rebind_alloc<T>;
        rebindAlloc alloc = allocator;
        std::allocator_traits<rebindAlloc>::destroy(alloc, reinterpret_cast<T*>(ptr));
    }
};


class AllocatorBase {
  public:
    virtual void deallocate(void* ptr) = 0;
    virtual bool isFromCPointer() = 0;
    virtual ~AllocatorBase() = default;
};

template<typename T, typename Allocator, bool isFromAllocateShared>
class AllocatorDerived : public AllocatorBase {
  public:
    Allocator allocator;
    AllocatorDerived(Allocator allocator): allocator(allocator) {}
    void deallocate(void* ptr) override {
        using rebindAlloc = typename std::allocator_traits<Allocator>::template rebind_alloc<int8_t>;
        rebindAlloc alloc = allocator;
        std::allocator_traits<rebindAlloc>::deallocate(alloc, reinterpret_cast<int8_t*>(ptr), isFromAllocateShared * sizeof(T) + 2 * sizeof(size_t) + sizeof(DeleterBase) + sizeof(AllocatorBase));
    }
    bool isFromCPointer() override {
        return !isFromAllocateShared;
    }
};


template<typename T>
class WeakPtr;




template<typename T>
class SharedPtr {
  private:
    template<typename Y>
    friend class SharedPtr;

    template<typename Y>
    friend class WeakPtr;

    T* object = nullptr;
    size_t* shared_count = nullptr;
    size_t* weak_count = nullptr;
    DeleterBase* deleter = nullptr;
    AllocatorBase* allocator = nullptr;

    template<typename Y, typename... Args>
    friend SharedPtr<Y> makeShared(Args&&... args);

    template<typename Y, typename Allocator, typename... Args>
    friend SharedPtr<Y> allocateShared(const Allocator& allocator, Args&&... args);



    template<typename Y>
    SharedPtr(Y* object, size_t* shared_count, size_t* weak_count, DeleterBase* deleter, AllocatorBase* allocator):
        object(object), shared_count(shared_count), weak_count(weak_count), deleter(deleter), allocator(allocator) {}

    SharedPtr(const WeakPtr<T>& other): SharedPtr(other.object, other.shared_count, other.weak_count, other.deleter, other.allocator) {
        if (shared_count) {
            ++*shared_count;
        }
    }

    void clear() {
        object = nullptr;
        shared_count = nullptr;
        weak_count = nullptr;
        deleter = nullptr;
        allocator = nullptr;
    }
  public:
    SharedPtr() = default;

    SharedPtr(T* cptr): object(cptr) {
        std::allocator<int8_t> alloc;
        int8_t* ptr = alloc.allocate(2 * sizeof(size_t) + sizeof(DeleterBase) + sizeof(AllocatorBase));
        shared_count = reinterpret_cast<size_t*>(ptr);
        *shared_count = 1;
        ptr += sizeof(size_t);

        weak_count = reinterpret_cast<size_t*>(ptr);
        *weak_count = 0;
        ptr += sizeof(size_t);

        deleter = reinterpret_cast<DeleterBase*>(ptr);
        new(deleter) DeleterT<T>();
        ptr += sizeof(DeleterBase);

        allocator = reinterpret_cast<AllocatorBase*>(ptr);
        new(allocator) AllocatorDerived<T, std::allocator<int8_t>, false>(alloc);
    }

    template<typename Y, typename Deleter, typename Allocator = std::allocator<Y>>
    SharedPtr(Y* object, Deleter newDeleter, Allocator newAllocator = Allocator()): object(object) {
        using rebindAlloc = typename std::allocator_traits<Allocator>::template rebind_alloc<int8_t>;
        rebindAlloc alloc = newAllocator;
        int8_t* ptr = alloc.allocate(2 * sizeof(size_t) + sizeof(DeleterBase) + sizeof(AllocatorBase));
        shared_count = reinterpret_cast<size_t*>(ptr);
        *shared_count = 1;
        ptr += sizeof(size_t);

        weak_count = reinterpret_cast<size_t*>(ptr);
        *weak_count = 0;
        ptr += sizeof(size_t);

        deleter = reinterpret_cast<DeleterBase*>(ptr);
        new(deleter) DeleterDerived<T, Deleter>(newDeleter);
        ptr += sizeof(DeleterBase);

        allocator = reinterpret_cast<AllocatorBase*>(ptr);
        new(allocator) AllocatorDerived<T, Allocator, false>(newAllocator);
    }

    SharedPtr(const SharedPtr& other): SharedPtr(other.object, other.shared_count, other.weak_count, other.deleter, other.allocator) {
        if (shared_count) {
            ++*shared_count;
        }
    }
    template<typename Y>
    SharedPtr(const SharedPtr<Y>& other): SharedPtr(other.object, other.shared_count, other.weak_count, other.deleter, other.allocator) {
        if (shared_count) {
            ++*shared_count;
        }
    }

    SharedPtr(SharedPtr&& other): SharedPtr(other.object, other.shared_count, other.weak_count, other.deleter, other.allocator) {
        other.clear();
    }
    template<typename Y>
    SharedPtr(SharedPtr<Y>&& other): SharedPtr(other.object, other.shared_count, other.weak_count, other.deleter, other.allocator) {
        other.clear();
    }

    SharedPtr& operator=(const SharedPtr& other) {
        SharedPtr copy = other;
        swap(copy);
        return *this;
    }
    template<typename Y>
    SharedPtr& operator=(const SharedPtr<Y>& other) {
        SharedPtr copy = other;
        swap(copy);
        return *this;
    }

    SharedPtr& operator=(SharedPtr&& other) {
        SharedPtr copy = std::move(other);
        swap(copy);
        return *this;
    }
    template<typename Y>
    SharedPtr& operator=(SharedPtr<Y>&& other) {
        SharedPtr copy = std::move(other);
        swap(copy);
        return *this;
    }

    ~SharedPtr() {
        if (!shared_count) {
            return;
        }
        --*shared_count;
        if (*shared_count == 0) {
            deleter->operator()(object);
            if (*weak_count == 0) {
                if (allocator->isFromCPointer()) {
                    allocator->deallocate(shared_count);
                } else {
                    allocator->deallocate(object);
                }
            }
        }
    }


    void swap(SharedPtr& other) {
        std::swap(object, other.object);
        std::swap(shared_count, other.shared_count);
        std::swap(weak_count, other.weak_count);
        std::swap(deleter, other.deleter);
        std::swap(allocator, other.allocator);
    }

    size_t use_count() const {
        return *shared_count;
    }

    void reset() {
        SharedPtr<T> objectT;
        swap(objectT);
    }

    template<typename Y>
    void reset(Y* object) {
        SharedPtr<T> objectT(object);
        swap(objectT);
    }

    T& operator*() const {
        return *object;
    }

    T* operator->() const {
        return object;
    }

    T* get() const {
        return object;
    }
};

template<typename Y, typename Allocator, typename... Args>
SharedPtr<Y> allocateShared(const Allocator& allocator, Args&&... args) {
    using rebindAlloc = typename std::allocator_traits<Allocator>::template rebind_alloc<int8_t>;
    rebindAlloc alloc = allocator;
    int8_t* ptr = std::allocator_traits<rebindAlloc>::allocate(alloc, sizeof(Y) + 2 * sizeof(size_t) + sizeof(DeleterBase) + sizeof(AllocatorBase));

    using rebindAllocY = typename std::allocator_traits<Allocator>::template rebind_alloc<Y>;
    rebindAllocY allocY = allocator;
    std::allocator_traits<rebindAllocY>::construct(allocY, reinterpret_cast<Y*>(ptr), std::forward<Args>(args)...);

    ptr += sizeof(Y);
    size_t* shared_count = reinterpret_cast<size_t*>(ptr);
    *shared_count = 1;
    ptr += sizeof(size_t);

    size_t* weak_count = reinterpret_cast<size_t*>(ptr);
    *weak_count = 0;
    ptr += sizeof(size_t);

    DeleterBase* deleter = reinterpret_cast<DeleterBase*>(ptr);
    new(deleter) DeleterWithAllocator<Y, Allocator>(allocY);
    ptr += sizeof(DeleterBase);

    AllocatorBase* newAlloc = reinterpret_cast<AllocatorBase*>(ptr);
    new(newAlloc) AllocatorDerived<Y, Allocator, true>(allocY);

    ptr -= sizeof(Y) + 2 * sizeof(size_t) + sizeof(DeleterBase);
    return SharedPtr<Y>(reinterpret_cast<Y*>(ptr), shared_count, weak_count, deleter, newAlloc);
}

template<typename Y, typename... Args>
SharedPtr<Y> makeShared(Args&&... args) {
    return allocateShared<Y>(std::allocator<Y>(), std::forward<Args>(args)...);
}






template<typename T>
class WeakPtr {
  private:

    template<typename Y>
    friend class WeakPtr;

    template<typename Y>
    friend class SharedPtr;

    T* object = nullptr;
    size_t* shared_count = nullptr;
    size_t* weak_count = nullptr;
    DeleterBase* deleter = nullptr;
    AllocatorBase* allocator = nullptr;

    template<typename Y>
    WeakPtr(Y* object, size_t* shared_count, size_t* weak_count, DeleterBase* deleter, AllocatorBase* allocator):
        object(object), shared_count(shared_count), weak_count(weak_count), deleter(deleter), allocator(allocator) {}

    void clear() {
        object = nullptr;
        shared_count = nullptr;
        weak_count = nullptr;
        deleter = nullptr;
        allocator = nullptr;
    }
  public:
    WeakPtr() = default;

    template<typename Y>
    WeakPtr(const SharedPtr<Y>& other): WeakPtr(other.object, other.shared_count, other.weak_count, other.deleter, other.allocator) {
        if (weak_count) {
            ++*weak_count;
        }
    }

    WeakPtr(const WeakPtr& other): WeakPtr(other.object, other.shared_count, other.weak_count, other.deleter, other.allocator) {
        if (weak_count) {
            ++*weak_count;
        }
    }
    template<typename Y>
    WeakPtr(const WeakPtr<Y>& other): WeakPtr(other.object, other.shared_count, other.weak_count, other.deleter, other.allocator) {
        if (weak_count) {
            ++*weak_count;
        }
    }

    WeakPtr(WeakPtr&& other): WeakPtr(other.object, other.shared_count, other.weak_count, other.deleter, other.allocator) {
        other.clear();
    }
    template<typename Y>
    WeakPtr(WeakPtr<Y>&& other): WeakPtr(other.object, other.shared_count, other.weak_count, other.deleter, other.allocator) {
        other.clear();
    }

    WeakPtr& operator=(const WeakPtr& other) {
        WeakPtr copy = other;
        swap(copy);
        return *this;
    }
    template<typename Y>
    WeakPtr& operator=(const WeakPtr<Y>& other) {
        WeakPtr copy = other;
        swap(copy);
        return *this;
    }

    WeakPtr& operator=(WeakPtr&& other) {
        WeakPtr copy = std::move(other);
        swap(copy);
        return *this;
    }
    template<typename Y>
    WeakPtr& operator=(WeakPtr<Y>&& other) {
        WeakPtr copy = std::move(other);
        swap(copy);
        return *this;
    }

    ~WeakPtr() {
        if (!weak_count) {
            return;
        }
        --*weak_count;
        if (*weak_count == 0 && *shared_count == 0) {
            if (allocator->isFromCPointer()) {
                allocator->deallocate(shared_count);
            } else {
                allocator->deallocate(object);
            }
        }
    }

    void swap(WeakPtr& other) {
        std::swap(object, other.object);
        std::swap(shared_count, other.shared_count);
        std::swap(weak_count, other.weak_count);
        std::swap(deleter, other.deleter);
        std::swap(allocator, other.allocator);
    }

    bool expired() const {
        return !shared_count || *shared_count == 0;
    }

    SharedPtr<T> lock() const {
        if (!expired()) {
            return SharedPtr<T>(*this);
        }
        return SharedPtr<T>();
    }

    size_t use_count() const {
        return *shared_count;
    }
};
