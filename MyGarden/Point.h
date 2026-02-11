#pragma once
#include <functional>
#include <memory>

struct Point {
    int x;
    int y;
    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }
    Point operator+(const Point& point) const {
        return Point{x + point.x, y + point.y};
    }
};

namespace std {
template <>
struct hash<Point> {
    size_t operator()(const Point& p) const {
        size_t h1 = hash<int>{}(p.x);
        size_t h2 = hash<int>{}(p.y);
        return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
    }
};
}  // namespace std