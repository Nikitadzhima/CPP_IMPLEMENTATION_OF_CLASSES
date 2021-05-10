#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
#include <algorithm>
#include <cmath>

const double eps = 1e-4;
const double PI = acos(-1);

bool doublesAreEqual(const double a, const double b) {
    return fabs(b - a) < eps;
}

bool doublesAreNotEqual(const double a, const double b) {
    return !doublesAreEqual(a, b);
}

bool firstDoubleIsMore(const double a, const double b) {
    return a - b > eps;
}

bool firstDoubleIsMoreOrEqual(const double a, const double b) {
    return firstDoubleIsMore(a, b) || doublesAreEqual(a, b);
}

bool firstDoubleIsLess(const double a, const double b) {
    return !firstDoubleIsMoreOrEqual(a, b);
}

bool firstDoubleIsLessOrEqual(const double a, const double b) {
    return firstDoubleIsLess(a, b) || doublesAreEqual(a, b);
}

struct Point {
    // point (x, y)
  public:
    double x = 0;
    double y = 0;

    Point() = default;

    Point(double a, double b) : x(a), y(b) {}

    bool operator==(const Point& another) const {
        return doublesAreEqual(x, another.x) && doublesAreEqual(y, another.y);
    }

    bool operator!=(const Point& another) const {
        return !(*this == another);
    }
};

double Distance(const Point& a, const Point& b) {
    return sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y));
}

struct vector2D {
  public:
    double x = 0;
    double y = 0;

    vector2D() = default;

    vector2D(double a, double b): x(a), y(b) {}

    vector2D(Point p): x(p.x), y(p.y) {}

    vector2D(Point start, Point finish): x(finish.x - start.x), y(finish.y - start.y) {}

    double vec_mul(const vector2D& d) const {
        return x * d.y - y * d.x;
    }

    double scal_mul(const vector2D& d) const {
        return x * d.x + y * d.y;
    }

    vector2D operator+(const vector2D& d) const {
        return {x + d.x, y + d.y};
    }

    vector2D operator-(const vector2D& d) const {
        return {x - d.x, y - d.y};
    }

    vector2D operator*(double k) const {
        return {x * k, y * k};
    }

    vector2D operator/(double k) const {
        return {x / k, y / k};
    }

    vector2D rotated(double alpha) const {
        alpha *= PI / 180;
        return {x * cos(alpha) - y * sin(alpha), x * sin(alpha) + y * cos(alpha)};
    }

    double length() const {
        return Distance({x, y}, {0, 0});
    }
};

Point operator+(const Point& a, const vector2D& v) {
    return {a.x + v.x, a.y + v.y};
}

Point operator-(const Point& a, const vector2D& v) {
    return {a.x - v.x, a.y - v.y};
}

vector2D operator-(const Point& a, const Point& b) {
    return {a.x - b.x, a.y - b.y};
}

double angle(const vector2D& a, const vector2D& b) {
    if (doublesAreEqual(a.length(), 0) || doublesAreEqual(b.length(), 0)) return 0;
    double d = acos((a.scal_mul(b) / (a.length() * b.length())));
    if (firstDoubleIsLess(a.vec_mul(b), 0)) {
        d = -d;
    }
    return d;
}

class Line {
  private:
    // ax + by + c = 0
    double a = 0;
    double b = 1;
    double c = 0;

    void normalize() {
        if (doublesAreEqual(b, 0)) {
            c /= a;
            b = 0;
            a = 1;
        } else {
            a /= b;
            c /= b;
            b = 1;
        }
    }
  public:
    Line() = default;

    Line(Point d, Point e) { // Points d, e belongs to Line
        if (doublesAreEqual(d.x, e.x)) {
            a = 1;
            b = 0;
            c = -d.x;
        } else {
            a = (e.y - d.y) / (d.x - e.x);
            b = 1;
            c = -d.y - d.x * a;
        }
        normalize();
    }

    Line(double k, double b) : a(k), b(-1), c(b) {
        normalize();
    } // Line: y = kx + b

    Line(Point d, double k) : Line(k, d.y - k * d.x) {} // y = kx + b, Point d belongs to Line

