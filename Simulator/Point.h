#pragma once

class Point {
public:
    int x;
    int y;

    Point(int x, int y) : x(x), y(y) {}

    bool isEqual(Point other) {
        return this->x == other.x && this->y == other.y;
    }
};
