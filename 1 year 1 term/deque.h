#include <iostream>
#include <vector>

template<typename T>
class Deque {
  private:
    std::vector<T> d;

    std::vector<int> info; // delta, start, finish: [start, finish)


    int fromRealToVec(int realId) const {
        return realId + info[1];
    }

    int fromVecToFake(int vecId) const {
        return vecId - info[0];
    }

    void shift() {
        size_t sz = size();
        if (sz == 0) {
            d.resize(1);
            info[1] = 1;
            info[2] = 1;
            return;
        }
        d.resize(2 * sz);
        for (size_t i = 0; i < sz; ++i) {
            std::swap(d[i], d[i + sz]);
        }
        info[0] += sz;
        info[1] += sz;
        info[2] += sz;
    }
  public:
    Deque() {
        info = {0, 0, 0};
    }

    Deque(int sz, T val = T()) {
        d.resize(sz, val);
        info = {0, 0, sz};
    }

    size_t size() const {
        return info[2] - info[1];
    }

    T operator[](int realId) const {
        return d[fromRealToVec(realId)];
    }

    T& operator[](int realId) {
        return d[fromRealToVec(realId)];
    }

    T at(int realId) const {
        if (realId < 0 || realId >= static_cast<int>(size())) {
            throw std::out_of_range("Deque: out of range");
            return d[0];
        }
        return d[fromRealToVec(realId)];
    }

    T& at(int realId) {
        if (realId < 0 || realId >= static_cast<int>(size())) {
            throw std::out_of_range("Deque: out of range");
            return d[0];
        }
        return d[fromRealToVec(realId)];
    }

    void push_back(T val) {
        d.push_back(val);
        ++info[2];
    }

    void push_front(T val) {
        if (!info[1]) {
            shift();
        }
        --info[1];
        d[info[1]] = val;
    }

    void pop_back() {
        --info[2];
        d.pop_back();
    }

    void pop_front() {
        ++info[1];
    }

    class const_iterator {
      protected:
        int fakeId;

        std::vector<T>* d;

        std::vector<int>* info;


        int fromFakeToVec(int fakeId) const {
            return ((fakeId + (*(this->info))[0] < 0) ? 2 : fakeId + (*(this->info))[0]);
        }

        int fromVecToReal(int vecId) const {
            return vecId - (*(this->info))[1];
        }
      public:
        const_iterator() = default;

        const_iterator(int fakeId, std::vector<T>* d, std::vector<int>* info): fakeId(fakeId), d(d), info(info) {}

        const_iterator operator+(int delta) const {
            return const_iterator(fakeId + delta, d, info);
        }

        const_iterator operator-(int delta) const {
            return *this + (-delta);
        }

        const_iterator& operator++() {
            *this = *this + 1;
            return *this;
        }

        const_iterator& operator--() {
            *this = *this - 1;
            return *this;
        }

        const_iterator operator++(int) {
            const_iterator cur = *this;
            ++*this;
            return cur;
        }

        const_iterator operator--(int) {
            const_iterator cur = *this;
            --*this;
            return cur;
        }

        bool operator==(const const_iterator other) const {
            return d == other.d && fakeId == other.fakeId;
        }

        bool operator!=(const const_iterator other) const {
            return !(*this == other);
        }

        bool operator<(const const_iterator other) const {
            return d == other.d && fakeId < other.fakeId;
        }

        bool operator>(const const_iterator other) const {
            return other < *this;
        }

        bool operator<=(const const_iterator other) const {
            return !(*this > other);
        }

        bool operator>=(const const_iterator other) const {
            return !(*this < other);
        }

        int operator-(const const_iterator other) const {
            return fakeId - other.fakeId;
        }

        const T& operator*() const {
            return (*d)[this->fromFakeToVec(fakeId)];
        }

        const T* operator->() const {
            return &((*d)[this->fromFakeToVec(fakeId)]);
        }
    };

    class iterator: public const_iterator {
      public:
        iterator() = default;

        iterator(int fakeId, std::vector<T>* d, std::vector<int>* info): const_iterator(fakeId, d, info) {}

        iterator operator+(int delta) const {
            return iterator(this->fakeId + delta, this->d, this->info);
        }

        iterator operator-(int delta) const {
            return *this + (-delta);
        }

        int operator-(const iterator other) const {
            return this->fakeId - other.fakeId;
        }

        T& operator*() const {
            return (*(this->d))[this->fromFakeToVec(this->fakeId)];
        }

        T* operator->() const {
            return &((*(this->d))[this->fromFakeToVec(this->fakeId)]);
        }
    };

    iterator begin() {
        return iterator(fromVecToFake(info[1]), &d, &info);
    }

    const_iterator cbegin() const {
        return const_iterator(fromVecToFake(info[1]), const_cast<std::vector<T>*>(&d), const_cast<std::vector<int>*>(&info));
    }

    const_iterator begin() const {
        return cbegin();
    }

    iterator end() {
        return iterator(fromVecToFake(info[2]), &d, &info);
    }

    const_iterator cend() const {
        return const_iterator(fromVecToFake(info[2]), const_cast<std::vector<T>*>(&d), const_cast<std::vector<int>*>(&info));
    }

    const_iterator end() const {
        return cbegin();
    }

    void insert(iterator id, T x) {
        push_back(x);
        for (iterator it = end() - 1; it > id; --it) {
            std::swap(*it, *(it - 1));
        }
    }

    void erase(iterator id) {
        for (iterator it = id; it < (end() - 1); ++it) {
            std::swap(*it, *(it + 1));
        }
        pop_back();
    }
}
