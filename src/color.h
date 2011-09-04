#include <allegro.h>

#define PI 3.141592653569793

struct color {
    int r, g, b;

    color(int, int, int);
};

color interpolate(color, color, double);
int makecol32(color);

const color black(0, 0, 0);
const color white(0xFF, 0xFF, 0xFF);

struct pos {
    int x, y;

    pos(int x, int y);
    double norm(double = 2.0);
};

pos operator+(pos p1, pos p2);
pos operator-(pos p1, pos p2);

double dist(pos, pos, double = 2.0);
double angle(pos, pos);
double area(pos, pos, pos);

