/*
 * this is the new main function, following the specifications in
 * ./doc/spec.txt.
 *
 */

#include "blot.h"

int main()
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
