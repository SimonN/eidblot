#pragma once

namespace Blot {

    typedef std::pair <int, int> XY;
    struct Area;
    int pink;

    void cut_into_list       (BITMAP*, std::list <BITMAP*>&);
    void find_connected_area (      Area&, const int,   const int);
    void fill_area_with_color(const Area&, const int);
    void apply_texture       (BITMAP*, BITMAP*);

}
