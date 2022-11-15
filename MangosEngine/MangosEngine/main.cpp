#include "application.hpp"

int main()
{
    try
    {
        mgo::Application application;
        application.run();
    }
    catch (const std::exception& errorMessage)
    {
        MGO_LOG(errorMessage.what());
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
