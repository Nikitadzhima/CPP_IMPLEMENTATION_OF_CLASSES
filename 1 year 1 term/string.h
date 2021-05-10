#include <iostream>
#include <cstring>

class String {
  private:
    size_t sz_ = 0;
    size_t cap_ = 1;
    char* str_ = new char[1];

  private:
    void increase_size() {
        size_t new_cap = cap_ * 2;
        char* cur_s = new char[new_cap];
        memcpy(cur_s, str_, cap_);
        delete[] str_;
        str_ = cur_s;
        cap_ = new_cap;
    }

  public:
    String() {}

    String(const char* s): sz_(strlen(s)), cap_(2 * sz_ + 1), str_(new char [cap_]) {
        memcpy(str_, s, sz_);
    }

    String(size_t sz, char val): sz_(sz), cap_(2 * sz_ + 1), str_(new char[cap_]) {
        memset(str_, val, sz_);
    }

    String(char val): String(1, val) {}

    String(const String& s): sz_(s.sz_), cap_(2 * sz_ + 1), str_(new char[cap_]) {
        memcpy(str_, s.str_, sz_);
    }

    String& operator=(const String& s) {
        if (&s == this) {
            return *this;
        }
        String cur_s = s;
        std::swap(sz_, cur_s.sz_);
        std::swap(cap_, cur_s.cap_);
        std::swap(str_, cur_s.str_);
        return *this;
    }

    char& operator[](size_t id) {
        return str_[id];
    }

    char operator[](size_t id) const {
        return str_[id];
    }

    ~String() {
        delete[] str_;
    }

    size_t length() const {
        return sz_;
    }

    char& front() {
        return *str_;
    }

    char front() const {
        return *str_;
    }

    char& back() {
        return *(str_ + sz_ - 1);
    }

    char back() const {
        return *(str_ + sz_ - 1);
    }

    bool empty() const {
        return (sz_ == 0);
    }

    void clear() {
        sz_ = 0;
        cap_ = 1;
        delete[] str_;
        str_ = new char[1];
    }

    void push_back(char val) {
        if (sz_ == cap_) {
            increase_size();
        }
        str_[sz_] = val;
        ++sz_;
    }

    void pop_back() {
        --sz_;
    }

    String& operator+=(char val) {
        push_back(val);
        return *this;
    }

    String& operator+=(const String& s) {
        size_t new_sz = sz_ + s.sz_;
        size_t new_cap = (new_sz > 0) ? (new_sz << 1) : 1;
        char* cur_s = new char[new_cap];

        memcpy(cur_s, str_, sz_);
        memcpy(cur_s + sz_, s.str_, s.sz_);

        delete[] str_;
        str_ = cur_s;
        sz_ = new_sz;
        cap_ = new_cap;
        return *this;
    }

    String substr(size_t start, size_t count) const {
        String cur_s;
        cur_s.sz_ = count;
        cur_s.cap_ = count;
        cur_s.str_ = new char[cap_];
        memcpy(cur_s.str_, str_ + start, count);
        return cur_s;
    }

    size_t find_substr(const String& word, bool first) const;

    size_t find(const String& word) const {
        return find_substr(word, true);
    }
    size_t rfind(const String& word) const {
        return find_substr(word, false);
    }

    bool operator==(const String& s) const {
        if (sz_ != s.sz_) {
            return false;
        }
        for (size_t i = 0; i < sz_; ++i) {
            if (str_[i] != s[i]) {
                return false;
            }
        }
        return true;
    }
};

String operator+(const String& s1, const String& s2) {
    String new_s = s1;
    new_s += s2;
    return new_s;
}


size_t String::find_substr(const String& word, bool first) const {
    size_t ans = sz_;
    for (size_t id = 0; id < sz_; ++id) {
        bool f = true;
        for (size_t i = 0; i < word.sz_; ++i) {
            if (id + i >= sz_ || word.str_[i] != str_[id + i]) {
                f = false;
            }
        }
        if (f) {
            ans = id;
            if (first) {
                break;
            }
        }
    }
    return ans;
}

std::istream& operator>>(std::istream& in, String& s) {
    char val;
    bool isFirst = true;
    s.clear();
    while ((val = in.get())) {
        if (val == EOF) {
            break;
        }
        if (iswspace(val)) {
            if (!isFirst) {
                break;
            }
        } else {
            isFirst = false;
            s.push_back(val);
        }
    }
    return in;
}

std::ostream& operator<<(std::ostream& out, const String& s) {
    size_t sz = s.length();
    for (size_t i = 0; i < sz; ++i) {
        out << s[i];
    }
    return out;
}
