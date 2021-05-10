#include <iostream>
#include <string>
#include <cstring>
#include <vector>

class BigInteger {
    friend std::istream& operator>>(std::istream& in, BigInteger& x);
  private:
    static const int base = 1'000'000'000;
    std::vector<int> num;
    bool isNotNeg;

    void normalize() {
        if (num.size() == 0) {
            num.push_back(0);
        }
        while (num.size() > 1 && num.back() == 0) {
            num.pop_back();
        }
        if (num.size() == 1 && num[0] == 0) {
            isNotNeg = true;
        }
    }

    static BigInteger stupid_mul(const BigInteger& a, const BigInteger& x);

    static void addNilsToEqualizeSizes(BigInteger& a, BigInteger& b);

    std::pair<BigInteger, BigInteger> split() const;

  public:
    BigInteger() : num({0}), isNotNeg(true) {}

    BigInteger(long long x) {
        isNotNeg = true;
        if (x < 0) {
            isNotNeg = false;
            x = -x;
        }
        do {
            num.push_back(x % base);
            x /= base;
        } while (x > 0);
    }

    void invertSign() {
        isNotNeg = !isNotNeg;
        if (num.size() == 1 && num[0] == 0) {
            isNotNeg = true;
        }
    }

    bool getSign() const {
        return isNotNeg;
    }

    int getLastNumbers() const {
        return num[0];
    }

    size_t getLength() const {
        return num.size();
    }

    std::vector<int> getNum() const {
        return num;
    }

    long double toDouble() const {
        long double ans = 0;
        long double b = 1;
        for (size_t i = 0; i < num.size(); ++i) {
            ans += num[i] * b;
            b *= base;
        }
        if (!isNotNeg) {
            ans = -ans;
        }
        return ans;
    }

    std::string toString() const {
        std::string cur = "";
        if (!isNotNeg) {
            cur = "-";
        }
        if (num.size() != 0) {
            cur += std::to_string(num.back());
        }
        for (int i = static_cast<int>(num.size()) - 2; i >= 0; --i) {
            int a = base;
            int b = num[i];
            for (int j = 0; j < 9; ++j) {
                a /= 10;
                cur += '0' + b / a;
                b %= a;
            }
        }
        return cur;
    }


    BigInteger operator-() const {
        BigInteger ans;
        ans = *this;
        ans.isNotNeg = !ans.isNotNeg;
        ans.normalize();
        return ans;
    }

    explicit operator bool() const;

    BigInteger& operator+=(const BigInteger& x);
    BigInteger& operator-=(const BigInteger& x);
    BigInteger& operator*=(const BigInteger& x);
    BigInteger& operator/=(const BigInteger& x);
    BigInteger& operator%=(const BigInteger& x);


    BigInteger& operator++() {
        *this += 1;
        return *this;
    }

    BigInteger operator++(int) {
        BigInteger cur = *this;
        ++*this;
        return cur;
    }

    BigInteger& operator--() {
        *this -= 1;
        return *this;
    }

    BigInteger operator--(int) {
        BigInteger cur = *this;
        --*this;
        return cur;
    }
};


bool operator==(const BigInteger& a, const BigInteger& x) {
    return !(a.getSign() != x.getSign() || a.getLength() != x.getLength() || a.getNum() != x.getNum());
}

bool operator!=(const BigInteger& a, const BigInteger& x) {
    return !(a == x);
}

bool operator<(const BigInteger& a, const BigInteger& x) {
    bool asign = a.getSign();
    bool xsign = x.getSign();
    if (!asign && !xsign) {
        return -x < -a;
    } else if (!asign) {
        return true;
    } else if (!xsign) {
        return false;
    }
    size_t len1 = a.getLength();
    size_t len2 = x.getLength();
    if (len1 != len2) {
        return len1 < len2;
    }
    std::vector<int> anum = a.getNum(),
                      xnum = x.getNum();
    for (int i = static_cast<int>(anum.size()) - 1; i >= 0; --i) {
        if (anum[i] != xnum[i]) {
            return anum[i] < xnum[i];
        }
    }
    return false;
}

bool operator<=(const BigInteger& a, const BigInteger& x) {
    return !(x < a);
}

bool operator>(const BigInteger& a, const BigInteger& x) {
    return x < a;
}

bool operator>=(const BigInteger& a, const BigInteger& x) {
    return !(a < x);
}

