#include <cmath>
#include <algorithm>

#include "color.h"

color::color(int r, int g, int b) : r(r), g(g), b(b) {};

color interpolate(const color c1, const color c2, double coef) {
    coef = std::max(coef, 0.0);
    coef = std::min(coef, 1.0);
    return color(
        (int) (c1.r * coef + c2.r * (1 - coef)),
        (int) (c1.g * coef + c2.g * (1 - coef)),
        (int) (c1.b * coef + c2.b * (1 - coef))
    );
}

int makecol32(color c) {
    return makecol32(c.r, c.g, c.b);
}

pos::pos(int x, int y) : x(x), y(y) {}

double pos::norm(double norm) {
    return std::pow(
        std::pow(std::abs(this->x), norm) + std::pow(std::abs(this->y), norm),
        1 / norm);

}

pos operator+(pos p1, pos p2) {
    return pos(p1.x + p2.x, p1.y + p2.y);
}

pos operator-(pos p1, pos p2) {
    return pos(p1.x - p2.x, p1.y - p2.y);
}

double dist(pos p1, pos p2, double norm) {
    pos d = p2 - p1;
    return d.norm(norm);
}

double angle(pos p1, pos p2) {
    pos d = p2 - p1;
    return atan2(d.y, d.x);
}

double area(pos p, pos q, pos r) {
    return (q.x-p.x)*(r.y-p.y) - (q.y-p.y)*(r.x-p.x);
}