    Line(Point d, vector2D v) : Line(d, d + v) {}

    vector2D normal() const {
        return {a, b};
    }

    bool operator==(const Line& another) const {
        return doublesAreEqual(a, another.a) && doublesAreEqual(b, another.b) && doublesAreEqual(c, another.c);
    }

    bool operator!=(const Line& another) const {
        return !(*this == another);
    }

    double geta() const {
        return a;
    }

    double getb() const {
        return b;
    }

    double getc() const {
        return c;
    }
};


Point intersection(const Line& n, const Line& m) {
    double a1 = n.geta();
    double b1 = n.getb();
    double c1 = n.getc();
    double a2 = m.geta();
    double b2 = m.getb();
    double c2 = m.getc();
    double x, y = (a1 * c2 - c1 * a2) / (b1 * a2 - a1 * b2);
    if (a1 != 0) {
        x = (-c1 - b1 * y) / a1;
    } else {
        x = (-c2 - b2 * y) / a2;
    }
    return {x, y};
}

class Shape {
  public:
    virtual double perimeter() const = 0;
    virtual double area() const = 0;

    virtual bool operator==(const Shape& another) const = 0;
    virtual bool operator!=(const Shape& another) const = 0;
    virtual bool isCongruentTo(const Shape& another) const = 0;
    virtual bool isSimilarTo(const Shape& another) const = 0;
    virtual bool containsPoint(Point point) const = 0;

    virtual void rotate(Point center, double angle) = 0;
    virtual void reflex(Point center) = 0;
    virtual void reflex(Line axis) = 0;
    virtual void scale(Point center, double coefficent) = 0;

    virtual ~Shape() {};
};

class Ellipse: public Shape {
  protected:
    Point f1 = {-1, 0};
    Point f2 = {1, 0};
    double r = 1.5;

  public:
    Ellipse() = default;

    Ellipse(Point a, Point b, double d): f1(a), f2(b), r(d / 2) {}

    double perimeter() const override {
        double a = r;
        double e = Distance(f1, f2) / (2 * a);
        double b = a * sqrt(1 - e * e);
        return PI * (3 * (a + b) - sqrt((3 * a + b) * (a + 3 * b)));
    }

    double area() const override {
        double a = r;
        double e = Distance(f1, f2) / (2 * a);
        double b = a * sqrt(1 - e * e);
        return PI * a * b;
    }

    bool operator==(const Shape& another) const override  {
        const Ellipse* panother = dynamic_cast<const Ellipse*>(&another);
        return panother ? doublesAreEqual(r, panother->r) && ((f1 == panother->f1 && f2 == panother->f2) || (f1 == panother->f2 && f2 == panother->f1)) : false;
    }

    bool operator!=(const Shape& another) const override {
        return !(*this == another);
    }

    bool isCongruentTo(const Shape& another) const override {
        const Ellipse* panother = dynamic_cast<const Ellipse*>(&another);
        return panother ? doublesAreEqual(Distance(f1, f2), Distance(panother->f1, panother->f2)) && doublesAreEqual(r, panother->r) : false;
    }

    bool isSimilarTo(const Shape& another) const override {
        const Ellipse* panother = dynamic_cast<const Ellipse*>(&another);
        if (!panother) {
            return false;
        }
        double a = 2 * r;
        double c = Distance(f1, f2) / 2;
        double b = a * sqrt(1 - (c * c) / (a * a));
        double a2 = 2 * panother->r;
        double c2 = Distance(panother->f1, panother->f2) / 2;
        double b2 = a2 * sqrt(1 - (c2 * c2 / (a2 * a2)));
        return doublesAreEqual(b / a, b2 / a2);
        return false;
    }

    bool containsPoint(Point point) const override {
        return firstDoubleIsLessOrEqual(Distance(point, f1) + Distance(point, f2), 2 * r);
    }

    void rotate(Point center, double angle) override {
        f1 = center + (f1 - center).rotated(angle);
        f2 = center + (f2 - center).rotated(angle);
    }

    void reflex(Point center) override {
        f1 = center - (f1 - center);
        f2 = center - (f2 - center);
    }

