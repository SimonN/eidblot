#include <vector>

#include "blot.h"
#include <cstdlib>
#include <cmath>


int Blotter::pink(0);



void Blotter::initialize()
{
    pink = makecol(0xFF, 0, 0xFF);
    srand(time(0));
}



Blotter::Blotter()
:
    main_mode(BEVEL),

    bevel_thickness(4),
    bevel_dampening(1),
    bevel_strength (0),

    pillar_dampening_of_120(60),
    pillar_strength        (0),

    texture  (0)
{
}



Blotter::~Blotter()
{
    if (texture) ::destroy_bitmap(texture);
    destroy_all_shapes();
}



void Blotter::destroy_all_shapes()
{
    for (Bitlist::iterator itr = shapes.begin(); itr != shapes.end(); ++itr)
        if (*itr != 0)
            ::destroy_bitmap(*itr);
    shapes.clear();
}



void Blotter::set_main_mode(MainMode m)  { main_mode = m; }

void Blotter::set_bevel_thickness(int i) { if (i >= 0) bevel_thickness = i; }
void Blotter::set_bevel_dampening(int i) { if (i >= 0) bevel_dampening = i; }
void Blotter::set_bevel_strength (int i) { if (i >= 0) bevel_strength  = i; }

void Blotter::set_pillar_dampening(int i)
{
    if (i >= 0) pillar_dampening_of_120 = i;
    if (pillar_dampening_of_120 > 120) pillar_dampening_of_120 = 120;
}

void Blotter::set_pillar_strength (int i) { if (i >= 0) pillar_strength  = i; }



bool Blotter::make_granulate(
    const int base_r,
    const int base_g,
    const int base_b,
    const int granularity
) {
    if (granularity < 0) return false;

    if (texture) ::destroy_bitmap(texture);
    texture = create_bitmap(640, 640);
    if (!texture) return false;

    for  (int x = 0; x < texture->w; ++x)
     for (int y = 0; y < texture->h; ++y) {
        const int rd = 2 * granularity + 1;
        int rr = base_r + granularity - (::rand() % rd);
        int rg = base_g + granularity - (::rand() % rd);
        int rb = base_b + granularity - (::rand() % rd);
        if (rr > 255) rr = 255; else if (rr < 0) rr = 0;
        if (rg > 255) rg = 255; else if (rg < 0) rg = 0;
        if (rb > 255) rb = 255; else if (rb < 0) rb = 0;
        ::_putpixel32(texture, x, y, makecol32(rr, rg, rb));
    }
    return true;
}


std::vector<std::vector<std::pair<double,double> > > Blotter::make_voronoi_table(
    const int amount,
    const pos size,
    const int thinness,
    const double norm
) {
    std::vector<std::vector<std::pair<double,double> > > matrix(
        size.x, std::vector<std::pair<double,double> >(
            size.y, std::pair<double,double>(0, 0)));

    std::vector<pos> dots;

    /* generate dot positions with minimum distance between them
     * dots are duplicated in all 8 directions, so it tesselates */
    srand(22934);
    double mindist = 2*std::sqrt(texture->w * texture->h / (16 * amount));
    for (int i = 0; i < amount; ++i) {
      retry:
        int base_x = rand() % size.x;
        int base_y = rand() % size.y;
        pos p(base_x, base_y);
        for (int j = 0; j < 9*i; ++j)
            if (dist(p, dots[j], norm) < mindist) goto retry;
        for (int x = -texture->w; x <= texture->w ; x += texture->w)
            for (int y = -texture->h; y <= texture->h ; y += texture->h)
                dots.push_back(pos(base_x + x, base_y + y));
    }

    // calculate the matrix
    for (int x = 0; x < texture->w; ++x)
        for (int y = 0; y < texture->h; ++y) {
            double mindist = 999999, mindist2 = 999999, mindist3 = 999999;
            double minangle, minangle2, minangle3;
            int minindex, minindex2, minindex3;
            //determine closes point
            for (int i = 0; i < 9*amount; ++i) {
                double d = dist(pos(x,y), dots[i], norm);
                if (d < mindist) {
                    minindex = i;
                    mindist = d;
                    minangle = angle(pos(x,y), dots[i]);
                }
            }
            pos q = dots[minindex];
            // determine closes remaining point that doesn't lie behind
            // first one when seen from (x,y)
            for (int i = 0; i < 9*amount; ++i) {
                if (dots[i].x == dots[minindex].x
                        && dots[i].y == dots[minindex].y) continue;
                double d = dist(pos(x,y), dots[i], norm);
                if (d < mindist2) {
                    double angl = angle(dots[minindex], dots[i]);
                    double da = std::fabs(angl - minangle);
                    if (da > PI/2 && da < 3*PI/2) { // far apart
                        mindist2 = d;
                        minangle2 = angl;
                        minindex2 = i;
                    }
                }
            }
            pos p = dots[minindex2];
            // determine closes remaining point that lies one the same side as
            // (x,y) relative to the line determined by the previous 2 dots
            for (int i = 0; i < 9*amount; ++i) {
                if (dots[i].x == dots[minindex].x
                        && dots[i].y == dots[minindex].y) continue;
                if (dots[i].x == dots[minindex2].x
                        && dots[i].y == dots[minindex2].y) continue;
                double d = dist(pos(x,y), dots[i], norm);
                if (d < mindist3) {
                    if (area(p, q, dots[i]) * area(p, q, pos(x,y)) < 0)
                        continue;
                    mindist3 = d;
                    minangle3 = angle(dots[minindex], dots[i]);
                    minindex3 = i;
                }
            }
            pos r = dots[minindex3];
            pos v = p - q;
            pos b(x, y);
            double coef = calc_voronoi_lambda(q, p-q, b, thinness);
            double angl = voronoi_antialiased_angle(p, q, b);
            double coef2 = std::max(coef, calc_voronoi_lambda(r, q-r, b, thinness));
            if (coef2 > coef) {
                coef = coef2;
                angl = voronoi_antialiased_angle(r, q, b);
            }
            matrix[x][y].first  = std::max(0.0, coef);
            matrix[x][y].second = angl;
        }

    return matrix;
}

