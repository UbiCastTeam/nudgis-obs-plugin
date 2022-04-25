#include "nudgis-service.hpp"

#include <iostream>

#define DEF_FILENAME "/home/pagou/Dev/ubicast/mediaserver-client/test.mkv"

int main(int argc, char *argv[])
{
    const char * filename = DEF_FILENAME;
    if (argc > 1)
        filename = argv[1];

    std::cout << "start with " << std::endl
              << "  filename: " << filename << std::endl;

    nudgis_upload_file(filename);
    return 0;
}