BigInteger::operator bool() const {
    return *this != 0;
}

BigInteger& BigInteger::operator+=(const BigInteger& x) {
    BigInteger cur;
    if ((*this < 0 && x > 0) || (*this > 0 && x < 0)) {
        return *this -= -x;
    }
    if (*this == 0 && x < 0) {
        this->isNotNeg = false;
    }
    for (size_t i = 0; i < x.num.size(); ++i) {
        if (i >= num.size()) {
            num.push_back(0);
        }
        num[i] += x.num[i];
    }
    num.push_back(0);
    for (size_t i = 0; i + 1 < num.size(); ++i) {
        if (num[i] >= base) {
            num[i] -= base;
            ++num[i + 1];
        }
    }
    normalize();
    return *this;
}

BigInteger& BigInteger::operator-=(const BigInteger& x) {
    if ((!this->isNotNeg && x.isNotNeg) || (this->isNotNeg && !x.isNotNeg)) {
        return *this += -x;
    }
    if ((x.isNotNeg && x > *this) || (!x.isNotNeg && x < *this)) {
        BigInteger cur = x;
        cur -= *this;
        return *this = -cur;
    }

    for (size_t i = 0; i < x.num.size(); ++i) {
        num[i] -= x.num[i];
    }
    for (size_t i = 0; i < num.size(); ++i) {
        if (num[i] < 0) {
            num[i] += base;
            --num[i + 1];
        }
    }

    normalize();
    return *this;
}

BigInteger BigInteger::stupid_mul(const BigInteger& a, const BigInteger& x) {
    BigInteger ans = 0;
    ans.num.resize(a.num.size() + x.num.size(), 0);
    for (size_t i = 0; i < a.num.size(); ++i) {
        if (a.num[i] == 0) {
            continue;
        }
        int nxt = 0;
        size_t j = 0;
        for (j = 0; j < x.num.size(); ++j) {
            long long cur = static_cast<long long>(a.num[i]) * x.num[j] + nxt + ans.num[i + j];
            ans.num[i + j] = cur % base;
            nxt = cur / base;
        }
        while (nxt) {
            long long cur = static_cast<long long>(nxt) + ans.num[i + j];
            ans.num[i + j] = cur % base;
            nxt = cur / base;
            ++j;
        }
    }

    ans.normalize();
    return ans;
}


void BigInteger::addNilsToEqualizeSizes(BigInteger& a, BigInteger& b) {
    while (a.num.size() < b.num.size()) {
        a.num.push_back(0);
    }
    while (b.num.size() < a.num.size()) {
        b.num.push_back(0);
    }
}

std::pair <BigInteger, BigInteger> BigInteger::split() const {
    BigInteger a1(0);
    BigInteger a2(0);
    a1.num.clear(), a2.num.clear();

    for (size_t i = 0; i < num.size() / 2; i++) {
        a1.num.push_back(num[i]);
    }
    for (size_t i = num.size() / 2; i < num.size(); i++) {
        a2.num.push_back(num[i]);
    }

    a1.normalize();
    a2.normalize();
    return {a1, a2};
}

BigInteger& BigInteger::operator*=(const BigInteger& x) {
    bool sig = true;
    if (isNotNeg != x.isNotNeg) {
        sig = false;
    }
    if (num.size() * x.num.size() < 100) {
        *this = stupid_mul(*this, x);
        this->isNotNeg = sig;
        normalize();
        return *this;
    }

    BigInteger a = *this;
    BigInteger b = x;

    addNilsToEqualizeSizes(a, b);

    BigInteger aa = 0;
    aa.num.clear();
    for (size_t i = 0; i < a.num.size() / 2; i++) {
        aa.num.push_back(0);
    }
    aa.num.push_back(1);

    auto A = a.split();
    BigInteger a1 = A.first;
    BigInteger a2 = A.second;
    auto B = b.split();
    BigInteger b1 = B.first;
    BigInteger b2 = B.second;

    BigInteger p1 = a1;
    p1 *= b1;
    BigInteger p2 = a2;
    p2 *= b2;

    a1 += a2, b1 += b2;
    a1 *= b1;
    a1 -= p1;
    a1 -= p2;
    a2 = stupid_mul(aa, a1);
    aa = stupid_mul(aa, aa);
    aa = stupid_mul(aa, p2);
    p1 += a2;
    p1 += aa;

    *this = p1;
    this->isNotNeg = sig;

    normalize();
    return *this;
}

