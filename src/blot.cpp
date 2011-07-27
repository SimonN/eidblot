#include <vector>

#include "blot.h"



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
    
    pillar_dampening_of_120(30),
    pillar_strength       (0),
    
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
     for (int y = 0; x < texture->h; ++y) {
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
    if (main_mode == BEVEL) return process_shape_bevel (piece);
    else                    return process_shape_pillar(piece);
}
