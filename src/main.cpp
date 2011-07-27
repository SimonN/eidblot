/*
 * this is the new main function, following the specifications in
 * ./doc/spec.txt.
 *
 */

#include <iostream>

#include "io.h"
#include "blot.h"

int process(const std::vector <IO::Line>&);



int main(int argc, char* argv[])
{
    allegro_init();
    set_color_depth(32);
    Blotter::initialize();

    std::cout << "This is eidblot." << std::endl;

    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " rules1.txt [ rules2.txt ... ]"
                  << std::endl;
        return 0;
    }

    int  scripts_processed = 0;
    int  images_written    = 0;
    bool error_occured     = false;

    for (int i = 1; i < argc; ++i) {
        const std::string filename = argv[i];
        std::vector <IO::Line> lines;
        if (! IO::fill_vector_from_file(lines, argv[i])) {
            std::cout << "Error: ``" << filename << "'' isn't a commands file."
            << std::endl;
            error_occured = true;
            break;
        }
        else if (lines.empty()) {
            std::cout << "Error: ``" << filename << "'' has no commands."
                << std::endl;
            error_occured = true;
            break;
        }
        else {
            std::cout << "Processing commands file ``"
                << filename << "''." << std::endl;
            int imgs = process(lines);
            if (imgs >= 0) {
                scripts_processed += 1;
                images_written    += imgs;
                std::cout << "Done with ``" << filename << "'', " << imgs
                    << " images written." << std::endl;
            }
            else {
                // process() has already printed the error.
                error_occured = true;
                break;
            }
        }
    }
        
    std::cout
        << scripts_processed << "/" << argc - 1
        << " commands files successfully processed, ";
    if (error_occured) std::cout << "aborted due to error." << std::endl;
    else std::cout << images_written << " images written. " << std::endl;

    return 0;
}
END_OF_MAIN()
