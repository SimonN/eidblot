/*
 * eidblot
 *
 * Eiderdaus's Lix terrain making program. See spec.txt for info.
 *
 */

#include <allegro.h>
#include <iostream>
#include <set>
#include <list>
#include <vector>
#include <sstream>

typedef std::pair <int, int> XY;

struct Area {
    BITMAP* ground;
    std::set <XY> xy;
    int x_min;
    int x_max;
    int y_min;
    int y_max;

    Area(BITMAP* g) :
     ground(g), x_min(ground->w - 1), y_min(ground->h - 1), x_max(0), y_max(0)
     {}
};

int pink = 0; // global

// Forward declarations
void cut_into_list       (BITMAP*, std::list <BITMAP*>&);
void find_connected_area (      Area&, const int,   const int);
void fill_area_with_color(const Area&, const int);
void apply_texture       (BITMAP*, BITMAP*);



void output_progress_dot(const int n)
{
    if      (n == 0) ; // don't do anything
    else if (n % 200 == 0) std::cout << "\n\n";
    else if (n %  40 == 0) std::cout << "\n";
    else if (n %   5 == 0) std::cout << " ";
    std::cout << "." << std::flush;
}



int main(int argc, char* argv[])
{
    allegro_init();
    set_color_depth(32);

    srand(time(0));

    std::cout << "This is eidblot." << std::endl;

    pink = makecol(0xFF, 0, 0xFF);

    if (argc != 3) {
        std::cout << "Usage: " << argv[0] << " shapes.bmp texture.bmp"
                  << std::endl;
        return -1;
    }

    BITMAP* shapebit = load_bitmap(argv[1], 0);
    BITMAP* texture  = load_bitmap(argv[2], 0);
    if (!texture || !shapebit) {
        if (!texture ) std::cout << "Error loading ``" << argv[2] << "''.\n";
        if (!shapebit) std::cout << "Error loading ``" << argv[1] << "''.\n";
        std::cout << "Aborting." << std::endl;
        if (texture)  destroy_bitmap(texture);
        if (shapebit) destroy_bitmap(shapebit);
        return -1;
    }

    std::list <BITMAP*> shapes;
    std::cout << "Searching for shapes in ``"
              << argv[1] << "'':" << std::endl;
    cut_into_list(shapebit, shapes);
    std::cout << "Found " << shapes.size() << " shapes."
              << " Texturing them with ``" << argv[2] << "'':" << std::endl;

    int outfile_count = 0;
    for (std::list <BITMAP*> ::iterator itr = shapes.begin();
     itr != shapes.end(); ++itr, ++outfile_count) {
        apply_texture(*itr, texture);

        std::ostringstream filename;
        if (outfile_count < 100) filename << "0";
        if (outfile_count <  10) filename << "0";
        filename << outfile_count << ".pcx";
        save_pcx(filename.str().c_str(), *itr, 0);

        output_progress_dot(outfile_count);

        destroy_bitmap(*itr);
    }
    std::cout << std::endl;

    std::cout << "Done." << std::endl;
    return 0;
}
END_OF_MAIN()



void cut_into_list(BITMAP* shapebit, std::list <BITMAP*>& list)
{
    for  (int y = 0; y < shapebit->h; ++y)
     for (int x = 0; x < shapebit->w; ++x) {
        if (getpixel(shapebit, x, y) != pink) {
            Area area(shapebit);
            find_connected_area(area, x, y);

            BITMAP* piece = create_bitmap(area.x_max - area.x_min + 1,
                                          area.y_max - area.y_min + 1);
            if (!piece || piece->w <= 0 || piece->h <= 0) {
                std::cout << "Error making a piece, continuing." << std::endl;
                if (piece) destroy_bitmap(piece);
                continue;
            }
            clear_to_color(piece, pink);
            for (std::set <XY> ::const_iterator
             itr = area.xy.begin(); itr != area.xy.end(); ++itr) {
                putpixel(piece,
                         itr->first - area.x_min, itr->second - area.y_min,
                         getpixel(area.ground, itr->first, itr->second));
            }
            list.push_back(piece);
            fill_area_with_color(area, pink);
            
            output_progress_dot(list.size() - 1);
        }
    }
    std::cout << std::endl;
}