double Blotter::voronoi_antialiased_angle(pos p, pos q, pos b) {
    pos v = p-q;
    //when orthigonally projected onto the line with start position q
    // and direction v, lambda is the coefficient so that the projection
    // of b is q + lambda*v
    double lambda = calc_ortho_lambda(q, v, b);
    double d = (1-2*lambda) * v.norm() + 0.5;

    //double d = dist(b, p) - dist(b, q);
    double angl = angle(p, q);
    if (d >= 1.0)
        return angl;
    double negangl = angle(q, p);
    /*
    // move angle into interval [-PI/4, 3*PI/4] by mirroring along the
    // y = -x axis if it is outside that interval (math coords)
    if (angl < -PI/4)
        angl = -PI/2 - angl;
    else if (angl > 3*PI/4)
        angl = -3*PI/2 - angl;
    if (negangl < -PI/4)
        negangl = -PI/2 - negangl;
    else if (negangl > 3*PI/4)
        negangl = -3*PI/2 - negangl;
    */
    // move angle into interval [-3*PI/4, PI/4] by mirroring along the
    // y = -x axis if it is outside that interval (screen coords)
    if (angl > PI/4)
        angl = PI/2 - angl;
    else if (angl < -3*PI/4)
        angl = -3*PI/2 - angl;
    if (negangl > PI/4)
        negangl = PI/2 - negangl;
    else if (negangl < -3*PI/4)
        negangl = -3*PI/2 - negangl;
    // interpolate between the two angles
    return d * angl + (1-d) * negangl;
}


double Blotter::calc_ortho_lambda(pos q, pos v, pos b) {
    //when orthigonally projected onto the line with start position q
    // and direction v, lambda is the coefficient so that the projection
    // of b is q + lambda*v
    double lambda = 0;
    if (v.y == 0) lambda = (q.x-b.x)/v.x;
    else lambda = -((q.x-b.x)*v.x/ (double)v.y + q.y-b.y)
        /(double)(v.x*v.x/(double)v.y+v.y);
    return lambda;
}

double Blotter::calc_voronoi_lambda(pos q, pos v, pos b, int thinness) {
    double lambda = calc_ortho_lambda(q, v, b);
    // turn it into a shading coefficient
    return 1-thinness*std::abs(2*lambda-1);
}


bool Blotter::make_voronoi(
    const color cl,
    const color cd,
    const int amount,
    const pos size,
    const int thinness,
    const double norm
) {
    if (norm <= 0) return false;
    if (amount <= 2) return false;

    if (texture) ::destroy_bitmap(texture);
    texture = create_bitmap(size.x, size.y);
    if (!texture) return false;

    std::vector<std::vector<std::pair<double,double> > > matrix
        = make_voronoi_table(amount, size, thinness, norm);

    for (int x = 0; x < texture->w; ++x)
        for (int y = 0; y < texture->h; ++y) {
            color ci = interpolate(cl, cd, matrix[x][y].first);
            ::_putpixel32(texture, x, y, makecol32(ci));
        }
    return true;
}