BigInteger& BigInteger::operator/=(const BigInteger& x) {
    BigInteger a = *this;
    BigInteger b = x;
    BigInteger ans, c = 0;
    if (ans.num.size() != 0)ans.num.clear();
    if (b < 0) b = -b;
    if (a < 0) a = -a;
    if (c.num.size() != 0) c.num.clear();
    c.isNotNeg = true;
    for (int i = static_cast<int>(a.num.size()) - 1; i >= 0; --i) {
        for (size_t j = 0; j < c.num.size() / 2; ++j) {
            std::swap(c.num[j], c.num[c.num.size() - j - 1]);
        }
        c.num.push_back(a.num[i]);
        for (size_t j = 0; j < c.num.size() / 2; ++j) {
            std::swap(c.num[j], c.num[c.num.size() - j - 1]);
        }
        while (c.num.size() > 1 && c.num.back() == 0) c.num.pop_back();
        BigInteger cur;
        if (c >= b) {
            int l = 1, r = base;
            while (r - l > 1) {
                int mid = (r + l) / 2;
                cur = b;
                cur *= mid;
                if (cur <= c) l = mid;
                else r = mid;
            }
            ans.num.push_back(l);
            cur = b;
            cur *= l;
            c -= cur;
        } else {
            ans.num.push_back(0);
        }
    }
    for (size_t j = 0; j < ans.num.size() / 2; ++j) {
        std::swap(ans.num[j], ans.num[ans.num.size() - j - 1]);
    }
    ans.isNotNeg = (isNotNeg == x.isNotNeg);
    *this = ans;
    normalize();
    return *this;
}

BigInteger& BigInteger::operator%=(const BigInteger& x) {
    BigInteger cur = *this;
    cur /= x;
    cur *= x;
    *this -= cur;
    return *this;
}

BigInteger operator ""_bi(unsigned long long x) {
    return BigInteger(x);
}

BigInteger operator+(const BigInteger& a, const BigInteger& b) {
    BigInteger x = a;
    x += b;
    return x;
}

BigInteger operator-(const BigInteger& a, const BigInteger& b) {
    BigInteger x = a;
    x -= b;
    return x;
}

BigInteger operator*(const BigInteger& a, const BigInteger& b) {
    BigInteger x = a;
    x *= b;
    return x;
}

BigInteger operator/(const BigInteger& a, const BigInteger& b) {
    BigInteger x = a;
    x /= b;
    return x;
}

BigInteger operator%(const BigInteger& a, const BigInteger& b) {
    BigInteger x = a;
    x %= b;
    return x;
}

std::istream& operator>>(std::istream& in, BigInteger& x) {
    char val;
    bool isFirst = true;
    x.isNotNeg = true;
    std::string cur = "";
    bool is_null = true;
    while ((val = in.get())) {
        if (val == EOF) {
            break;
        }
        if (iswspace(val)) {
            if (!isFirst) {
                break;
            }
            continue;
        }
        if (val == '-') {
            x.isNotNeg = false;
        } else if (val == '+') {
            x.isNotNeg = true;
        } else if (val != '0' || (val == '0' && !is_null)) {
            cur += val;
            is_null = false;
        }
        isFirst = false;
    }
    if (is_null) {
        cur = "0";
        x.isNotNeg = true;
    }
    size_t sz = cur.size();
    if (x.num.size() != 0) {
        x.num.clear();
    }
    for (int i = static_cast<int>(sz); i > 0; i -= 9) {
        int a = 0;
        for (int j = std::max(i - 9, 0); j < i; ++j) {
            a = a * 10 + cur[j] - '0';
        }
        x.num.push_back(a);
    }
    return in;
}

std::ostream& operator<<(std::ostream& out, const BigInteger& x) {
    std::string num = x.toString();
    out << num;
    return out;
}

BigInteger gcd(const BigInteger& a, const BigInteger& b) {
    BigInteger aa = a;
    BigInteger bb = b;
    while (bb) {
        aa %= bb;
        std::swap(aa, bb);
    }
    return aa;
}

class Rational {
  private:
    BigInteger n = 0;
    BigInteger m = 1;

