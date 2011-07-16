#pragma once

#include <allegro.h>
#include <string>
#include <set>
#include <list>

class Blotter {

public:

    static void initialize();
    
    static int pink;

    Blotter();
    ~Blotter();

    void set_thickness(int);
    void set_dampening(int);
    void set_strength (int);
    
    bool    load_texture(const std::string&); // returns false on error
    BITMAP* get_texture ();                   // returns 0 on error
    
    bool    load_shapes(const std::string&); // returns false on error
    int     get_shapes_remaining();
    BITMAP* pop_shape_caller_should_destroy_it_later(); // returns 0 on error

    bool process_shape(BITMAP*); // modifies the bitmap, returns 0 on error,
                                 // which means the texture is too small    
private:

    typedef std::pair <int, int> Xy;
    typedef std::set  <Xy>       Xyset;
    typedef std::list <BITMAP*>  Bitlist;
    
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


    
    int thickness;
    int dampening;
    int strength;
    
    BITMAP* texture;
    Bitlist shapes;

    void destroy_all_shapes  ();
    bool cut_into_list       (BITMAP*); // returns false on error
    void find_connected_area (      Area&, const int,   const int);
    void fill_area_with_color(const Area&, const int);

};