void find_connected_area(Area& area, const int x, const int y)
{
    std::list <XY> open; // Pixels that still have to be examined.
    open.push_back(XY(x, y));

    const int xl = area.ground->w;
    const int yl = area.ground->h;

    while (!open.empty()) {
        XY cur = *open.begin();
        open.erase(open.begin());

        if (getpixel(area.ground, cur.first, cur.second) != pink) {
            bool already_in_area = false;
            std::set <XY> ::const_iterator found = area.xy.find(cur);
            // If cur is not in the set area.xy
            if (found == area.xy.end()) {
                area.xy.insert(cur);
                if (area.x_min > cur.first)  area.x_min = cur.first;
                if (area.x_max < cur.first)  area.x_max = cur.first;
                if (area.y_min > cur.second) area.y_min = cur.second;
                if (area.y_max < cur.second) area.y_max = cur.second;
                const bool u = cur.first  > 0;
                const bool d = cur.first  < xl - 1;
                const bool l = cur.second > 0;
                const bool r = cur.second < yl - 1;
                if (u     ) open.push_back(XY(cur.first-1, cur.second  ));
                if (u && r) open.push_back(XY(cur.first-1, cur.second+1));
                if (     r) open.push_back(XY(cur.first,   cur.second+1));
                if (d && r) open.push_back(XY(cur.first+1, cur.second+1));
                if (d     ) open.push_back(XY(cur.first+1, cur.second  ));
                if (d && l) open.push_back(XY(cur.first+1, cur.second-1));
                if (     l) open.push_back(XY(cur.first,   cur.second-1));
                if (u && l) open.push_back(XY(cur.first-1, cur.second-1));
            }
        }
        // Done checking a pixel.
    }
    // Done adding a complete connected area to Area& area.
}



void fill_area_with_color(const Area& area, const int color)
{
    for (std::set <XY> ::const_iterator
     itr = area.xy.begin(); itr != area.xy.end(); ++itr) {
        putpixel(area.ground, itr->first, itr->second, color);
    }
}



// Just for use in void apply_texture().
inline bool is_pink(BITMAP* b, const int x, const int y)
{
    if (x < 0 || y < 0 || x >= b->w || y >= b->h) return true;
    else return _getpixel32(b, x, y) == pink;
}

inline int recolor_base_color(
    const int base,
    const int light,
    const int thick
) {
    if   (light > 0) return base + (255 - base) * light / thick;
    else             return base +     base     * light / thick;
}



void apply_texture(BITMAP* piece, BITMAP* texture)
{
    if (piece->w > texture->w || piece->h > texture->h) {
        std::cout << "Texture too small for piece. Check your output later."
         << std::endl;
        return;
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
    const int thickness = 4;

    // The greater this value is in comparison to thickness, the less the
    // edges will be colored towards white/dark. See int recolor_base_color()
    // above for details. This value must be greater than thickness in order
    // to hand int makecol32() values between 0 and 255 inclusive!
    // Don't set it to 0 in case thickness is 0, or the code will divide by 0.
    const int extra_thickness = thickness + 2;

    // Finding the outsides.
    for  (int y = 0; y < piece->h; ++y)
     for (int x = 0; x < piece->w; ++x) {
        const int exy = piece->w * y + x;
        if (getpixel(piece, x, y) == pink) continue;
        else {
            bool lighter = is_pink(piece, x-1, y  )
                        || is_pink(piece, x  , y-1)
                        || is_pink(piece, x-1, y-1);
            bool darker  = is_pink(piece, x+1, y  )
                        || is_pink(piece, x  , y+1)
                        || is_pink(piece, x+1, y+1);
            if (lighter && !darker) edge[exy] = thickness;
            if (!lighter && darker) edge[exy] = -thickness;
        }
    }

    // Finding more. It's enough to just handle the proper inside.
    for (int step = thickness - 1; step > 0; --step) {
        for  (int y = 1; y < piece->h - 1; ++y)
         for (int x = 1; x < piece->w - 1; ++x) {
            const int exy = piece->w * y + x;
            if (edge[exy] != 0 || is_pink(piece, x, y)) continue;
            else {
                // We're in the proper inside. +-1 is always valid
                // in each direction and yields an existing pixel again.
                bool lighter = edge[exy - 1]            == step + 1
                            || edge[exy - piece->w]     == step + 1
                            || edge[exy - piece->w - 1] == step + 1;
                bool darker  = edge[exy + 1]            == -step - 1
                            || edge[exy + piece->w]     == -step - 1
                            || edge[exy + piece->w + 1] == -step - 1;
                if (lighter && !darker) edge[exy] = step;
                if (!lighter && darker) edge[exy] = -step;
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
        const int exy = piece->w * y + x;
        const int p = _getpixel32(piece, x, y);
        if (p == pink) continue;
        else if (edge[exy] == 0) continue;
        else {
            _putpixel32(piece, x, y, makecol32(
             recolor_base_color(getr32(p), edge[exy], extra_thickness),
             recolor_base_color(getg32(p), edge[exy], extra_thickness),
             recolor_base_color(getb32(p), edge[exy], extra_thickness)));
        }
    }
}