    void normalize() {
        if (!m.getSign()) {
            n.invertSign();
            m.invertSign();
        }
        bool f = false;
        if (!n.getSign()) {
            f = true;
            n.invertSign();
        }
        BigInteger curGcd = gcd(n, m);
        n /= curGcd;
        m /= curGcd;
        if (f) {
            n.invertSign();
        }
    }

  public:
    Rational() : n(0), m(1) {}

    Rational(const BigInteger x) : n(x), m(1) {}

    Rational (const int& x) : n(x), m(1) {}

    Rational (const BigInteger x, const BigInteger y) : n(x), m(y) {}

    BigInteger getn() const {
        return n;
    }

    BigInteger getm() const {
        return m;
    }

    size_t getNsize() const {
        return n.getLength();
    }

    size_t getMsize() const {
        return m.getLength();
    }

    Rational operator-() const {
        Rational ans = *this;
        ans.n.invertSign();
        return ans;
    }

    std::string toString() const {
        std::string cur = "";
        cur = n.toString();
        if (m == 1) {
            return cur;
        }
        cur += '/';
        cur += m.toString();
        return cur;
    }

    Rational& operator+=(const Rational& x) {
        n = n * x.m + x.n * m;
        m *= x.m;
        normalize();
        return *this;
    }

    Rational& operator-=(const Rational& x) {
        n = n * x.m - x.n * m;
        m *= x.m;
        normalize();
        return *this;
    }

    Rational& operator*=(const Rational& x) {
        BigInteger nul(0);
        if (x.getn() == nul || this->getn() == nul) {
            return *this = static_cast<Rational>(0);
        }
        n *= x.n;
        m *= x.m;
        normalize();
        return *this;
    }

    Rational& operator/=(const Rational& x) {
        n *= x.m;
        m *= x.n;
        normalize();
        return *this;
    }

    std::string asDecimal(size_t precision = 0) const {
        BigInteger a = n;
        std::string ans = "";
        bool isNeg = false;
        if (a < 0) {
            a.invertSign();
            isNeg = true;
        }
        for (size_t i = 0; i < precision + 1; ++i) {
            a *= 10;
        }
        a /= m;
        if (a % 10 >= 5) {
            a += 5;
        }
        a /= 10;
        std::string s = a.toString();
        if (precision == 0) {
            if (isNeg && a != 0) {
                s = "-" + s;
            }
            return s;
        }
        std::string cur = "";
        for (int i = 0; i < static_cast<int>(precision) - static_cast<int>(s.size()) + 1; ++i) {
            cur += "0";
        }
        s = cur + s;
        ans = "";
        for (size_t i = 0; i < s.size(); ++i) {
            if (s[i] != 0) {
                if (isNeg) {
                    ans += "-";
                }
                break;
            }
        }
        for (size_t i = 0; i < s.size(); ++i) {
            if (s.size() - i == precision) {
                ans += ".";
            }
            ans += s[i];
        }
        return ans;
    }
    explicit operator double() const {
        return std::stod(asDecimal(10));
    }
};


std::ostream& operator<<(std::ostream& out, const Rational& x) {
    out << x.toString();
    return out;
}

std::istream& operator>>(std::istream& in, Rational& x) {
    BigInteger y;
    in >> y;
    x = static_cast<Rational>(y);
    return in;
}

bool operator==(const Rational& a, const Rational& x) {
    return a.getn() == x.getn() && a.getm() == x.getm();
}

bool operator!=(const Rational& a, const Rational& x) {
    return !(a == x);
}

bool operator<(const Rational& a, const Rational& x) {
    return a.getn() * x.getm() < a.getm() * x.getn();
}

bool operator<=(const Rational& a, const Rational& x) {
    return !(x < a);
}

bool operator>(const Rational& a, const Rational& x) {
    return x < a;
}

bool operator>=(const Rational& a, const Rational& x) {
    return !(a < x);
}

Rational operator+(const Rational& a, const Rational& b) {
    Rational ans = a;
    ans += b;
    return ans;
}

Rational operator-(const Rational& a, const Rational& b) {
    Rational ans = a;
    ans -= b;
    return ans;
}

Rational operator*(const Rational& a, const Rational& b) {
    Rational ans = a;
    ans *= b;
    return ans;
}

Rational operator/(const Rational& a, const Rational& b) {
    Rational ans = a;
    ans /= b;
    return ans;
}
