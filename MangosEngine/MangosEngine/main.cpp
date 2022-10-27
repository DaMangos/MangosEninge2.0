#include "App.hpp"

int main()
{
    try
    {
        mgo::App app;
        app.Run();
    }
    catch (const std::exception& caught)
    {
        std::cerr << caught.what() << '\n';
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