    void reflex(Line axis) override {
        Line n(f1, axis.normal());
        Point x = intersection(axis, n);
        f1 = f1 + (x - f1) * 2;
        n = {f2, axis.normal()};
        x = intersection(axis, n);
        f2 = f2 + (x - f2) * 2;
    }

    void scale(Point center, double coefficent) override {
        f1 = center + (f1 - center) * coefficent;
        f2 = center + (f2 - center) * coefficent;
        r *= coefficent;
    }

    std::pair <Point, Point> focuses() const {
        return {f1, f2};
    }

    std::pair <Line, Line> directrices() const {
        Line Ox(f1, f2);
        vector2D n = Ox.normal();
        double dist = Distance(f1, f2);
        Point x1 = f1 + (f2 - f1) / 2 + (f2 - f1) * r * r * 2 / (dist * dist);
        Point x2 = f1 + (f2 - f1) / 2 - (f2 - f1) * r * r * 2 / (dist * dist);
        Line a(x1, n);
        Line b(x2, n);
        return {a, b};
    }

    double eccentricity() const {
        return Distance(f1, f2) / (2 * r);
    }

    Point center() const {
        return f1 + (f2 - f1) / 2;
    }

    ~Ellipse() {}
};

class Circle: public Ellipse {
  public:
    using Ellipse::Ellipse;

    Circle(): Ellipse({0, 0}, {0, 0}, 2) {}

    Circle(Point center, double radius): Ellipse(center, center, 2 * radius) {}

    double radius() const {
        return r;
    }

    double perimeter() const override {
        return 2 * PI * r;
    }

    double area() const override {
        return PI * r * r;
    }
};

class Polygon: public Shape {
  protected:
    std::vector <Point> vertices;

    std::pair <bool, double> compare(const Polygon& other) const {
        if (vertices.size() != other.vertices.size()) {
            return {false, 0};
        }

        size_t n = vertices.size();
        std::vector <Point> cur = other.vertices;

        for (size_t i = 0; i < n; ++i) {
            for (size_t j = 0; j < n; ++j) {
                double alpha;
                double coef;
                bool ans = true;

                for (size_t k = 0; k < n; ++k) {
                    vector2D v1 = vertices[(i + 1 + k) % n] - vertices[(i + k) % n];
                    vector2D v2 = cur[(j + 1 + k) % n] - cur[(j + k) % n];
                    vector2D v3 = vertices[(i + 2 + k) % n] - vertices[(i + 1 + k) % n];
                    vector2D v4 = cur[(j + 2 + k) % n] - cur[(j + 1 + k) % n];

                    if (k == 0) {
                        coef = v1.length() / v2.length();
                        alpha = angle(v1, v3) / angle(v2, v4);
                        if (doublesAreNotEqual(alpha, 1) && doublesAreNotEqual(alpha, -1)) {
                            ans = false;
                            break;
                        }
                    } else if (doublesAreNotEqual(v1.length() / v2.length(), coef) || doublesAreNotEqual(alpha, angle(v1, v3) / angle(v2, v4))) {
                        ans = false;
                        break;
                    }
                }
                if (ans) {
                    return {true, coef};
                }
            }
        }
        return {false, 0};
    }
  public:
    Polygon() = default;

    Polygon(std::vector <Point> points) : vertices(points) {}

    //Polygon(std::initializer_list <Point> points) : vertices(points) {} we have to use it but why??

    double perimeter() const override {
        double ans = 0;
        size_t n = vertices.size();
        for (size_t i = 0; i + 1 < n; ++i) {
            ans += Distance(vertices[i], vertices[i + 1]);
        }
        ans += Distance(vertices.back(), vertices.front());
        return ans;
    }

    double area() const override {
        double ans = 0;
        size_t n = vertices.size();
        for (size_t i = 0; i + 1 < n; ++i) {
            ans += (vertices[i].y + vertices[i + 1].y) * (vertices[i + 1].x - vertices[i].x) / 2;
        }
        ans += (vertices.back().y + vertices.front().y) * (vertices.front().x - vertices.back().x) / 2;
        return fabs(ans);
    }