bool Blotter::make_voronoi_shaded(
    const color cl,
    const color cd,
    const int amount,
    const pos size,
    const int thinness,
    const double norm
) {
    if (norm <= 0) return false;
    if (amount <= 2) return false;

    if (texture) ::destroy_bitmap(texture);
    texture = create_bitmap(size.x, size.y);
    if (!texture) return false;

    std::vector<std::vector<std::pair<double,double> > > matrix
        = make_voronoi_table(amount, size, thinness, norm);

    for (int x = 0; x < texture->w; ++x)
        for (int y = 0; y < texture->h; ++y) {
            color cm = interpolate(cl, cd, 0.5);
            double angl = matrix[x][y].second - PI/4;
            double shade = cos(angl)/2 + 0.5;
            color ci = interpolate(cl, cd, shade);
            ci = interpolate(ci, cm, matrix[x][y].first);
            ::_putpixel32(texture, x, y, makecol32(ci));
        }
    return true;
}

bool Blotter::load_texture(const std::string& s)
{
    if (texture) ::destroy_bitmap(texture);
    texture = ::load_bitmap(s.c_str(), 0);
    return texture != 0;
}



BITMAP* Blotter::get_texture()
{
    return texture;
}



bool Blotter::load_shapes(const std::string& s)
{
    destroy_all_shapes();

    BITMAP* sheet = load_bitmap(s.c_str(), 0);
    if (!sheet) return false;
    else {
        bool ok = cut_into_list(sheet);
        ::destroy_bitmap(sheet);
        return ok;
    }
}



int Blotter::get_shapes_remaining()
{
    return shapes.size();
}



BITMAP* Blotter::pop_shape_caller_should_destroy_it_later()
{
    if (shapes.empty()) return 0;
    BITMAP* first = *shapes.begin();
    shapes.erase(shapes.begin());
    return first;
}



bool Blotter::cut_into_list(BITMAP* shapebit)
{
    for  (int y = 0; y < shapebit->h; ++y)
     for (int x = 0; x < shapebit->w; ++x) {
        if (getpixel(shapebit, x, y) != pink) {
            Area area(shapebit);
            find_connected_area(area, x, y);

            BITMAP* piece = create_bitmap(area.x_max - area.x_min + 1,
                                          area.y_max - area.y_min + 1);
            if (!piece || piece->w <= 0 || piece->h <= 0) {
                if (piece) destroy_bitmap(piece);
                return false;
            }
            clear_to_color(piece, pink);
            for (std::set <Xy> ::const_iterator
             itr = area.Xy.begin(); itr != area.Xy.end(); ++itr) {
                _putpixel32(piece,
                    itr->first - area.x_min, itr->second - area.y_min,
                    _getpixel32(area.ground, itr->first, itr->second));
            }
            shapes.push_back(piece);
            fill_area_with_color(area, pink);
        }
    }
    return true;
}



void Blotter::find_connected_area(
    Blotter::Area& area,
    const int x,
    const int y
) {
    std::list <Xy> open; // Pixels that still have to be examined.
    open.push_back(Xy(x, y));

    const int xl = area.ground->w;
    const int yl = area.ground->h;

    while (!open.empty()) {
        Xy cur = *open.begin();
        open.erase(open.begin());

        if (getpixel(area.ground, cur.first, cur.second) != pink) {
            std::set <Xy> ::const_iterator found = area.Xy.find(cur);
            // If cur is not in the set area.Xy
            if (found == area.Xy.end()) {
                area.Xy.insert(cur);
                if (area.x_min > cur.first)  area.x_min = cur.first;
                if (area.x_max < cur.first)  area.x_max = cur.first;
                if (area.y_min > cur.second) area.y_min = cur.second;
                if (area.y_max < cur.second) area.y_max = cur.second;
                const bool u = cur.first  > 0;
                const bool d = cur.first  < xl - 1;
                const bool l = cur.second > 0;
                const bool r = cur.second < yl - 1;
                if (u     ) open.push_back(Xy(cur.first-1, cur.second  ));
                if (u && r) open.push_back(Xy(cur.first-1, cur.second+1));
                if (     r) open.push_back(Xy(cur.first,   cur.second+1));
                if (d && r) open.push_back(Xy(cur.first+1, cur.second+1));
                if (d     ) open.push_back(Xy(cur.first+1, cur.second  ));
                if (d && l) open.push_back(Xy(cur.first+1, cur.second-1));
                if (     l) open.push_back(Xy(cur.first,   cur.second-1));
                if (u && l) open.push_back(Xy(cur.first-1, cur.second-1));
            }
        }
        // Done checking a pixel.
    }
    // Done adding a complete connected area to Area& area.
}



void Blotter::fill_area_with_color(const Blotter::Area& area, const int color)
{
    for (std::set <Xy> ::const_iterator
     itr = area.Xy.begin(); itr != area.Xy.end(); ++itr) {
        putpixel(area.ground, itr->first, itr->second, color);
    }
}



bool Blotter::process_shape(BITMAP* piece)
{
    if      (main_mode == RAW)   return process_shape_raw   (piece);
    else if (main_mode == BEVEL) return process_shape_bevel (piece);
    else                         return process_shape_pillar(piece);
}
