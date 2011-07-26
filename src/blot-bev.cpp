#include <vector>

#include "blot.h"

// This is private to bevelling
inline int recolor_base_color(
    const int base,
    const int light,
    const int thick,
    const int cap
) {
    int color_new;
    if (light > 0) {
        color_new = base + (255 - base) * light / thick;
        if (cap && color_new - base > cap * light) return base + cap * light;
        else                                       return color_new;
    }
    else {
        color_new = base +     base     * light / thick;
        if (cap && color_new - base < cap * light) return base + cap * light;
        else                                       return color_new;
    }    
}



bool Blotter::process_shape_bevel(BITMAP* piece)
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
    const int extra_thickness = bevel_thickness + bevel_dampening;

    // Find extreme outside points which would get colored badly otherwise
    for  (int y = 0; y < piece->h; ++y)
     for (int x = 0; x < piece->w; ++x) {
        const int eXy = piece->w * y + x;
        if (getpixel(piece, x, y) == pink) continue;
        else {
            bool lighter = is_pink(piece, x-1, y-1);
            bool darker  = is_pink(piece, x+1, y+1);
            if (lighter && !darker) edge[eXy] = bevel_thickness;
            if (!lighter && darker) edge[eXy] = -bevel_thickness;
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
            if (lighter && !darker) edge[eXy] = bevel_thickness;
            if (!lighter && darker) edge[eXy] = -bevel_thickness;
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
            if (lighter && !darker) edge[eXy] =   (bevel_thickness / 6) + 1;
            if (!lighter && darker) edge[eXy] = - (bevel_thickness / 6) - 1;
        }
    }

    // Finding inside pixels. It's enough to just handle the proper inside.
    for (int step = bevel_thickness - 1; step > 0; --step) {
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
            const int& str = bevel_strength;
            _putpixel32(piece, x, y, makecol32(
             recolor_base_color(getr32(p), edge[eXy], extra_thickness, str),
             recolor_base_color(getg32(p), edge[eXy], extra_thickness, str),
             recolor_base_color(getb32(p), edge[eXy], extra_thickness, str)));
        }
    }
    return true;
}
