#include "app.hpp"

int main()
{
    try
    {
        mgo::App app;
        app.run();
    }
    catch (const std::exception& errorMessage)
    {
        std::cerr << errorMessage.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
