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
    
    // these are just to give a better error message to help newb users
    bool newb_shapes_loaded  = false;
    bool newb_texture_loaded = false;
        
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
            else if (l.text1 == "thickness") {
                b.set_thickness(l.nr1);
                std::cout << "Thickness set to " << l.nr1 << "." << std::endl;
            }
            else if (l.text1 == "dampening") {
                b.set_dampening(l.nr1);
                std::cout << "Dampening set to " << l.nr1 << "." << std::endl;
            }
            else if (l.text1 == "strength") {
                b.set_strength(l.nr1);
                std::cout << "Strength set to " << l.nr1 << "." << std::endl;
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
            else if (l.text1 == "texture") {
                if (b.load_texture(l.text2)) {
                    newb_texture_loaded = true;
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
                std::cout << "Now loading shapes file ``" << l.text2 << "''..."
                    << std::endl;
                if (b.load_shapes(l.text2)) {
                    newb_shapes_loaded = true;
                    std::cout << "Found " << b.get_shapes_remaining()
                        << " shapes in ``" << l.text2 << "''." << std::endl;
                }
                else {
                    std::cout << "Error: File not found or too huge."
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
            BITMAP* shape = b.pop_shape_caller_should_destroy_it_later();
            if (shape == 0) {
                std::cout << "Error: No shapes available." << std::endl;
                if (!newb_texture_loaded || !newb_shapes_loaded) {
                    std::cout << "Every commands file must first load a "
                        << (!newb_texture_loaded ? "texture file" : "")
                        << (!newb_texture_loaded && !newb_shapes_loaded ?
                            " and a " : "")
                        << (!newb_shapes_loaded ? "shapes file" : "")
                        << "." << std::endl << "Before doing so, "
                        << "no output commands (`>' lines) may be specified."
                        << std::endl;                    
                }
                return -1;
                // don't have to destroy this after all
            }
            else {
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
                    std::cout << "Written ``" << outfile << "'', "
                        << shape->w << " x " << shape->h << "." << std::endl;
                    ::destroy_bitmap(shape);
                }
            }
            // end else (shape != 0)
        }
        // end processing > lines
    }
    return images_written;
}



/*
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
    */
