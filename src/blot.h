#pragma once

#include <allegro.h>
#include <string>
#include <set>
#include <list>
#include "color.h"

class Blotter {

public:

    enum MainMode {
        BEVEL,
        PILLAR_HORZ,
        PILLAR_VERT,
        RAW
    };

    static void initialize();

    static int pink;

    Blotter();
    ~Blotter();

    void set_main_mode(MainMode);

    void set_bevel_thickness(int);
    void set_bevel_dampening(int);
    void set_bevel_strength (int);

    void set_pillar_dampening(int);
    void set_pillar_strength (int);

    bool    make_granulate(int, int, int, int); // r-g-b-granularity; 0 on err
    /* light color, dark color,
     * amount of dots, size, thinness,
     * p of the p-norm to be used for the induced metric */
    bool    make_voronoi(color, color, int, pos, int, double = 2.0);
    // same as above, however like shaded tiles
    bool    make_voronoi_shaded(color, color, int, pos, int, double = 2.0);
    // generates a matrix containing a shading factor
    // and an angle for each pixel
    std::vector<std::vector<std::pair<double,double> > >
        make_voronoi_table(int, pos, int, double = 2.0);
    bool    load_texture  (const std::string&); // returns false on error
    BITMAP* get_texture   ();                   // returns 0 on error

    bool    load_shapes(const std::string&); // returns false on error
    int     get_shapes_remaining();
    BITMAP* pop_shape_caller_should_destroy_it_later(); // returns 0 on error

    bool process_shape(BITMAP*); // modifies the bitmap, returns 0 on error,
                                 // which means the texture is too small
private:

    typedef std::pair <int, int> Xy;
    typedef std::set  <Xy>       Xyset;
    typedef std::list <BITMAP*>  Bitlist;

    double calc_ortho_lambda(pos, pos, pos);
    double calc_voronoi_lambda(pos, pos, pos, int);
    double voronoi_antialiased_angle(pos, pos, pos);

    struct Area {
        BITMAP* ground;
        Xyset   Xy;
        int x_min;
        int x_max;
        int y_min;
        int y_max;

        Area(BITMAP* g) : ground(g),
            x_min(ground->w - 1), x_max(0),
            y_min(ground->h - 1), y_max(0) { }
    };

    MainMode main_mode;

    int bevel_thickness;
    int bevel_dampening;
    int bevel_strength;

    int pillar_dampening_of_120;
    int pillar_strength;

    BITMAP* texture;
    Bitlist shapes;

    void destroy_all_shapes  ();
    bool cut_into_list       (BITMAP*); // returns false on error
    void find_connected_area (      Area&, const int,   const int);
    void fill_area_with_color(const Area&, const int);

    bool process_shape_bevel (BITMAP*);
    bool process_shape_pillar(BITMAP*);
    bool process_shape_raw(BITMAP*);

    ///////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////

    // Inline function
    inline bool is_pink(BITMAP* b, const int x, const int y)
    {
        if (x < 0 || y < 0 || x >= b->w || y >= b->h) return true;
        else return _getpixel32(b, x, y) == Blotter::pink;
    }

};