    bool operator==(const Shape& another) const override {
        const Polygon* panother = dynamic_cast<const Polygon*>(&another);
        if (!panother) {
            return false;
        }
        if (vertices.size() != panother->verticesCount()) {
            return false;
        }
        std::vector <Point> cur = panother->vertices;
        bool similarPointsFounded = false;
        size_t n = cur.size();

        for (size_t i = 0; i < n; ++i) {
            for (size_t j = 0; j < n; ++j) {
                if (vertices[i] != cur[j]) {
                    continue;
                }
                similarPointsFounded = true;
                bool similar = true;
                if (vertices[(i + 1) % n] != cur[(j + 1) % n]) {
                    std::reverse(cur.begin(), cur.end());
                    j = n - 1 - j;
                }
                for (size_t k = 0; k < n; ++k) {
                    if (vertices[(i + k) % n] != cur[(j + k) % n]) {
                        similar = false;
                        break;
                    }
                }
                return similar;
            }
            if (similarPointsFounded) {
                return false;
            }
        }
        return false;
    }

    bool operator!=(const Shape& another) const override {
        return !(*this == another);
    }

    bool isCongruentTo(const Shape& another) const override {
        const Polygon* panother = dynamic_cast<const Polygon*>(&another);

        if (!panother) {
            return false;
        }
        if (vertices.size() != panother->vertices.size()) {
            return false;
        }

        Polygon cur = *panother;
        auto ans = this->compare(cur);
        if (ans.first && doublesAreEqual(ans.second, 1)) {
            return true;
        }

        reverse(cur.vertices.begin(), cur.vertices.end());
        ans = this->compare(cur);
        if (ans.first && doublesAreEqual(ans.second, 1)) {
            return true;
        }
        return false;
    }

    bool isSimilarTo(const Shape& another) const override {
        const Polygon* panother = dynamic_cast<const Polygon*>(&another);
        if (!panother) {
            return false;
        }
        if (vertices.size() != panother->vertices.size()) {
            return false;
        }

        Polygon cur = *panother;
        auto ans = this->compare(cur);
        if (ans.first) {
            return true;
        }

        reverse(cur.vertices.begin(), cur.vertices.end());
        ans = this->compare(cur);
        if (ans.first) {
            return true;
        }
        return false;
    }

    bool containsPoint(Point point) const override {
        size_t n = vertices.size();
        double sum = 0;
        for (size_t i = 0; i + 1 < n; ++i) {
            vector2D v1 = vertices[i] - point;
            vector2D v2 = vertices[i + 1] - point;
            sum += angle(v1, v2);
        }
        sum += angle(vertices.back() - point, vertices.front() - point);
        return doublesAreEqual(2 * PI, fabs(sum));
    }

    void rotate(Point center, double angle) override {
        for (auto& v : vertices) {
            v = center + (v - center).rotated(angle);
        }
    }

    void reflex(Point center) override {
        for (auto& v : vertices) {
            v = v + (center - v) * 2;
        }
    }

    void reflex(Line axis) override {
        for (auto& v : vertices) {
            Line n(v, axis.normal());
            Point x = intersection(axis, n);
            v = v + (x - v) * 2;
        }
    }

    void scale(Point center, double coefficent) override {
        for (auto& v : vertices) {
            v = center + (v - center) * coefficent;
        }
    }

    size_t verticesCount() const {
        return vertices.size();
    }

    std::vector <Point> getVertices() const {
        return vertices;
    }

    bool isConvex() const {
        bool moreThanZero = false;
        bool lessThanZero = false;

        vector2D a, b;
        size_t n = vertices.size();
        for (size_t i = 0; i + 2 < n; ++i) {
            a = {vertices[i], vertices[i + 1]};
            b = {vertices[i + 1], vertices[i + 2]};
            firstDoubleIsLess(a.vec_mul(b), 0) ? lessThanZero : moreThanZero = true;
        }

        a = {vertices[n - 2], vertices.back()};
        b = {vertices.back(), vertices.front()};
        if (firstDoubleIsLess(a.vec_mul(b), 0)) {
            lessThanZero = true;
        } else {
            moreThanZero = true;
        }

        a = {vertices.back(), vertices.front()};
        b = {vertices.front(), vertices[1]};
        if (firstDoubleIsLess(a.vec_mul(b), 0)) {
            lessThanZero = true;
        } else {
            moreThanZero = true;
        }
        return !(moreThanZero && lessThanZero);
    }

