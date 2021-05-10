#include <vector>
#include <iostream>
#include <string>
#include <cstring>
#include <algorithm>
#include <math.h>
#include <iomanip>

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

    BigInteger stupid_mul(const BigInteger& a, const BigInteger& x);

    void addNilsToEqualizeSizes(BigInteger& a, BigInteger& b);

    std::pair<BigInteger, BigInteger> split(const BigInteger& a) const;

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

std::pair <BigInteger, BigInteger> BigInteger::split(const BigInteger& a) const {
    BigInteger a1(0);
    BigInteger a2(0);
    a1.num.clear(), a2.num.clear();

    for (size_t i = 0; i < a.num.size() / 2; i++) {
        a1.num.push_back(a.num[i]);
    }
    for (size_t i = a.num.size() / 2; i < a.num.size(); i++) {
        a2.num.push_back(a.num[i]);
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

    auto A = split(a);
    BigInteger a1 = A.first;
    BigInteger a2 = A.second;
    auto B = split(b);
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
    BigInteger ans, c;
    c.isNotNeg = true;
    for (int i = static_cast<int>(this->num.size()) - 1; i >= 0; --i) {
        for (size_t j = 0; j < c.num.size() / 2; ++j) {
            std::swap(c.num[j], c.num[c.num.size() - j - 1]);
        }
        c.num.push_back(this->num[i]);
        for (size_t j = 0; j < c.num.size() / 2; ++j) {
            std::swap(c.num[j], c.num[c.num.size() - j - 1]);
        }
        while (c.num.size() > 1 && c.num.back() == 0) {
            c.num.pop_back();
        }
        BigInteger cur;
        if (c >= x) {
            int l = 1, r = base;
            while (r - l > 1) {
                int mid = (r + l) / 2;
                cur = x;
                cur *= mid;
                if (cur <= c) {
                    l = mid;
                } else {
                    r = mid;
                }
            }
            ans.num.push_back(l);
            cur = x;
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
    if (*this < x) {
        return *this;
    }
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
    BigInteger* pa = &aa;
    BigInteger* pb = &bb;
    int cnt = 10;
    while (cnt-- && *pb) {
        *pa %= *pb;
        std::swap(pa, pb);
    }
    while (*pb) {
        if (*pa < 1000000000 && *pb < 1000000000) {
            return std::__gcd(pa->getLastNumbers(), pb->getLastNumbers());
        }
        while (*pa >= *pb) {
            *pa -= *pb;
        }
        if (!*pa) {
            return *pb;
        }
        do {
            *pb -= *pa;
        } while (*pb >= *pa);
    }
    return *pa;
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

template<bool f>
struct makeCompileErrorIfFalse {
    int a[-1 + f];
    static const bool value = true;
};


namespace {

template<unsigned N, unsigned D>
struct is_prime_helper {
    static const bool value = N % D == 0 ? false : is_prime_helper<N, D - 1 - (D & 1)>::value;
};

template<unsigned N>
struct is_prime_helper<N, 1> {
    static const bool value = true;
};

template<unsigned N>
struct is_prime_helper<N, 0> {
    static const bool value = true;
};

} // namespace helpers


template<unsigned N>
struct is_prime {
    static const bool value = (N <= 1 || (N > 2 && !(N & 1))) ? false : is_prime_helper<N, static_cast<unsigned>(std::floor(sqrt(N) + 0.5))>::value;
};

template<unsigned N>
static const bool is_prime_v = is_prime<N>::value;


namespace {
template<unsigned P, unsigned N>
struct has_primitive_root_helper {
    static const unsigned value = has_primitive_root_helper<P - 1, N>::value > 0 ? has_primitive_root_helper<P - 1, N>::value : (N % P == 0 ? P : 0);
};

template<unsigned N>
struct has_primitive_root_helper<1, N> {
    static const unsigned value = 0;
};

template<unsigned N>
struct has_primitive_root_helper<0, N> {
    static const unsigned value = 0;
};

template<>
struct has_primitive_root_helper<0, 0> {
    static const unsigned value = 0;
};

template<unsigned P>
struct has_primitive_root_helper<P, 0> {
    static const unsigned value = 0;
};

template<unsigned P, unsigned N>
struct is_deg_of {
    static const bool value = N % P == 0 ? is_deg_of<P, N / P>::value : false;
};

template<unsigned N>
struct is_deg_of<N, N> {
    static const bool value = true;
};

template <unsigned N>
struct is_deg_of<0, N> {
    static const bool value = true;
};

template<unsigned P>
struct is_deg_of<P, 0> {
    static const bool value = true;
};

template<>
struct is_deg_of<0, 0> {
    static const bool value = true;
};
} // namespace helpers


template<unsigned N>
struct has_primitive_root {
    static const bool value = is_prime_v<N> ? true : (((N & 1) == 0 && is_prime_v<N / 2>) ? true :
                ((N & 1) ? (is_prime_v<has_primitive_root_helper<static_cast<unsigned>(std::floor(sqrt(N) + 0.5)), N>::value>
                && is_deg_of<has_primitive_root_helper<static_cast<unsigned>(std::floor(sqrt(N) + 0.5)), N>::value, N>::value) :
                (((N & 3) != 0) && is_prime_v<has_primitive_root_helper<static_cast<unsigned>(std::floor(sqrt(N / 2) + 0.5)), N / 2>::value>
                && is_deg_of<has_primitive_root_helper<static_cast<unsigned>(std::floor(sqrt(N / 2) + 0.5)), N>::value, N / 2>::value)));
};

template<>
struct has_primitive_root<2> {
    static const bool value = true;
};

template<>
struct has_primitive_root<4> {
    static const bool value = true;
};

template<unsigned N>
static const bool has_primitive_root_v = has_primitive_root<N>::value;

long long phi(long long n) {
    long long ans = n;
    for (long long i = 2; i * i <= n; ++i) {
        if (n % i == 0) {
            while (n % i == 0) {
                n /= i;
            }
            ans -= ans / i;
        }
    }
    if (n > 1) {
        ans -= ans / n;
    }
    return ans;
}

template<unsigned Mod>
class Residue {
  private:
    int x;

  public:
    Residue() = default;

    explicit Residue(int xx) {
        if (xx >= 0) {
            x = xx % Mod;
        } else {
            x = (Mod - (-xx) % Mod) % Mod;
        }
    }

    explicit operator int() const {
        return x;
    }

    Residue operator+=(const Residue other) {
        x = (x + other.x) % Mod;
        return *this;
    }

    Residue operator-=(const Residue other) {
        x = (x - other.x + Mod) % Mod;
        return *this;
    }

    Residue operator*=(const Residue other) {
        x = (static_cast<long long>(x) * other.x) % Mod;
        return *this;
    }

    bool operator==(const Residue other) const {
        return x == other.x;
    }

    bool operator!=(const Residue other) const {
        return !(x == other.x);
    }

    Residue pow(unsigned k) const;

    Residue getInverse() const;

    Residue operator/=(const Residue other) {
        *this *= other.getInverse();
        return *this;
    }

    Residue order() const {
        int ph = phi(Mod);
        int ans = Mod;
        for (int i = 1; i * i <= ph; ++i) {
            if (ph % i == 0) {
                if (this->pow(i) == static_cast<Residue>(1)) {
                    return static_cast<Residue>(i);
                }
                if (this->pow(ph / i) == static_cast<Residue>(1)) {
                    ans = ph / i;
                }
            }
        }
        return static_cast<Residue>(ans);
    }

    Residue static getPrimitiveRoot();
};

template<unsigned Mod>
std::ostream& operator<<(std::ostream& out, const Residue<Mod>& x) {
    long long y = static_cast<int>(x);
    out << y;
    return out;
}

template<unsigned Mod>
std::istream& operator>>(std::istream& in, Residue<Mod>& x) {
    long long y;
    in >> y;
    x = static_cast<Residue<Mod>>(y);
    return in;
}

template<unsigned Mod>
Residue<Mod> operator+(const Residue<Mod> a, const Residue<Mod> b) {
    Residue<Mod> ans = a;
    ans += b;
    return ans;
}

template<unsigned Mod>
Residue<Mod> operator-(const Residue<Mod> a, const Residue<Mod> b) {
    Residue<Mod> ans = a;
    ans -= b;
    return ans;
}

template<unsigned Mod>
Residue<Mod> operator*(const Residue<Mod> a, const Residue<Mod> b) {
    Residue<Mod> ans = a;
    ans *= b;
    return ans;
}

template<unsigned Mod>
Residue<Mod> operator/(const Residue<Mod> a, const Residue<Mod> b) {
    Residue<Mod> ans = a;
    ans /= b;
    return ans;
}

template<unsigned Mod>
Residue<Mod> Residue<Mod>::pow(unsigned k) const {
    if (k == 0) {
        return Residue(1);
    }
    if ((k & 1)) {
        return Residue(x) * pow(k - 1);
    }
    Residue xx = pow(k / 2);
    return xx * xx;
}

template<unsigned Mod>
Residue<Mod> Residue<Mod>::getInverse() const {
    if (makeCompileErrorIfFalse<(is_prime_v<Mod>)>::value) {
        // everything is ok
    }
    return this->pow(Mod - 2);
}

template<unsigned Mod>
Residue<Mod> Residue<Mod>::getPrimitiveRoot() {
    if (makeCompileErrorIfFalse<(has_primitive_root_v<Mod>)>::value) {
        // everything is ok
    }
    int phim = phi(Mod);
    for (int i = 1;; ++i) {
        if (static_cast<Residue>(i).pow(phim) == static_cast<Residue>(1)) {
            bool f = true;
            for (int j = 1; j < phim; ++j) {
                if (static_cast<Residue>(i).pow(j) == static_cast<Residue>(1)) {
                    f = false;
                    break;
                }
            }
            if (f) {
                return static_cast<Residue>(i);
            }
        }
    }
}

size_t upperPowerOfTwo(size_t a) {
    size_t ans = 1;
    while (ans < a) {
        ans <<= 1;
    }
    return ans;
}

template<unsigned N, unsigned M, typename Field = Rational>
class Matrix {
  private:
    Field mat[N][M];

  public:
    Matrix() {
        for (size_t i = 0; i < N; ++i) {
            for (size_t j = 0; j < M; ++j) {
                mat[i][j] = static_cast<Field>(0);
            }
        }
    }

    Matrix(const Matrix& other) {
        for (size_t i = 0; i < N; ++i) {
            for (size_t j = 0; j < M; ++j) {
                mat[i][j] = other.mat[i][j];
            }
        }
    }

    Matrix& operator=(const Matrix& other) {
        if (&other == this) {
            return *this;
        }
        Matrix copyOfOther = other;
        std::swap(mat, copyOfOther.mat);
        return *this;
    }

    explicit Matrix(const std::vector<std::vector<Field>>& A) {
        for (size_t i = 0; i < N; ++i) {
            for (size_t j = 0; j < M; ++j) {
                mat[i][j] = A[i][j];
            }
        }
    }

    Matrix(const std::vector<std::vector<int>>& A) {
        for (size_t i = 0; i < N; ++i) {
            for (size_t j = 0; j < M; ++j) {
                mat[i][j] = static_cast<Field>(A[i][j]);
            }
        }
    }


    Matrix(const std::initializer_list<std::vector<int>>& A) {
        size_t i = 0;
        for (auto id : A) {
            for (size_t j = 0; j < M; ++j) {
                mat[i][j] = static_cast<Field>(id[j]);
            }
            ++i;
        }
    }

    Matrix(const std::initializer_list<double>& A) {
        size_t i = 0;
        size_t j = 0;
        for (auto id : A) {
            mat[i][j] = static_cast<Field>(id);
            ++j;
            if (j == M) {
                ++i;
                j = 0;
            }
        }
    }

    bool operator==(const Matrix& other) const {
        for (size_t i = 0; i < N; ++i) {
            for (size_t j = 0; j < M; ++j) {
                if (!(mat[i][j] == other[i][j])) {
                    return false;
                }
            }
        }
        return true;
    }

    bool operator!=(const Matrix& other) const {
        return !(*this == other);
    }

    Field* operator[](size_t id) {
        return mat[id];
    }

    const Field* operator[](size_t id) const {
        return mat[id];
    }

    Matrix& operator+=(const Matrix& other) {
        for (size_t i = 0; i < N; ++i) {
            for (size_t j = 0; j < M; ++j) {
                mat[i][j] += other.mat[i][j];
            }
        }
        return *this;
    }

    Matrix& operator-=(const Matrix& other) {
        for (size_t i = 0; i < N; ++i) {
            for (size_t j = 0; j < M; ++j) {
                mat[i][j] -= other.mat[i][j];
            }
        }
        return *this;
    }

    Matrix& operator*=(const Field& del) {
        for (size_t i = 0; i < N; ++i) {
            for (size_t j = 0; j < M; ++j) {
                mat[i][j] *= del;
            }
        }
        return *this;
    }

    Matrix operator+(const Matrix& other) const {
        Matrix ans = *this;
        ans += other;
        return ans;
    }

    Matrix operator-(const Matrix& other) const {
        Matrix ans = *this;
        ans -= other;
        return ans;
    }

    Matrix operator*(const Field& del) const {
        Matrix ans = *this;
        ans *= del;
        return ans;
    }

    template<unsigned K>
    Matrix<N, K, Field> operator*(const Matrix<M, K, Field>& other) const {
        Matrix<N, K, Field> ans;
        if (std::max({N, M, K}) >= 64) {
            const size_t newN = std::max({N, M, K}) + std::max({N, M, K}) % 2;
            Matrix<newN, newN, Field> A;
            Matrix<newN, newN, Field> B;
            for (size_t i = 0; i < newN; ++i) {
                for (size_t j = 0; j < newN; ++j) {
                    A[i][j] = (i < N && j < M) ? mat[i][j] : static_cast<Field>(0);
                    B[i][j] = (i < M && j < K) ? other[i][j] : static_cast<Field>(0);
                }
            }
            const size_t halfNewN = newN / 2;
            Matrix<halfNewN, halfNewN, Field> A11;
            Matrix<halfNewN, halfNewN, Field> A12;
            Matrix<halfNewN, halfNewN, Field> A21;
            Matrix<halfNewN, halfNewN, Field> A22;
            Matrix<halfNewN, halfNewN, Field> B11;
            Matrix<halfNewN, halfNewN, Field> B12;
            Matrix<halfNewN, halfNewN, Field> B21;
            Matrix<halfNewN, halfNewN, Field> B22;
            for (size_t i = 0; i < halfNewN; ++i) {
                for (size_t j = 0; j < halfNewN; ++j) {
                    A11[i][j] = A[i][j];
                    A12[i][j] = A[i][j + halfNewN];
                    A21[i][j] = A[i + halfNewN][j];
                    A22[i][j] = A[i + halfNewN][j + halfNewN];

                    B11[i][j] = B[i][j];
                    B12[i][j] = B[i][j + halfNewN];
                    B21[i][j] = B[i + halfNewN][j];
                    B22[i][j] = B[i + halfNewN][j + halfNewN];
                }
            }

            Matrix<halfNewN, halfNewN, Field> P1 = (A11 + A22) * (B11 + B22);
            Matrix<halfNewN, halfNewN, Field> P2 = (A21 + A22) * B11;
            Matrix<halfNewN, halfNewN, Field> P3 = A11 * (B12 - B22);
            Matrix<halfNewN, halfNewN, Field> P4 = A22 * (B21 - B11);
            Matrix<halfNewN, halfNewN, Field> P5 = (A11 + A12) * B22;
            Matrix<halfNewN, halfNewN, Field> P6 = (A21 - A11) * (B11 + B12);
            Matrix<halfNewN, halfNewN, Field> P7 = (A12 - A22) * (B21 + B22);

            Matrix<halfNewN, halfNewN, Field> C11 = P1 + P4 - P5 + P7;
            Matrix<halfNewN, halfNewN, Field> C12 = P3 + P5;
            Matrix<halfNewN, halfNewN, Field> C21 = P2 + P4;
            Matrix<halfNewN, halfNewN, Field> C22 = P1 - P2 + P3 + P6;

            for (size_t i = 0; i < N; ++i) {
                for (size_t j = 0; j < K; ++j) {
                    if (i < halfNewN && j < halfNewN) {
                        ans[i][j] = C11[i][j];
                    } else if (i < halfNewN) {
                        ans[i][j] = C12[i][j - halfNewN];
                    } else if (j < halfNewN) {
                        ans[i][j] = C21[i - halfNewN][j];
                    } else {
                        ans[i][j] = C22[i - halfNewN][j - halfNewN];
                    }
                }
            }

            return ans;
        }
        for (size_t i = 0; i < N; ++i) {
            for (size_t j = 0; j < K; ++j) {
                for (size_t k = 0; k < M; ++k) {
                    ans[i][j] += mat[i][k] * other[k][j];
                }
            }
        }
        return ans;
    }

    std::vector<Field> getRow(unsigned id) const {
        std::vector<Field> ans(M);
        for (size_t j = 0; j < M; ++j) {
            ans[j] = mat[id][j];
        }
        return ans;
    }

    std::vector<Field> getColumn(unsigned id) const {
        std::vector<Field> ans(N);
        for (size_t i = 0; i < N; ++i) {
            ans[i] = mat[i][id];
        }
        return ans;
    }

    Matrix<N, N, Field> operator*=(const Matrix<N, N, Field>& other) {
        if (makeCompileErrorIfFalse<(N == M)>::value) {
            // everything is ok
        }
        Field ans[N][M];

        for (size_t i = 0; i < N; ++i) {
            for (size_t j = 0; j < N; ++j) {
                for (size_t k = 0; k < N; ++k) {
                    ans[i][j] += mat[i][k] * other[k][j];
                }
            }
        }
        std::swap(mat, ans);
        return *this;
    }

    Field trace() const {
        if (makeCompileErrorIfFalse<(N == M)>::value) {
            // everything is ok
        }
        Field ans = static_cast<Field>(0);
        for (size_t i = 0; i < N; ++i) {
            ans += mat[i][i];
        }
        return ans;
    }

    Matrix<M, N, Field> transposed() const {
        Matrix<M, N, Field> ans;
        for (size_t i = 0; i < M; ++i) {
            for (size_t j = 0; j < N; ++j) {
                ans[i][j] = mat[j][i];
            }
        }
        return ans;
    }

    std::pair<Matrix, int> diagonaledByGauss() const { // matrix and count of swaps of rows
        size_t a = 0;
        size_t b = 0;
        Matrix ans = *this;
        size_t cnt = 0;
        while (a < N && b < M) {
            for (size_t i = a + 1; i < N; ++i) {
                if (ans[i][b] == static_cast<Field>(0)) {
                    continue;
                }
                if (ans[a][b] == static_cast<Field>(0)) {
                    for (size_t k = b; k < M; ++k) {
                        std::swap(ans[i][k], ans[a][k]);
                    }
                    ++cnt;
                    continue;
                }
                Field cur = static_cast<Field>(-1) * ans[i][b] / ans[a][b];
                for (size_t k = b; k < M; ++k) {
                    ans[i][k] += cur * ans[a][k];
                }
            }
            if (!(ans[a][b] == static_cast<Field>(0))) {
                ++a;
            }
            ++b;
        }
        return {ans, cnt};
    }

    int rank() const {
        auto d = this->diagonaledByGauss();

        int ans = 0;
        for (size_t i = 0; i < N; ++i) {
            ans += !(d.first[i][M - 1] == static_cast<Field>(0));
        }
        return ans;
    }

    Field det() const {
        if (makeCompileErrorIfFalse<(N == M)>::value) {
            // everything is ok
        }

        auto d = this->diagonaledByGauss();

        if (d.first[N - 1][N - 1] == static_cast<Field>(0)) {
            return static_cast<Field>(0);
        }
        Field ans = (d.second % 2) ? static_cast<Field>(-1) : static_cast<Field>(1);

        for (size_t i = 0; i < N; ++i) {
            ans *= d.first[i][i];
        }
        return ans;
    }

    void invert() {
        if (makeCompileErrorIfFalse<(N == M)>::value) {
            // everything is ok
        }

        Matrix<N, 2 * N, Field> cur;

        for (size_t i = 0; i < N; ++i) {
            for (size_t j = 0; j < N; ++j) {
                cur[i][j] = mat[i][j];
            }
            cur[i][i + N] = static_cast<Field>(1);
        }
        auto d = cur.diagonaledByGauss();
        cur = d.first;

        for (int j = N - 1; j >= 0; --j) {
            Field del = static_cast<Field>(1) / cur[j][j];
            for (size_t k = j; k < 2 * N; ++k) {
                cur[j][k] *= del;
            }
            for (int i = j - 1; i >= 0; --i) {
                Field del = static_cast<Field>(-1) * cur[i][j] / cur[j][j];
                for (size_t k = j; k < 2 * N; ++k) {
                    cur[i][k] += cur[j][k] * del;
                }
            }
        }

        for (size_t i = 0; i < N; ++i) {
            for (size_t j = 0; j < N; ++j) {
                mat[i][j] = cur[i][j + N];
            }
        }
    }

    Matrix inverted() const {
        if (makeCompileErrorIfFalse<(N == M)>::value) {
            // everything is ok
        }

        Matrix ans = *this;
        ans.invert();
        return ans;
    }
};

template<unsigned N, typename Field = Rational>
using SquareMatrix = Matrix<N, N, Field>;


template<unsigned N, unsigned M, typename Field>
Matrix<N, M, Field> operator*(const Field& del, const Matrix<N, M, Field>& A) {
    Matrix<N, M, Field> ans = A;
    ans *= del;
    return ans;
}

template<unsigned N, unsigned M, typename Field>
std::istream& operator>>(std::istream& in, Matrix<N, M, Field>& A) {
    for (size_t i = 0; i < N; ++i) {
        for (size_t j = 0; j < N; ++j) {
            in >> A[i][j];
        }
    }
    return in;
}

template<unsigned N, unsigned M, typename Field>
std::ostream& operator<<(std::ostream& out, Matrix<N, M, Field>& A) {
    for (size_t i = 0; i < N; ++i) {
        for (size_t j = 0; j < N; ++j) {
            out << A[i][j] << ' ';
        } out << '\n';
    }
    return out;
}
