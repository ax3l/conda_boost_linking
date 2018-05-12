#include <iostream>
#include <boost/filesystem.hpp>

#include "myLib.hpp"


int f(
    int argc,
    char* argv[]
)
{
    using namespace boost::filesystem;

    if( argc < 2 )
    {
        std::cout << "Usage: tut1 pathToFile\n";
        return 1;
    }

    std::cout << argv[ 1 ] << " "
              << file_size( argv[ 1 ] )
              << std::endl;
    return 0;
}
