#include <vector>

#include "blot.h"

inline int recolor_pillar_color(
    const int    base,
    const double light,
    const int    dampening,
    const int    cap
) {
    double extra;
    if (light > 0) extra = (255 - base) * (120 - dampening) / 120.0;
    else           extra = - base       * (120 - dampening) / 120.0;
    
    if (cap) {
        if (extra >  cap) extra = cap;
        if (extra < -cap) extra = -cap;
    }
    const double abslight = (light >= 0 ? light : -light);
    const int ret = base
        + extra * abslight
        + 0.5; // this is a correcting summand to counter the floor rounding
               // from the conversion double -> int
    if      (ret <   0) return 0;
    else if (ret > 255) return 255;
    else                return ret;
}



bool Blotter::process_shape_pillar(BITMAP* piece)
{
    if (!texture || piece->w > texture->w || piece->h > texture->h) {
        return false;
    }

    const bool horz = (main_mode == PILLAR_HORZ);

    // horizontal means that the top  will be lighter, bottom will be darker.
    // vertical   means that the left will be lighter, right  will be darker.
    const int span   = horz ? piece->h : piece->w;
    const int boring = horz ? piece->w : piece->h;
    if (span <= 1) return true;

    std::vector <double> light(span, 0); // max = 1.0, min = -1.0
    for (int i = 0; i < span; ++i) {
        // linear from +1.0 to -1.0
        light[i] = 1.0 - i * 2.0 / (span - 1);
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

    // Apply the lighting effect
    for  (int s = 0; s < span;   ++s)
     for (int b = 0; b < boring; ++b) {
        const int x = horz ? b : s;
        const int y = horz ? s : b;
        const int p = _getpixel32(piece, x, y);
        if (p == pink) continue;
        
        const int& arg1 = pillar_dampening_of_120;
        const int& arg2 = pillar_strength;
        _putpixel32(piece, x, y, makecol32(
            recolor_pillar_color(getr32(p), light[s], arg1, arg2),
            recolor_pillar_color(getg32(p), light[s], arg1, arg2),
            recolor_pillar_color(getb32(p), light[s], arg1, arg2)));
    }
    return true;
}
