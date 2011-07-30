// there's no header for processs(), so forward-declare it if necessary.

#include <iostream>
#include <string>

#include "io.h"
#include "blot.h"

// returns -1 on error, otherwise the number of written images.
// the caller should just exit on getting the -1, process has already
// written the error to standard output.
int process(const std::vector <IO::Line>& iolines)
{
    Blotter b;
    
    std::string prefix = "";
    std::string suffix = ".pcx";
    
    int gran_red   = 0;
    int gran_green = 0;
    int gran_blue  = 0;
    int gran_granu = 16;
    
    // these are just to give a better error message to help newb users
    bool newb_shapes_loaded  = false;
        
    int images_written = 0;
    
    for (std::vector <IO::Line> ::const_iterator lineitr = iolines.begin();
        lineitr != iolines.end(); ++lineitr)
    {
        const IO::Line l = *lineitr;
        
        if (l.type == '#') {
            if (l.nr1 < 0) {
                std::cout << "Error: ``#" << l.text1
                    << "'' has a negative argument." << std::endl;
                return -1;
            }
            else if (l.text1 == "bevel-thickness") {
                b.set_bevel_thickness(l.nr1);
                std::cout << "Bevel Thickness set to "
                    << l.nr1 << "." << std::endl;
            }
            else if (l.text1 == "bevel-dampening") {
                b.set_bevel_dampening(l.nr1);
                std::cout << "Bevel Dampening set to "
                    << l.nr1 << "." << std::endl;
            }
            else if (l.text1 == "bevel-strength") {
                b.set_bevel_strength(l.nr1);
                std::cout << "Bevel Strength set to "
                    << l.nr1 << "." << std::endl;
            }
            else if (l.text1 == "pillar-dampening") {
                b.set_pillar_dampening(l.nr1);
                std::cout << "Pillar Dampening set to "
                    << l.nr1 << "." << std::endl;
            }
            else if (l.text1 == "pillar-dampening") {
                b.set_pillar_dampening(l.nr1);
                std::cout << "Pillar Dampening set to "
                    << l.nr1 << "." << std::endl;
            }
            else if (l.text1 == "pillar-strength") {
                b.set_pillar_strength(l.nr1);
                std::cout << "Pillar Strength set to "
                    << l.nr1 << "." << std::endl;
            }
            else if (l.text1 == "granulate-red"
                  || l.text1 == "granulate-green"
                  || l.text1 == "granulate-blue"
                  || l.text1 == "granulate-granularity") {
                if (l.nr1 < 0 || l.nr1 > 255) {
                    std::cout << "Error: ``#" << l.text1
                        << " " << l.nr1 << "'' is outside of [0, 255]."
                        << std::endl;
                    return -1;
                }
                else {
                    if      (l.text1 == "granulate-red")   gran_red   = l.nr1;
                    else if (l.text1 == "granulate-green") gran_green = l.nr1;
                    else if (l.text1 == "granulate-blue")  gran_blue  = l.nr1;
                    else                                   gran_granu = l.nr1;
                }
            }
            else {
                std::cout << "Error: ``#" << l.text1
                    << "'' is an unknown command." << std::endl;
                return -1;
            }
        }
        
        else if (l.type == '$') {
            if      (l.text1 == "prefix") prefix = l.text2; 
            else if (l.text1 == "suffix") suffix = l.text2;
            else if (l.text1 == "mode") {
                if (l.text2 == "bevel") {
                    b.set_main_mode(Blotter::BEVEL);
                    std::cout << "Bevel mode selected." << std::endl;
                }
                else if (l.text2 == "pillar-horizontal") {
                    b.set_main_mode(Blotter::PILLAR_HORZ);
                    std::cout << "Pillar-Horizontal mode selected."
                        << std::endl;
                }
                else if (l.text2 == "pillar-vertical") {
                    b.set_main_mode(Blotter::PILLAR_VERT);
                    std::cout << "Pillar-Vertical mode selected." << std::endl;
                }
                else {
                    std::cout << "Error: ``" << l.text2
                        << "'' is an unknown mode." << std::endl;
                    return -1;                    
                }
            }
            else if (l.text1 == "granulate-generate") {
                if (b.make_granulate(gran_red,  gran_green,
                                     gran_blue, gran_granu)) {
                    std::cout << "Using granulate texture with color ("
                        << gran_red  << ", " << gran_green << ", "
                        << gran_blue << ") and granularity "
                        << gran_granu << "." << std::endl;
                }
                else {
                    std::cout << "Error: Couldn't make granulate texture."
                        << std::endl;
                    return -1;
                }
            }
            else if (l.text1 == "texture") {
                if (b.load_texture(l.text2)) {
                    std::cout << "Using texture ``" << l.text2 << "''."
                        << std::endl;
                }
                else {
                    std::cout << "Error: ``" << l.text2
                        << "'' isn't a texture file." << std::endl;
                    return -1;
                }
            }
            else if (l.text1 == "shapes") {
                if (b.load_shapes(l.text2)) {
                    newb_shapes_loaded = true;
                    std::cout << "Using " << b.get_shapes_remaining()
                        << " shapes from ``" << l.text2 << "''." << std::endl;
                }
                else {
                    std::cout << "Error: ``" << l.text2
                        << "'' isn't a shapes file, or it's too huge."
                        << std::endl;
                    return -1;
                }
            }
            else {
                std::cout << "Error: ``$" << l.text1
                    << "'' is an unknown command." << std::endl;
                return -1;
            }
        }
        
        else if (l.type == '>') {
            bool txtr = b.get_texture();
            BITMAP* shape = b.pop_shape_caller_should_destroy_it_later();
            if (shape && txtr) {
                bool ok = b.process_shape(shape);
                if (!ok) {
                    const int tw = b.get_texture()->w;
                    const int th = b.get_texture()->h;
                    const int sw = shape->w;
                    const int sh = shape->h;
                    std::cout
                        << "Error: The texture (" << tw << " x " << th << ")"
                        << "is too small for the shape (" << sw << " x "
                        << sh << ")." << std::endl;
                    ::destroy_bitmap(shape);
                    return -1;
                }
                else {
                    std::string outfile = prefix + l.text1 + suffix;
                    save_pcx(outfile.c_str(), shape, 0);
                    ++images_written;
                    std::cout << "  > ``" << outfile << "'', "
                        << shape->w << " x " << shape->h << "." << std::endl;
                    ::destroy_bitmap(shape);
                }
            }
            // end no error in > line
            else  {
                if (shape == 0 && !txtr)
                    std::cout << "Error: No texture or shapes available."
                    << std::endl;
                else if (shape == 0 && !newb_shapes_loaded) {
                    std::cout << "Error: No shapes available." << std::endl;
                }
                else if (shape == 0 && newb_shapes_loaded) {
                    std::cout << "Error: No more shapes left." << std::endl;
                }
                else std::cout << "Error: No texture available." << std::endl;                
                if (!txtr || !newb_shapes_loaded)
                    std::cout << "Every commands file must first "
                    << (!txtr ? "load/generate a texture" : "")
                    << (!txtr && !newb_shapes_loaded ? " and " : "")
                    << (!newb_shapes_loaded ? "load a shapes file" : "")
                    << "." << std::endl << "Before doing so, "
                    << "no output commands (`>' lines) may be specified."
                    << std::endl;                    
                if (shape) ::destroy_bitmap(shape);
                return -1;
            }

        }
        // end processing > lines
    }
    return images_written;
}