    ~Polygon() {}
};

class Rectangle: public Polygon {
  public:
    using Polygon::Polygon;

    Rectangle(): Polygon({{0, 0}, {0, 1}, {1, 1}, {1, 0}}) {}

    Rectangle(Point a, Point b, double tgx) {
        tgx = std::max(tgx, 1 / tgx);
        Point c = a + (b - a).rotated(atan(tgx)) * cos(atan(tgx));
        Point d = a + (b - c);
        *this = Rectangle({a, c, b, d});
    }

    Point center() const {
        Line a(vertices[0], vertices[2]);
        Line b(vertices[1], vertices[3]);
        return intersection(a, b);
    }

    std::pair <Line, Line> diagonals() const {
        Line a(vertices[0], vertices[2]);
        Line b(vertices[1], vertices[3]);
        return {a, b};
    }

    ~Rectangle() {}
};

class Square: public Rectangle {
  public:
    using Rectangle::Rectangle;

    Square() : Rectangle({0, 0}, {1, 1}, 1) {}

    Square(Point a, Point b): Rectangle(a, b, 1) {}

    Circle circumscribedCircle() const {
        Point center = this->center();
        double radius = Distance(vertices[0], vertices[2]) / 2;
        return {center, center, radius};
    }

    Circle inscribedCircle() const {
        Point center = this->center();
        double radius = Distance(vertices[0], vertices[1]) / 2;
        return {center, center, radius};
    }
};

class Triangle: public Polygon {
  public:
    using Polygon::Polygon;

    Triangle(): Polygon({{0, 0}, {0, 1}, {1, 0}}) {}

    Triangle(Point a, Point b, Point c): Polygon({a, b, c}) {}

    Circle circumscribedCircle() const {
        Point a = vertices[0];
        Point b = vertices[1];
        Point c = vertices[2];
        double radius = Distance(a, b) * Distance(b, c) * Distance(a, c) / 4 / this->area();

        Line ab(a, b);
        vector2D abn = ab.normal();
        Line h1(a + (b - a) / 2, abn);

        Line bc(b, c);
        vector2D bcn = bc.normal();
        Line h2(b + (c - b) / 2, bcn);

        return {intersection(h1, h2), radius};
    }

    Circle inscribedCircle() const {
        Point a = vertices[0];
        Point b = vertices[1];
        Point c = vertices[2];
        double radius = 2 * this->area() / this->perimeter();

        Line ab(a, b);
        Line ac(a, c);
        Point bb = a + (b - a) / Distance(a, b);
        Point cc = a + (c - a) / Distance(a, c);
        Line h1(a, bb + (cc - bb) / 2);

        Line bc(b, c);
        Point aa = b + (a - b) / Distance(a, b);
        cc = b + (c - b) / Distance(c, b);
        Line h2(b, aa + (cc - aa) / 2);

        return {intersection(h1, h2), radius};
    }

    Point centroid() const {
        Point a = vertices[0];
        Point b = vertices[1];
        Point c = vertices[2];
        Point m = a + (b - a) + (c - b) / 2;
        return a + (m - a) * 2 / 3;
    }

    Point orthocenter() const {
        Point a = vertices[0];
        Point b = vertices[1];
        Point c = vertices[2];
        Line bc(b, c);
        Line ah(a, bc.normal());
        Line ac(a, c);
        Line bh(b, ac.normal());
        return intersection(ah, bh);
    }

    Line EulerLine() const {
        return Line(this->centroid(), this->orthocenter());
    }

    Circle ninePointsCircle() const {
        double radius = this->circumscribedCircle().radius() / 2;
        Point x1 = this->orthocenter();
        Point x2 = this->circumscribedCircle().center();
        Point center = x1 + (x2 - x1) / 2;
        return {center, radius};
    }
};


