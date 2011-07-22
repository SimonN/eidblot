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
    thickness(4),
    dampening(1),
    strength (255),
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



void Blotter::set_thickness(int i) { if (i >= 0) thickness = i; }
void Blotter::set_dampening(int i) { if (i >= 0) dampening = i; }
void Blotter::set_strength (int i) { if (i >= 0) strength  = i; }



bool Blotter::load_texture(const std::string& s)
{
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



// Just for use in void apply_texture().
inline bool is_pink(BITMAP* b, const int x, const int y)
{
    if (x < 0 || y < 0 || x >= b->w || y >= b->h) return true;
    else return _getpixel32(b, x, y) == Blotter::pink;
}

inline int recolor_base_color(
    const int base,
    const int light,
    const int thick
) {
    if   (light > 0) return base + (255 - base) * light / thick;
    else             return base +     base     * light / thick;
}



bool Blotter::process_shape(BITMAP* piece)
{
    if (piece->w > texture->w || piece->h > texture->h) {
        return false;
    }
    // The following vector should be viewed as functions from the set of
    // piece coordinates to ints. The value of the coordinate (x, y) is
    // stored at [y * piece->w + x].
    std::vector <int> edge(piece->w * piece->h, 0);

    // Pixels that are closer to the top/left edges should be lighter, colors
    // close to bottom/right edges should be darker in the final output.
    // std::vector <int> edge stores 0 for each pink pixel, 0 for each pixel
    // that is far away from pink, a positive value for pixels that should
    // be made lighter, and a negative value for pixels that should be darker.

    // The maximum distance from a pink pixel to still have an effect on this
    // recoloring is stored in the following variable.
    // thickness;

    // The greater this value is in comparison to thickness, the less the
    // edges will be colored towards white/dark. See int recolor_base_color()
    // above for details. This value must be greater than thickness in order
    // to hand int makecol32() values between 0 and 255 inclusive!
    // Don't set it to 0 in case thickness is 0, or the code will divide by 0.
    const int extra_thickness = thickness + dampening;

    // Find extreme outside points which would get colored badly otherwise
    for  (int y = 0; y < piece->h; ++y)
     for (int x = 0; x < piece->w; ++x) {
        const int eXy = piece->w * y + x;
        if (getpixel(piece, x, y) == pink) continue;
        else {
            bool lighter = is_pink(piece, x-1, y-1);
            bool darker  = is_pink(piece, x+1, y+1);
            if (lighter && !darker) edge[eXy] = thickness;
            if (!lighter && darker) edge[eXy] = -thickness;
        }
    }

    // Finding the outsides.
    for  (int y = 0; y < piece->h; ++y)
     for (int x = 0; x < piece->w; ++x) {
        const int eXy = piece->w * y + x;
        if (getpixel(piece, x, y) == pink) continue;
        if (edge[eXy] != 0)                continue;
        else {
            bool lighter = is_pink(piece, x-1, y  )
                        || is_pink(piece, x  , y-1)
                        || is_pink(piece, x-1, y-1);
            bool darker  = is_pink(piece, x+1, y  )
                        || is_pink(piece, x  , y+1)
                        || is_pink(piece, x+1, y+1);
            if (lighter && !darker) edge[eXy] = thickness;
            if (!lighter && darker) edge[eXy] = -thickness;
        }
    }

    // Finding outside pixels that weren't colored yet,
    // e.g. on tau/8-slopes
    for  (int y = 0; y < piece->h; ++y)
     for (int x = 0; x < piece->w; ++x) {
        const int eXy = piece->w * y + x;
        if (getpixel(piece, x, y) == pink) continue;
        if (edge[eXy] != 0)                continue;
        else {
            bool lighter = is_pink(piece, x,   y-1)
                      && ! is_pink(piece, x-1, y-1)
                      && ! is_pink(piece, x+1, y+1);
            bool darker  = is_pink(piece, x,   y+1)
                      && ! is_pink(piece, x-1, y-1)
                      && ! is_pink(piece, x+1, y+1);
            if (lighter && !darker) edge[eXy] =   (thickness / 2);
            if (!lighter && darker) edge[eXy] = - (thickness / 2);
        }
    }

    // Finding inside pixels. It's enough to just handle the proper inside.
    for (int step = thickness - 1; step > 0; --step) {
        for  (int y = 1; y < piece->h - 1; ++y)
         for (int x = 1; x < piece->w - 1; ++x) {
            const int eXy = piece->w * y + x;
            if (edge[eXy] != 0 || is_pink(piece, x, y)) continue;
            else {
                // We're in the proper inside. +-1 is always valid
                // in each direction and yields an existing pixel again.
                bool lighter = edge[eXy - 1]            == step + 1
                            || edge[eXy - piece->w]     == step + 1
                            || edge[eXy - piece->w - 1] == step + 1;
                bool darker  = edge[eXy + 1]            == -step - 1
                            || edge[eXy + piece->w]     == -step - 1
                            || edge[eXy + piece->w + 1] == -step - 1;
                if (lighter && !darker) edge[eXy] = step;
                if (!lighter && darker) edge[eXy] = -step;
            }
        }
    }

    // Apply the texture
    const int off_x = rand() % (texture->w - piece->w + 1);
    const int off_y = rand() % (texture->h - piece->h + 1);
    for  (int y = 0; y < piece->h; ++y)
     for (int x = 0; x < piece->w; ++x) {
        if (_getpixel32(piece, x, y) == pink) continue;
        else {
            _putpixel32(piece, x, y,
             _getpixel32(texture, off_x + x, off_y + y));
        }
    }

    // Recolor the piece
    for  (int y = 0; y < piece->h; ++y)
     for (int x = 0; x < piece->w; ++x) {
        const int eXy = piece->w * y + x;
        const int p = _getpixel32(piece, x, y);
        if (p == pink) continue;
        else if (edge[eXy] == 0) continue;
        else {
            _putpixel32(piece, x, y, makecol32(
             recolor_base_color(getr32(p), edge[eXy], extra_thickness),
             recolor_base_color(getg32(p), edge[eXy], extra_thickness),
             recolor_base_color(getb32(p), edge[eXy], extra_thickness)));
        }
    }
    return true;
}
