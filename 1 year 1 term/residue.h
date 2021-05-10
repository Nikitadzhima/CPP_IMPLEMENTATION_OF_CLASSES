#include <iostream>
#include <math.h>
#include <algorithm>
#include <vector>

namespace {

template<unsigned N, unsigned D>
struct is_prime_helper {
    static const bool value = N % D == 0 ? false :
        is_prime_helper<N, D - 1 - (D & 1)>::value;
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
    static const bool value = (N <= 1 || (N > 2 && !(N & 1))) ? false :
        is_prime_helper<N, static_cast<unsigned>(std::floor(sqrt(N) + 0.5))>::value;
};

template<unsigned N>
static const bool is_prime_v = is_prime<N>::value;


namespace {

template<unsigned X, unsigned N>
struct check_deg {
    static const bool value = N % X == 0 ? check_deg<X, N / X>::value : false;
};

template<unsigned X>
struct check_deg<X, X> {
    static const bool value = true;
};

template<unsigned N>
struct check_deg<1, N> {
    static const bool value = false;
};

template<unsigned N>
struct check_deg<0, N> {
    static const bool value = false;
};

template<unsigned X>
struct check_deg<X, 0> {
    static const bool value = false;
};

template<>
struct check_deg<1, 1> {
    static const bool value = false;
};

template<>
struct check_deg<0, 0> {
    static const bool value = false;
};

template<unsigned N, unsigned P>
struct find_min_del {
    static const unsigned value = find_min_del<N, P - 1 - (P & 1)>::value == 0 ?
        (N % P == 0 ? P : 0) : find_min_del<N, P - 1 - (P & 1)>::value;
};

template<unsigned N>
struct find_min_del<N, 1> {
    static const unsigned value = 0;
};

template<unsigned N>
struct find_min_del<N, 0> {
    static const unsigned value = 0;
};

template<unsigned N>
struct is_deg_of_simple {
    static const bool value = is_prime_v<N> ||
        check_deg<find_min_del<N, static_cast<unsigned>(std::floor(sqrt(N) + 0.5))>::value, N>::value;
};

template<>
struct is_deg_of_simple<2> {
    static const bool value = false;
};

template<>
struct is_deg_of_simple<1> {
    static const bool value = false;
};

template<>
struct is_deg_of_simple<0> {
    static const bool value = false;
};

} // namespace helpers

template<unsigned N>
struct has_primitive_root {
    static const bool value = (N % 4 == 0) ? false :
        (N % 4 == 2 ? is_deg_of_simple<N / 2>::value : is_deg_of_simple<N>::value);
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

namespace {

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

int gcd(int a, int b) {
    while (b) {
        a %= b;
        std::swap(a, b);
    }
    return a;
}
} // namespace details

template<unsigned Mod>
class Residue {
  private:
    long long x;

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

    Residue& operator+=(const Residue other) {
        x = (x + other.x) % Mod;
        return *this;
    }

    Residue& operator-=(const Residue other) {
        x = (x - other.x + Mod) % Mod;
        return *this;
    }

    Residue& operator*=(const Residue other) {
        x = (static_cast<long long>(x) * other.x) % Mod;
        return *this;
    }

    bool operator==(const Residue other) {
        return x == other.x;
    }

    Residue pow(unsigned p) const {
        if (p == 0) {
            return static_cast<Residue>(1);
        }
        if (p % 2 == 1) {
            return *this * pow(p - 1);
        }
        Residue c = pow(p / 2);
        return c * c;
    }

    Residue getInverse() const;

    Residue& operator/=(const Residue other) {
        return *this *= other.getInverse();
    }

    int order() const {
        long long ph = phi(Mod);
        long long ans = Mod;
        for (long long i = 1; i * i <= ph; ++i) {
            if (ph % i == 0) {
                if (this->pow(i) == static_cast<Residue>(1)) {
                    return i;
                }
                if (this->pow(ph / i) == static_cast<Residue>(1)) {
                    ans = ph / i;
                }
            }
        }
        return ans;
    }

    static Residue getPrimitiveRoot();
};

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


void crashIfFalse(bool flag) {
    int a[-1 + flag];
    std::reverse(a, a + 1);
};

template<unsigned Mod>
Residue<Mod> Residue<Mod>::getInverse() const {
    crashIfFalse(is_prime_v<Mod>);
    return this->pow(Mod - 2);
}


template<unsigned Mod>
Residue<Mod> Residue<Mod>::getPrimitiveRoot() {
    crashIfFalse(has_primitive_root_v<Mod>);
    int phim = phi(Mod);
    int phim2 = phim;
    std::vector<int> fac;
    for (int i = 2; i * i <= phim2; ++i) {
        if (phim2 % i == 0) {
            while (phim2 % i == 0) {
                phim2 /= i;
            }
            fac.push_back(i);
        }
    }
    if (phim2 > 1) {
        fac.push_back(phim2);
    }
    for (int i = 2;; ++i) {
        if (gcd(i, Mod) != 1) {
            continue;
        }
        if (!(static_cast<Residue>(i).pow(phim) == static_cast<Residue>(1))) {
            continue;
        }
        bool f = true;
        for (auto j : fac) {
            if ((static_cast<Residue>(i).pow(phim / j) == static_cast<Residue>(1))) {
                f = false;
                break;
            }
        }
        if (f) {
            return static_cast<Residue>(i);
        }
    }
}
